
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServers_h
#define swc_lib_manager_RangeServers_h
#include <memory>

#include "swcdb/lib/db/Columns/MNGR/Columns.h"

#include "swcdb/lib/db/Files/RsData.h"
#include "RsStatus.h"

#include "swcdb/lib/db/Protocol/req/LoadRange.h"
#include "swcdb/lib/db/Protocol/req/IsRangeLoaded.h"
#include "swcdb/lib/db/Protocol/req/RsIdReqNeeded.h"
#include "swcdb/lib/db/Protocol/req/MngrUpdateRangeServers.h"

#include "swcdb/lib/db/Files/Schema.h"

namespace SWC { namespace server { namespace Mngr {

class RangeServers;
typedef std::shared_ptr<RangeServers> RangeServersPtr;
}}

class EnvRangeServers {
  
  public:

  static void init() {
    m_env = std::make_shared<EnvRangeServers>();
  }

  static server::Mngr::RangeServersPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_rangeservers;
  }

  EnvRangeServers() 
    : m_rangeservers(std::make_shared<server::Mngr::RangeServers>()) {}

  virtual ~EnvRangeServers(){}

  private:
  server::Mngr::RangeServersPtr                  m_rangeservers = nullptr;
  inline static std::shared_ptr<EnvRangeServers> m_env = nullptr;
};


namespace server { namespace Mngr {


class RangeServers {

  public:
  RangeServers()
    : m_assign_timer(
        std::make_shared<asio::high_resolution_timer>(*EnvIoCtx::io()->ptr())
      ) {    
    cfg_rs_failures = EnvConfig::settings()->get_ptr<gInt32t>(
      "swc.mngr.ranges.assign.RS.remove.failures");
    cfg_delay_rs_chg = EnvConfig::settings()->get_ptr<gInt32t>(
      "swc.mngr.ranges.assign.delay.onRangeServerChange");
    cfg_delay_cols_init = EnvConfig::settings()->get_ptr<gInt32t>(
      "swc.mngr.ranges.assign.delay.afterColumnsInit");
    cfg_chk_assign = EnvConfig::settings()->get_ptr<gInt32t>(
      "swc.mngr.ranges.assign.interval.check");

    cfg_rs_conn_timeout = EnvConfig::settings()->get_ptr<gInt32t>(
      "swc.mngr.ranges.assign.RS.connection.timeout");
    cfg_rs_conn_probes = EnvConfig::settings()->get_ptr<gInt32t>(
      "swc.mngr.ranges.assign.RS.connection.probes");
  }

  void new_columns() {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_columns_set = false;
    }
    timer_assignment_checkin(500);
  }
  
  void require_sync() {
    rs_changes(m_rs_status, true);
  }

  virtual ~RangeServers(){}

  void add_column(DB::SchemaPtr schema, int &err){
    if(EnvSchemas::get()->get(schema->col_name)){
      err = Error::SCHEMA_COL_NAME_EXISTS;
      return;
    }
    // needed, call is after mngr initialized (wait?)
    // err = Error::MNGR_NOT_INITIALIZED;
    std::lock_guard<std::mutex> lock(m_mutex);

    bool reused;
    int64_t cid = get_next_cid(reused);
    if(reused)
      Column::clear_marked_deleted(cid);

    initialize_col(cid, false, true);

    schema = DB::Schema::make(cid, schema);
    EnvSchemas::get()->add(schema);  
    Files::Schema::save(schema);
  }

  void update_status(RsStatusList new_rs_status, bool sync_all){
    RsStatusList changed;

    {
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);

    if(m_root_mngr && !sync_all)
      return;

    RsStatusPtr h;
    bool found;
    bool chg;

    for(auto rs_new : new_rs_status){
      found = false;
      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
        h = *it;
        if(!has_endpoint(h->endpoints, rs_new->endpoints))
          continue;

        chg = false;
        if(rs_new->rs_id != h->rs_id){ 
          if(m_root_mngr)
            rs_new->rs_id = rs_set(rs_new->endpoints, rs_new->rs_id)->rs_id;
          
          if(m_root_mngr && rs_new->rs_id != h->rs_id)
            EnvMngrColumns::get()->change_rs(h->rs_id, rs_new->rs_id);

          h->rs_id = rs_new->rs_id;
          chg = true;
        }
       if(rs_new->endpoints != h->endpoints){
          h->endpoints = rs_new->endpoints;
          chg = true;
        }
        if(rs_new->state == RsStatus::State::ACK) {
          if(rs_new->state != h->state) {
            h->state = RsStatus::State::ACK;
            chg = true;
          }
        } else {
          EnvMngrColumns::get()->set_rs_unassigned(h->rs_id);
          m_rs_status.erase(it);
          chg = true;
        }

        if(chg && !sync_all)
          changed.push_back(rs_new);
        found = true;
        break;
      }

      if(!found){
        if(rs_new->state == RsStatus::State::ACK)
          m_rs_status.push_back(rs_new);
        if(!sync_all)
          changed.push_back(rs_new);
      }
    }
    }
    
