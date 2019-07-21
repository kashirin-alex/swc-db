/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_RoleState_h
#define swc_app_manager_RoleState_h

#include "swcdb/lib/client/Clients.h"

#include "HostStatus.h"

namespace SWC { namespace server { namespace Mngr {
class RoleState;
typedef std::shared_ptr<RoleState> RoleStatePtr;
}}}
#include "AppContextMngrClient.h"

#include "swcdb/lib/db/Protocol/req/MngrsState.h"


namespace SWC { namespace server { namespace Mngr {


class RoleState : public std::enable_shared_from_this<RoleState> {
  public:
  RoleState(IOCtxPtr ioctx) : m_ioctx(ioctx){
    cfg_conn_probes = Config::settings->get_ptr<gInt32t>(
      "swc.mngr.RoleState.connection.probes");
    cfg_conn_timeout = Config::settings->get_ptr<gInt32t>(
      "swc.mngr.RoleState.connection.timeout");
    cfg_req_timeout = Config::settings->get_ptr<gInt32t>(
      "swc.mngr.RoleState.request.timeout");
    cfg_check_interval = Config::settings->get_ptr<gInt32t>(
      "swc.mngr.RoleState.check.interval");
    cfg_delay_updated = Config::settings->get_ptr<gInt32t>(
      "swc.mngr.RoleState.check.delay.updated");
    cfg_delay_fallback = Config::settings->get_ptr<gInt32t>(
      "swc.mngr.RoleState.check.delay.fallback");
  }

  virtual ~RoleState() { }

  void init(EndPoints endpoints) {
    m_local_endpoints = endpoints;
    m_clients = std::make_shared<client::Clients>(
      m_ioctx, 
      std::make_shared<client::Mngr::AppContext>(shared_from_this())
    );
    
    m_local_token = endpoints_hash(m_local_endpoints);
    asio::post(*m_ioctx.get(), [ptr=shared_from_this()]{ptr->managers_checkin();});
  }

  void apply_cfg(){
    HT_DEBUG("apply_cfg");

    client::Mngr::SelectedGroups groups = 
      m_clients->mngrs_groups->get_groups();
    
    for(auto g : groups) {
      HT_DEBUG( g->to_string().c_str());
      uint32_t pr = 0;
      for(auto endpoints : g->get_hosts()) {

        bool found = false;
        std::lock_guard<std::mutex> lock(m_mutex);
        for(auto host : m_states){
          found = has_endpoint(endpoints, host->endpoints);
          if(found)break;
        }
        if(found)continue;

        m_states.push_back(std::make_shared<HostStatus>(
          g->col_begin, g->col_end, endpoints, nullptr, ++pr)); 
      }
    }
    m_local_groups = m_clients->mngrs_groups->get_groups(m_local_endpoints);
  }
  
  void timer_managers_checkin(uint32_t t_ms = 10000) {
    if(m_check_timer != nullptr) // adjust timer
      m_check_timer->cancel();

    m_check_timer = std::make_shared<asio::high_resolution_timer>(
      *m_ioctx.get(), std::chrono::milliseconds(t_ms)); 

    m_check_timer->async_wait(
      [ptr=shared_from_this()](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          ptr->managers_checkin();
          ptr->m_check_timer=nullptr;
        }
    }); 
    std::cout << " timer=" << t_ms << "\n";
  }

  void managers_checkin(){
    HT_DEBUG("managers_checkin");

    if(m_check_timer != nullptr) m_check_timer->cancel();

    apply_cfg();
    
    HostStatuses new_states;
    {
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t total = m_states.size();
    if(total == 0) {
      timer_managers_checkin(cfg_check_interval->get());
      return;
    }

    // request to a one manager followed local manager, incl. last's is first
    bool flw = false;
    HostStatusPtr host_chk = nullptr;

    auto it = m_states.begin();
    for(;;){

      host_chk = *it;
      
      if(it+1 == m_states.end())
        it = m_states.begin();
      else
        it++;

      if(has_endpoint(host_chk->endpoints, m_local_endpoints) && total > 1){
        if(flw) {
          timer_managers_checkin(cfg_check_interval->get());
          return;
        }
        flw = true;
        continue;
      }
      if(!flw)
        continue;
        
      if(host_chk->conn == nullptr || !host_chk->conn->is_open()){
        m_mngr_inchain = nullptr;
        host_chk->conn = m_clients->mngr_service->get_connection(
          host_chk->endpoints, 
          std::chrono::milliseconds(cfg_conn_timeout->get()), 
          cfg_conn_probes->get());
        
        if(host_chk->conn == nullptr){
          total--;
          host_chk->state = State::OFF;
          continue;
        } else 
          total = m_states.size();

        host_chk->conn->accept_requests();
      }
      m_mngr_inchain = host_chk->conn;

      break;
    }

    new_states.assign(m_states.begin(), m_states.end());
    }
    
    fill_states(new_states);
  }
  
