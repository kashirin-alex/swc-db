/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_MngrRole_h
#define swc_app_manager_MngrRole_h

#include "swcdb/manager/MngrStatus.h"

#include "swcdb/manager/AppContextClient.h"

#include "swcdb/db/Protocol/Mngr/req/MngrState.h"


namespace SWC { namespace server { namespace Mngr {


class MngrRole final {
  public:
  
  typedef MngrRole* Ptr;

  MngrRole()
    : m_check_timer(asio::high_resolution_timer(*Env::IoCtx::io()->ptr())),
      cfg_conn_probes(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.role.connection.probes")),
      cfg_conn_timeout(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.role.connection.timeout")),
      cfg_conn_fb_failures(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.role.connection.fallback.failures")),
      cfg_req_timeout(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.role.request.timeout")),
      cfg_check_interval(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.role.check.interval")),
      cfg_delay_updated(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.role.check.delay.updated")),
      cfg_delay_fallback(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.role.check.delay.fallback")),
      m_checkin(false),
      m_mngr_inchain(std::make_shared<Protocol::Common::Req::ConnQueue>()) {    
  }

  Ptr ptr() {
    return this;
  }

  ~MngrRole() { }

  void init(const EndPoints& endpoints) {
    m_local_endpoints = endpoints;
    m_local_token = endpoints_hash(m_local_endpoints);

    timer_managers_checkin(3000);
  }

  void timer_managers_checkin(uint32_t t_ms = 10000) {
    std::lock_guard lock(m_mutex_timer);
    if(!m_run)
      return;

    auto set_in = std::chrono::milliseconds(t_ms);
    auto set_on = m_check_timer.expires_from_now();
    if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
      return;

    m_check_timer.cancel();
    m_check_timer.expires_from_now(set_in);

    m_check_timer.async_wait(
      [ptr=ptr()](const asio::error_code ec) {
        if(ec != asio::error::operation_aborted){
          ptr->managers_checkin();
        }
    }); 
    SWC_LOGF(LOG_DEBUG, "MngrRole managers_checkin scheduled in ms=%d", t_ms);
  }

  bool has_active_columns(){
    std::shared_lock lock(m_mutex);
    return m_cols_active.size() > 0;
  }

  void get_active_columns(std::vector<int64_t> &cols){
    if(!has_active_columns())
      set_active_columns();

    std::shared_lock lock(m_mutex);
    cols.assign(m_cols_active.begin(), m_cols_active.end());
  }

  bool is_active(size_t cid){
    auto host = active_mngr(cid, cid);
    return host != nullptr && has_endpoint(host->endpoints, m_local_endpoints);
  }

  MngrStatus::Ptr active_mngr(size_t begin, size_t end){
    std::shared_lock lock(m_mutex);
    for(auto& host : m_states){
      if(host->state == Types::MngrState::ACTIVE 
        && host->col_begin <= begin 
        && (host->col_end == 0 || host->col_end >= end)){
        return host;
      }
    }
    return nullptr;
  }

  void req_mngr_inchain(Protocol::Common::Req::ConnQueue::ReqBase::Ptr req){
    m_mngr_inchain->put(req);
  }

  bool fill_states(MngrsStatus states, uint64_t token, 
                   ResponseCallback::Ptr cb){

    bool new_recs = false;
    bool turn_around = token == m_local_token;

    for(auto& host : states){

      bool local = has_endpoint(host->endpoints, m_local_endpoints);

      if(local && token == 0 
         && (int)host->state < (int)Types::MngrState::STANDBY){
        update_state(host->endpoints, Types::MngrState::STANDBY);
        continue;
      }
      
      MngrStatus::Ptr host_set = get_host(host->endpoints);
      
      if(host_set->state == Types::MngrState::OFF 
         && host->state > Types::MngrState::OFF) {
        //m_major_updates = true;
        timer_managers_checkin(500);
      }

      if((int)host->state < (int)Types::MngrState::STANDBY){
        if(host->state != host_set->state) {
          update_state(host->endpoints, 
            host->state != Types::MngrState::NOTSET?
            host->state : host_set->state);  
          new_recs = true;
        }
        continue;
      }

      MngrStatus::Ptr high_set = 
        get_highest_state_host(host->col_begin, host->col_end);
       
      if(high_set->state == Types::MngrState::ACTIVE) {
        if(host->state != host_set->state
           && host_set->priority != high_set->priority) {
          update_state(host->endpoints, Types::MngrState::STANDBY);
          new_recs = true;
        }
        continue;
      }
      if(host->state == Types::MngrState::ACTIVE){
        update_state(host->endpoints, host->state);
        new_recs = true;
        // m_major_updates = true;
        continue;
      }
      
        
      if(host->priority > high_set->priority){
        if((int)host->state > (int)Types::MngrState::STANDBY){
          update_state(host->endpoints, (Types::MngrState)(host->state-1));
          new_recs = true;
        }
      } else {
        if((int)host->state < (int)Types::MngrState::ACTIVE){
          update_state(host->endpoints, (Types::MngrState)(host->state+1));
          new_recs = true;
        }
      }

    }
    
    for(auto& host : states){
      if(!is_off(host->col_begin, host->col_end))
        continue;
      auto hosts_pr_group = 
        Env::Clients::get()->mngrs_groups->get_endpoints(
          host->col_begin, host->col_end);
      for(auto& h : hosts_pr_group){
        if(has_endpoint(h, host->endpoints) 
          || !has_endpoint(h, m_local_endpoints))
          continue;
            
        MngrStatus::Ptr l_host = get_host(m_local_endpoints);
        MngrStatus::Ptr l_hight = get_highest_state_host(
          l_host->col_begin, l_host->col_end);
        if(l_hight->state < Types::MngrState::WANT) {
          update_state(m_local_endpoints, Types::MngrState::WANT);
          new_recs = true;
        }
        break;
      }
    }

    {
      std::shared_lock lock(m_mutex);

      if(token == 0 || !turn_around) {
        if(token == 0)
          token = m_local_token;

        req_mngr_inchain(std::make_shared<Protocol::Mngr::Req::MngrState>(
          cb, m_states, token, m_local_endpoints[0], 
          (cfg_conn_probes->get() * cfg_conn_timeout->get()
          + cfg_req_timeout->get()) * m_states.size()
          ));
        //  timer_managers_checkin(3000);
        return false;
      }
    }
    
    if(cb != nullptr)
      cb->response_ok();

    timer_managers_checkin(
      new_recs ? cfg_delay_updated->get() : cfg_check_interval->get());
    
    std::cout << to_string() << "\n";
    return set_active_columns();
  }

  void update_manager_addr(uint64_t hash, const EndPoint& mngr_host){
    std::lock_guard lock(m_mutex);

    bool new_srv = m_mngrs_client_srv.insert(std::make_pair(hash, mngr_host)).second;
    if(new_srv) {
      //m_major_updates = true;
      timer_managers_checkin(500);
    }
  }
  
  bool disconnection(const EndPoint& endpoint_server, const EndPoint& endpoint_client, 
                     bool srv=false){
    EndPoints endpoints;
    {
      std::lock_guard lock(m_mutex);
    
      auto it = m_mngrs_client_srv.find(endpoint_hash(endpoint_server));
      if(it != m_mngrs_client_srv.end()) {
        endpoints.push_back((*it).second);
        m_mngrs_client_srv.erase(it);
      } else 
        endpoints.push_back(endpoint_server);
    }
    MngrStatus::Ptr host_set = get_host(endpoints);
    if(host_set == nullptr)
      return false;

    timer_managers_checkin(
      host_set->state == Types::MngrState::ACTIVE ? 
      cfg_delay_fallback->get() : cfg_check_interval->get());

    SWC_LOGF(LOG_DEBUG, "disconnection, srv=%d, server=[%s]:%d, client=[%s]:%d", 
              (int)srv,
              endpoint_server.address().to_string().c_str(), 
              endpoint_server.port(), 
              endpoint_client.address().to_string().c_str(), 
              endpoint_client.port());
    if(host_set->state != Types::MngrState::ACTIVE)
      update_state(endpoint_server, Types::MngrState::OFF);
    // m_major_updates = true;
    return true;
  }
  
  bool require_sync(){
    std::lock_guard lock(m_mutex);
    bool current = m_major_updates;
    m_major_updates = false;
    return current;
  }

  void stop() {
    {
      std::lock_guard lock(m_mutex_timer);
      m_check_timer.cancel();
      m_run = false;
    }

    m_mngr_inchain->stop();

    {
      std::shared_lock lock(m_mutex);
      for(auto& host : m_states) {
        if(host->conn != nullptr && host->conn->is_open())
          asio::post(
            *Env::IoCtx::io()->ptr(), [conn=host->conn](){conn->do_close();});
      }
    }
  }

  const std::string to_string() {
    std::string s("Mngrs Role:");
    
    for(auto& h : m_states) {
      s.append("\n ");
      s.append(h->to_string());
    }
    s.append("\nMngrInchain ");
    s.append(m_mngr_inchain->to_string());

    s.append("\nLocal-Endpoints: ");
    for(auto& endpoint : m_local_endpoints) {
      s.append(" [");
      s.append(endpoint.address().to_string());
      s.append("]:");
      s.append(std::to_string(endpoint.port()));
      s.append(",");
    }
    return s;
  }
  private:
  
  void apply_cfg(){

    client::Mngr::Groups::Selected groups = 
      Env::Clients::get()->mngrs_groups->get_groups();
    
    for(auto& g : groups) {
      // SWC_LOG(LOG_DEBUG,  g->to_string().c_str());
      uint32_t pr = 0;
      for(auto& endpoints : g->get_hosts()) {

        bool found = false;
        for(auto& host : m_states){
          found = has_endpoint(endpoints, host->endpoints);
          if(found)break;
        }
        if(found)continue;

        m_states.push_back(std::make_shared<MngrStatus>(
          g->col_begin, g->col_end, endpoints, nullptr, ++pr)); 
      }
    }
    
    m_local_groups = Env::Clients::get()->mngrs_groups->get_groups(
      m_local_endpoints);
  }

  void managers_checkin(){
    if(m_checkin++)
      return;

    //SWC_LOG(LOG_DEBUG, "managers_checkin");
    size_t sz;
    {
      std::lock_guard lock(m_mutex);
      apply_cfg();
      sz = m_states.size();
    }
   managers_checker(0, sz, false);
  }

  void fill_states(){
    MngrsStatus states;
    {
      std::shared_lock lock(m_mutex);
      states.assign(m_states.begin(), m_states.end());
    }
    fill_states(states, 0, nullptr);
  }

  void managers_checker(int next, size_t total, bool flw){
    // set manager followed(in-chain) local manager, incl. last's is first

    if(total == 0) {
      m_checkin=0;
      timer_managers_checkin(cfg_check_interval->get());
      return;
    }

    MngrStatus::Ptr host_chk;
    {
      std::shared_lock lock(m_mutex);
      if(!m_run)
        return;
      if(next == m_states.size())
        next = 0;
      host_chk = m_states.at(next++);
    }

    if(has_endpoint(host_chk->endpoints, m_local_endpoints) && total >= 1){
      if(flw) {
        m_checkin=0;
        timer_managers_checkin(cfg_check_interval->get());
        return;
      }
      flw = true;
      if(total > 1) {
        managers_checker(next, total, flw);
        return;
      }
    }
    if(!flw) {
      managers_checker(next, total, flw);
      return;
    }
        
    if(host_chk->conn != nullptr && host_chk->conn->is_open()){
      set_mngr_inchain(host_chk->conn);
      return;
    }

    Env::Clients::get()->mngr_service->get_connection(
      host_chk->endpoints, 
      [host_chk, next, total, flw, ptr=ptr()]
      (ConnHandlerPtr conn){
        ptr->manager_checker(host_chk, next, total, flw, conn);
      },
      std::chrono::milliseconds(cfg_conn_timeout->get()), 
      cfg_conn_probes->get()
    );
  }

  void manager_checker(MngrStatus::Ptr host, int next, size_t total, bool flw,
                       ConnHandlerPtr conn){
    if(conn == nullptr || !conn->is_open()){
      if(host->state == Types::MngrState::ACTIVE
         && ++host->failures <= cfg_conn_fb_failures->get()){   
        m_checkin=0;
        timer_managers_checkin(cfg_delay_fallback->get());
        SWC_LOGF(LOG_DEBUG, "Allowed conn Failure=%d before fallback", 
                  host->failures);
        return;
      }
      host->state = Types::MngrState::OFF;
      managers_checker(next, total-1, flw);
      return;
    }
    host->conn = conn;
    host->failures = 0;
    //conn->accept_requests();
    m_major_updates = true;
    set_mngr_inchain(host->conn);
  }
  
  void update_state(EndPoint endpoint, Types::MngrState state){
    std::lock_guard lock(m_mutex);

    for(auto& host : m_states){
      if(has_endpoint(endpoint, host->endpoints)){
        host->state = state;
      }
    }
  }

  void update_state(const EndPoints& endpoints, Types::MngrState state){
    std::lock_guard lock(m_mutex);

    for(auto& host : m_states){
      if(has_endpoint(endpoints, host->endpoints)){
        host->state = state;
      }
    }
  }
  
  MngrStatus::Ptr get_host(const EndPoints& endpoints){
    std::shared_lock lock(m_mutex);

    for(auto& host : m_states){
      if(has_endpoint(endpoints, host->endpoints))
        return host;
    }
    return nullptr;
  }

  MngrStatus::Ptr get_highest_state_host(uint64_t begin, uint64_t end){
    std::shared_lock lock(m_mutex);

    MngrStatus::Ptr h = nullptr;
    for(auto& host : m_states){
      if(host->col_begin == begin && host->col_end == end 
        && (h == nullptr || h->state < host->state)){
        h = host;
      }
    }
    return h;
  }
  
  bool is_off(uint64_t begin, uint64_t end){
    std::shared_lock lock(m_mutex);

    bool offline = true;
    for(auto& host : m_states){
      if(host->col_begin == begin 
        && host->col_end == end && host->state != Types::MngrState::OFF)
        offline = false;
    }
    return offline;
  }

  bool set_active_columns(){
    
    client::Mngr::Groups::Selected groups;
    {
      std::shared_lock lock(m_mutex);
      groups = m_local_groups;
    }

    std::vector<int64_t> cols_active;
    for(auto& group : groups){
      int64_t cid =   group->col_begin == 0 ?  1  : group->col_begin;
      int64_t cid_end = group->col_end == 0 ? cid : group->col_end;

      if(group->col_end == 0 && is_active(cid)){
        cols_active.push_back(group->col_end);
      }
      
      for(;cid <= cid_end; cid++) { 
        auto c_it = std::find_if(cols_active.begin(), cols_active.end(),  
          [cid](const int64_t& cid_set){return cid_set == cid;});
        if(c_it == cols_active.end() && is_active(cid))
           cols_active.push_back(cid);
      }
    }
    
    {
      std::lock_guard lock(m_mutex);
      if(cols_active != m_cols_active){
        m_cols_active.swap(cols_active);
        return true;
      }
    }
    return false;
  }
  
  void set_mngr_inchain(ConnHandlerPtr mngr){
    m_mngr_inchain->set(mngr);

    fill_states();
    m_checkin=0;
    timer_managers_checkin(cfg_check_interval->get());
  }

  EndPoints                       m_local_endpoints;
  uint64_t                        m_local_token;

  std::shared_mutex               m_mutex;
  MngrsStatus                     m_states;
  std::atomic<uint8_t>            m_checkin;
  client::Mngr::Groups::Selected  m_local_groups;
  std::vector<int64_t>            m_cols_active;
  bool                            m_major_updates = false;
  std::unordered_map<uint64_t,  EndPoint> m_mngrs_client_srv;
  
  std::mutex                      m_mutex_timer;
  asio::high_resolution_timer     m_check_timer; 
  bool                            m_run = true; 
  
  Protocol::Common::Req::ConnQueue::Ptr  m_mngr_inchain;


  const gInt32tPtr cfg_conn_probes;
  const gInt32tPtr cfg_conn_timeout;
  const gInt32tPtr cfg_conn_fb_failures;
  
  const gInt32tPtr cfg_req_timeout;
  const gInt32tPtr cfg_delay_updated;
  const gInt32tPtr cfg_check_interval;
  const gInt32tPtr cfg_delay_fallback;
};

}} // server namespace



namespace Env {
class MngrRole final {
  public:

  static void init() {
    m_env = std::make_shared<MngrRole>();
  }

  static server::Mngr::MngrRole::Ptr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_role_state;
  }

  MngrRole() : m_role_state(new server::Mngr::MngrRole()) {}

  ~MngrRole(){
    if(m_role_state != nullptr)
      delete m_role_state;
  }

  private:
  server::Mngr::MngrRole::Ptr              m_role_state = nullptr;
  inline static std::shared_ptr<MngrRole>  m_env = nullptr;
};
} // Env namespace



namespace Protocol { namespace Mngr {namespace Req {
  void MngrState::disconnected(ConnHandlerPtr conn) {
    Env::MngrRole::get()->disconnection(
      conn->endpoint_remote, conn->endpoint_local);
  }
}}}

namespace client { namespace Mngr {
  void AppContext::disconnected(ConnHandlerPtr conn){
    Env::MngrRole::get()->disconnection(
      conn->endpoint_remote, conn->endpoint_local);
  }
}}

}
#endif // swc_app_manager_MngrRole_h