    rs_changes(sync_all ? m_rs_status : changed, sync_all && !m_root_mngr);
    if(sync_all || changed.size() > 0){
      std::cout << "Updated RS-hosts: sync_all=" << sync_all << "\n";
      for(auto rs : sync_all ? m_rs_status : changed){
        std::cout << " " << rs->to_string() << "\n";
      }
    }
    
  }

  uint64_t rs_set_id(EndPoints endpoints, uint64_t opt_id=0){
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);
    return rs_set(endpoints, opt_id)->rs_id;
  }

  bool rs_ack_id(uint64_t rs_id, EndPoints endpoints){
    bool ack = false;
    RsStatusPtr new_ack = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
    
      for(auto h : m_rs_status){
        if(has_endpoint(h->endpoints, endpoints) && rs_id == h->rs_id){
          if(h->state != RsStatus::State::ACK)
            new_ack = h;
          h->state = RsStatus::State::ACK;
          ack = true;
          break;
        }
      }
    }

    if(new_ack != nullptr) 
      rs_changes({new_ack});
    return ack;
  }

  uint64_t rs_had_id(uint64_t rs_id, EndPoints endpoints){
    bool new_id_required = false;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);

      for(auto h : m_rs_status){
        if(rs_id == h->rs_id){
          if(has_endpoint(h->endpoints, endpoints))
            return 0; // zero=OK
          new_id_required = true;
          break;
        }
      }
    }
    
    return rs_set_id(endpoints, new_id_required ? 0 : rs_id);
  }

  void rs_shutdown(uint64_t rs_id, EndPoints endpoints){
    RsStatusPtr removed = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
        auto h = *it;
        if(has_endpoint(h->endpoints, endpoints)){
          m_rs_status.erase(it);
          EnvMngrColumns::get()->set_rs_unassigned(h->rs_id);
          h->state = RsStatus::State::REMOVED;
          removed = h;
          break;
        }
      }
    }
    if(removed != nullptr)
      rs_changes({removed});   
  }

  std::string to_string(){
    std::string s("RangeServers:");
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto h : m_rs_status) {
        s.append("\n ");
        s.append(h->to_string());
      }
    }
    s.append("\n");
    s.append(EnvMngrColumns::get()->to_string());
    return s;
  }

  void stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_assign_timer->cancel();
    m_run = false;
  }

  private:
  
  void rs_changes(RsStatusList hosts, bool sync_all=false){
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      if(hosts.size() > 0)
        Protocol::Req::MngrUpdateRangeServers::put(hosts, sync_all);
    }
    
    if(EnvMngrRoleState::get()->has_active_columns())
      timer_assignment_checkin(cfg_delay_rs_chg->get());
  }

  void check_assignment(uint32_t t_ms = 0){
    std::lock_guard<std::mutex> lock(m_mutex);

    if(!m_columns_set){
      initialize_cols();
      timer_assignment_checkin(cfg_delay_cols_init->get());
      return;

    } else if(t_ms) {
      timer_assignment_checkin(t_ms);
      return;

    } else if(!assign_ranges()) {
      timer_assignment_checkin(10000);
      return;
    }
    


    // for rangeserver cid-rid state

    timer_assignment_checkin(cfg_chk_assign->get());

  }

  void timer_assignment_checkin(uint32_t t_ms = 10000) {
    if(!m_run)
      return;
    auto set_in = std::chrono::milliseconds(t_ms);
    auto set_on = m_assign_timer->expires_from_now();
    if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
      return;
    m_assign_timer->cancel();
    m_assign_timer->expires_from_now(set_in);

    m_assign_timer->async_wait(
      [](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          EnvRangeServers::get()->check_assignment();
        }
    }); 
    HT_DEBUGF("RS ranges check_assignment scheduled in ms=%d", t_ms);
  }

  void initialize_cols(){
    std::vector<int64_t> cols;
    EnvMngrRoleState::get()->get_active_columns(cols);
    if(cols.size() == 0){
      m_columns_set = false;
      return; 
    }

    int64_t last_id = 1;
    auto c_it = std::find_if(cols.begin(), cols.end(),  
                [](const int64_t& cid_set){return cid_set == 1;});
    m_root_mngr = c_it != cols.end();

    if(m_root_mngr && m_last_cid == 0) { // done once
      while(initialize_col(last_id, true))
        initialize_schema(last_id++);
      m_last_cid = last_id-1;
    }

    bool till_end = *cols.begin() == 0;
    if(till_end){
      cols.erase(cols.begin());
      last_id = *cols.end();
    }

    while(cols.size() > 0 && initialize_col(*cols.begin()))
      cols.erase(cols.begin());
    
    if(till_end)
      while(initialize_col(++last_id));

    m_columns_set = true;
  }

  void initialize_schema(int64_t cid){
    if(Column::is_marked_deleted(cid)){
      m_cols_reuse.push(cid);
      return;
    }
    EnvSchemas::get()->add(Files::Schema::load(cid));
  }

  bool initialize_col(int64_t cid, 
                      bool only_exists_chk=false, bool new_col=false) {

    bool exists = Column::exists(cid);
    if(cid > 3){
      if(only_exists_chk || !exists)
        return exists;
    }

    ColumnPtr col = EnvMngrColumns::get()->get_column(cid, true);
    if(!exists && !col->create()) {
      HT_ERRORF("Unable to create column=%d", cid);
    }

    if(new_col || !exists){
      // initialize 1st range
      col->get_range(1, true);
      return true;
    }

    //col->load_last_rid() // column.data (last_range)
    int64_t last_rid = -1; //col->get_last_rid();
    if(last_rid == -1){
      int err = Error::OK;
      FS::IdEntries_t entries;
      col->ranges_by_fs(err, entries);
 
      for(auto rid : entries)
        col->get_range(rid, true); 
      
    } else {
      for(int64_t rid=1; rid<=last_rid; rid++)
        col->get_range(rid, true);
    }
    
    return true;
  }

  int64_t get_next_cid(bool &reused) {
    reused = m_cols_reuse.size() > 0;
    if(reused){
      int64_t cid = m_cols_reuse.front();
      m_cols_reuse.pop();
      return cid;
    }
    return ++m_last_cid;
  }
  
  bool assign_ranges(){

    RangePtr range;
    for(;;){
      if((range = EnvMngrColumns::get()->get_next_unassigned()) == nullptr)
        break;

      RsStatusPtr rs = nullptr;
      
      Files::RsDataPtr last_rs = range->get_last_rs();
      next_rs(last_rs, rs);
      if(rs == nullptr)
        return false;

      range->set_state(Range::State::QUEUED, rs->rs_id);

      asio::post(*EnvIoCtx::io()->ptr(), [rs, range, last_rs](){ 
        EnvRangeServers::get()->assign_range(rs, range, last_rs); 
      });
    }
    return true;
  }

  void next_rs(Files::RsDataPtr &last_rs, RsStatusPtr &rs_set){
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);
    
    if(m_rs_status.size() == 0)
      return;

    if(last_rs->endpoints.size() > 0) {
       for(auto rs : m_rs_status) {
          if(rs->state == RsStatus::State::ACK
            && rs->failures < cfg_rs_failures->get() 
            && has_endpoint(rs->endpoints, last_rs->endpoints)){
            rs_set = rs;
            last_rs = nullptr;
            break;
          }
       }
    } else 
      last_rs = nullptr;
    
    size_t num_rs;
    size_t avg_ranges;
    RsStatusPtr rs;

    while(rs_set == nullptr && m_rs_status.size() > 0){
      avg_ranges = 0;
      num_rs = 0;
      // avg_resource_ratio = 0;
      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++) {
        rs = *it;
        if(rs->state != RsStatus::State::ACK)
          continue;
        avg_ranges = avg_ranges*num_rs + rs->total_ranges;
        // resource_ratio = avg_resource_ratio*num_rs + rs->resource();
        avg_ranges /= ++num_rs;
        // avg_resource_ratio /= num_rs;
      }

      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
        rs = *it;
        if(rs->state != RsStatus::State::ACK || avg_ranges < rs->total_ranges)
          continue;

        if(rs->failures == cfg_rs_failures->get()){
          m_rs_status.erase(it);
          EnvMngrColumns::get()->set_rs_unassigned(rs->rs_id);
          continue;
        }
        rs_set = rs;
        break;
      }
    }

    if(rs_set != nullptr)
      rs_set->total_ranges++;
    return;
  }

  void assign_range(RsStatusPtr rs, RangePtr range, 
                    Files::RsDataPtr last_rs){
    if(last_rs == nullptr){
      assign_range(rs, range);
      return;
    }

    client::ClientConPtr conn = EnvClients::get()->rs_service->get_connection(
      last_rs->endpoints, 
      std::chrono::milliseconds(cfg_rs_conn_timeout->get()), 
      cfg_rs_conn_probes->get()
    );

    if(conn != nullptr 
      && (std::make_shared<Protocol::Req::RsIdReqNeeded>(
          conn, [rs, range] (bool err) {     
            err ? EnvRangeServers::get()->assign_range(rs, range)
                : EnvRangeServers::get()->range_loaded(rs, range, false);
        }))->run())
      EnvClients::get()->rs_service->preserve(conn);
    else 
      assign_range(rs, range);
  }

  void assign_range(RsStatusPtr rs, RangePtr range){
    
    client::ClientConPtr conn = EnvClients::get()->rs_service->get_connection(
      rs->endpoints, 
      std::chrono::milliseconds(cfg_rs_conn_timeout->get()), 
      cfg_rs_conn_probes->get()
    );
        
    if(conn == nullptr){
        {
          std::lock_guard<std::mutex> lock(m_mutex_rs_status);
          rs->failures++;
        }
        EnvRangeServers::get()->range_loaded(rs, range, false);
      return;
    }
    EnvClients::get()->rs_service->preserve(conn);
    
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      rs->failures = 0;
    }
    
    Protocol::Req::Callback::LoadRange_t cb = [rs, range](bool loaded){
      EnvRangeServers::get()->range_loaded(rs, range, loaded); 
    };
    if(!(std::make_shared<Protocol::Req::LoadRange>(conn, range, cb))->run())
      cb(false);
  }

  void range_loaded(RsStatusPtr rs, RangePtr range, bool loaded) { // + resource_chg

    if(!loaded){
      {
        std::lock_guard<std::mutex> lock(m_mutex_rs_status);
        rs->total_ranges--;
      }
      range->set_state(Range::State::NOTSET, 0); 

    } else {
      range->set_state(Range::State::ASSIGNED, rs->rs_id); 
      // adjust rs->resource
      // ++ mng_inchain - req. MngrRsResource
    }

    HT_DEBUGF("RANGE-LOADED, cid=%d %s\n%s", 
              range->cid, range->to_string().c_str(), to_string().c_str());
  }

  RsStatusPtr rs_set(EndPoints endpoints, uint64_t opt_id=0){

    for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)) {
        if(h->state == RsStatus::State::ACK) {
          if(endpoints.size() > h->endpoints.size())
            h->endpoints = endpoints;
          return h;
        } else {
          EnvMngrColumns::get()->set_rs_unassigned(h->rs_id);
          m_rs_status.erase(it);
          break;
        }
      }
    }

    uint64_t next_id=0;
    uint64_t nxt;
    bool ok;
    do {
      if(opt_id == 0) {
        nxt = ++next_id;
      } else {
        nxt = opt_id;
        opt_id = 0;
      }
      
      ok = true;
      for(auto h : m_rs_status){
        if(nxt == h->rs_id) {
          ok = false;
          break;
        };
      }
    } while(!ok);

    RsStatusPtr h = std::make_shared<RsStatus>(nxt, endpoints);
    m_rs_status.push_back(h);
    return h;
  }

  std::mutex          m_mutex;
  std::mutex          m_mutex_rs_status;
  
  bool                m_columns_set = false;
  TimerPtr            m_assign_timer; 
  bool                m_run=true; 

  RsStatusList m_rs_status;

  int64_t             m_last_cid = 0;
  std::queue<int64_t> m_cols_reuse;
  bool                m_root_mngr = false;

  gInt32tPtr          cfg_rs_failures;
  gInt32tPtr          cfg_delay_rs_chg;
  gInt32tPtr          cfg_delay_cols_init;
  gInt32tPtr          cfg_chk_assign;
  gInt32tPtr          cfg_rs_conn_timeout;
  gInt32tPtr          cfg_rs_conn_probes;
  
};


}}}

#endif // swc_lib_manager_RangeServers_h