  void update_state(EndPoint endpoint, State state){
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto host : m_states){
      if(has_endpoint(endpoint, host->endpoints)){
        host->state = state;
      }
    }
  }

  void update_state(EndPoints endpoints, State state){
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto host : m_states){
      if(has_endpoint(endpoints, host->endpoints)){
        host->state = state;
      }
    }
  }
  
  HostStatusPtr get_host(EndPoints endpoints){
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto host : m_states){
      if(has_endpoint(endpoints, host->endpoints))
        return host;
    }
    return nullptr;
  }

  bool has_state(uint64_t begin, uint64_t end, State state){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for(auto host : m_states){
      if(host->col_begin == begin 
        && host->col_end == end && host->state == state)
        return true;
    }
    return false;
  }

  HostStatusPtr get_highest_state_host(uint64_t begin, uint64_t end){
    std::lock_guard<std::mutex> lock(m_mutex);

    HostStatusPtr h = nullptr;
    for(auto host : m_states){
      if(host->col_begin == begin && host->col_end == end 
        && (h == nullptr || h->state < host->state)){
        h = host;
      }
    }
    return h;
  }
  
  bool is_off(uint64_t begin, uint64_t end){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    bool offline = true;
    for(auto host : m_states){
      if(host->col_begin == begin 
        && host->col_end == end && host->state != State::OFF)
        offline = false;
    }
    return offline;
  }

  void fill_states(HostStatuses states, 
                   uint64_t token=0, ResponseCallbackPtr cb=nullptr){

    bool turn_around = token == m_local_token;
    bool new_recs = false;

    /* 
    std::cout << "BEFORE:\n";
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto h : m_states)
        std::cout << h->to_string() << "\n";
    } 
    */

    for(auto host : states){

      bool local = has_endpoint(host->endpoints, m_local_endpoints);

      if(local && token == 0 && (int)host->state < (int)State::STANDBY){
        update_state(host->endpoints, State::STANDBY);
        continue;
      }
      
      HostStatusPtr host_set = get_host(host->endpoints);
      
      if((int)host->state < (int)State::STANDBY){
        if(host->state != host_set->state) {
          update_state(host->endpoints, 
            host->state > host_set->state ? host->state : host_set->state);  
          // pass, states: NOTSET | OFF
          new_recs = true;
        }
        continue;
      }

      HostStatusPtr high_set = 
        get_highest_state_host(host->col_begin, host->col_end);
       
      if(high_set->state == State::ACTIVE) {
        if(host->state != host_set->state
           && host_set->priority != high_set->priority) {
          update_state(host->endpoints, State::STANDBY);
          new_recs = true;
          continue;
        }
        continue;
      }
      if(host->state == State::ACTIVE){
        update_state(host->endpoints, host->state);
        new_recs = true;
        continue;
      }
      
        
      if(host->priority > high_set->priority){
        if((int)host->state > (int)State::STANDBY){
          update_state(host->endpoints, (State)(host->state-1));
          new_recs = true;
        }
      } else {
        if((int)host->state < (int)State::ACTIVE){
          update_state(host->endpoints, (State)(host->state+1));
          new_recs = true;
        }
      }

    }
    
    for(auto host : states){
      if(!is_off(host->col_begin, host->col_end))
        continue;
      auto hosts_pr_group = 
        m_clients->mngrs_groups->get_endpoints(
          host->col_begin, host->col_end);
      for(auto h : hosts_pr_group){
        if(has_endpoint(h, host->endpoints) 
          || !has_endpoint(h, m_local_endpoints))
          continue;
            
        HostStatusPtr l_host = get_host(m_local_endpoints);
        HostStatusPtr l_hight = get_highest_state_host(
          l_host->col_begin, l_host->col_end);
        if(l_hight->state < State::WANT) {
          update_state(m_local_endpoints, State::WANT);
          new_recs = true;
        }
        break;
      }
    }

    std::cout << "AFTER: local=" << m_local_token
              << " token=" << token << " turn_around=" << turn_around
              << " cb=" << (size_t)cb.get()
              << " dest=" << m_mngr_inchain->endpoint_remote_str() << "\n";
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto h : m_states)
        std::cout << h->to_string() << "\n";
    } 
    
    if(token == 0 || !turn_around) {
      if(token == 0)
        token = m_local_token;

      HostStatuses updated_states;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        updated_states.assign(m_states.begin(), m_states.end());
      }
      
      Protocol::Req::MngrsStatePtr req = 
        std::make_shared<Protocol::Req::MngrsState>(
          m_mngr_inchain, updated_states, token, m_local_endpoints[0], 
          cb, shared_from_this());

      if(!req->run(
              (cfg_conn_probes->get() * cfg_conn_timeout->get()
               + cfg_req_timeout->get()) * updated_states.size()))
        timer_managers_checkin(3000);
      return;
    }
    
    //if(cb != nullptr)
    //  cb->response_ok();

    timer_managers_checkin(
      new_recs ? cfg_delay_updated->get() : cfg_check_interval->get());
    
  }

  void update_manager_addr(uint64_t hash, EndPoint mngr_host){
    std::lock_guard<std::mutex> lock(m_mutex);
    //std::cout << "update_manager_addr,  hash=" << hash 
    //    << " mngr_host="<<mngr_host.address().to_string() << ":" <<mngr_host.port() << "\n";
    m_mngrs_client_srv.insert(std::make_pair(hash, mngr_host));
  }
  
  bool disconnection(EndPoint endpoint_server, EndPoint endpoint_client, 
                     bool srv=false){
    if(srv) {
      std::lock_guard<std::mutex> lock(m_mutex);
      auto it = m_mngrs_client_srv.find(endpoint_hash(endpoint_server));
      if(it == m_mngrs_client_srv.end())
        return false;
      endpoint_server = (*it).second;
      m_mngrs_client_srv.erase(it);
    }
    
    HT_INFOF("disconnection, srv=%d, server=[%s]:%d, client=[%s]:%d", 
              (int)srv,
              endpoint_server.address().to_string().c_str(), 
              endpoint_server.port(), 
              endpoint_client.address().to_string().c_str(), 
              endpoint_client.port());

    HostStatusPtr host_set = get_host((EndPoints){endpoint_server});
    if(host_set == nullptr)
      return false;
    timer_managers_checkin(
      host_set->state == State::ACTIVE ? 
      cfg_delay_fallback->get() : cfg_check_interval->get());

    update_state(endpoint_server, State::OFF);
    return true;
  }

  bool is_active(size_t cid){
    auto host = active_mngr(cid, cid);
    return host != nullptr && has_endpoint(host->endpoints, m_local_endpoints);
  }

  HostStatusPtr active_mngr(size_t begin, size_t end){
    
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto host : m_states){
      if(host->state == State::ACTIVE && host->col_begin <= begin 
        && (host->col_end == 0 || host->col_end >= end)){
        std::cout << "active, " << host->to_string() << "\n";
        return host;
      }
    }

    std::cout << "active-none begin=" << begin 
                           << " end=" << end << "\n";
    return nullptr;
  }

  private:
  
  IOCtxPtr                     m_ioctx;
  HostStatuses                 m_states;
  std::mutex                   m_mutex;
  TimerPtr                     m_check_timer; 
  
  client::ClientsPtr           m_clients;
  client::ClientConPtr         m_mngr_inchain = nullptr;
  EndPoints                    m_local_endpoints;
  uint64_t                     m_local_token;
  client::Mngr::SelectedGroups m_local_groups;

  std::unordered_map<uint64_t, EndPoint> m_mngrs_client_srv;


  gInt32tPtr  cfg_conn_probes;
  gInt32tPtr  cfg_conn_timeout;
  gInt32tPtr  cfg_req_timeout;
  gInt32tPtr  cfg_delay_updated;
  gInt32tPtr  cfg_check_interval;
  gInt32tPtr  cfg_delay_fallback;
};

}}}


namespace SWC {namespace Protocol { namespace Req {
void MngrsState::disconnected() {
  role_state->disconnection(conn->endpoint_remote, conn->endpoint_local);
}
}}}

namespace SWC { namespace client { namespace Mngr {
  void AppContext::disconnected(ConnHandlerPtr conn){
    role_state->disconnection(conn->endpoint_remote, conn->endpoint_local);
  }
}}}

#endif // swc_app_manager_RoleState_h