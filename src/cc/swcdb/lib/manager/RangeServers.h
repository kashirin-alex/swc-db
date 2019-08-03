
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServers_h
#define swc_lib_manager_RangeServers_h
#include <memory>

#include "swcdb/lib/db/Columns/MNGR/Columns.h"
#include "swcdb/lib/db/Protocol/req/LoadRange.h"
#include "swcdb/lib/db/Protocol/req/IsRangeLoaded.h"
#include "swcdb/lib/db/Protocol/req/RsIdReqNeeded.h"
#include "swcdb/lib/db/Files/RsData.h"
#include "RangeServerStatus.h"

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
  }

  void init() {
    check_assignment();
  }

  virtual ~RangeServers(){}

  
  uint64_t rs_set_id(EndPoints endpoints, uint64_t opt_id=0){
    return rs_set(endpoints, opt_id)->rs_id;
  }

  bool rs_ack_id(uint64_t rs_id, EndPoints endpoints){
    bool ack = false;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
    
      for(auto h : m_rs_status){
        if(has_endpoint(h->endpoints, endpoints) && rs_id == h->rs_id){
          h->ack = true;
          ack = true;
          break;
        }
      }
    }

    if(ack)
      check_assignment(30000);
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
    bool was_removed=false;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
        auto h = *it;
        if(has_endpoint(h->endpoints, endpoints)){
          m_rs_status.erase(it);
          was_removed = true;
          EnvMngrColumns::get()->set_rs_unassigned(h->rs_id);
          break;
        }
      }
    }

    if(was_removed)
      check_assignment(30000);
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

  void assign_range(RangeServerStatusPtr rs, RangePtr range){
    
    client::ClientConPtr conn = EnvClients::get()->rs_service->get_connection(
                            rs->endpoints, std::chrono::milliseconds(5000), 1);      
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

  void range_loaded(RangeServerStatusPtr rs, RangePtr range, bool loaded) {

    if(!loaded){
      {
        std::lock_guard<std::mutex> lock(m_mutex_rs_status);
        rs->total_ranges--;
      }
      range->set_state(Range::State::NOTSET, 0); 

    } else {
      range->set_state(Range::State::ASSIGNED, rs->rs_id); 
    }

    HT_DEBUGF("RANGE-LOADED, cid=%d %s\n%s", 
              range->cid, range->to_string().c_str(), to_string().c_str());
  }

  private:


  RangeServerStatusPtr rs_set(EndPoints endpoints, uint64_t opt_id=0){
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);

    for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)) {
        if(h->ack) {
          if(endpoints.size() > h->endpoints.size())
            h->endpoints = endpoints;
          return h;
        } else {
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

    RangeServerStatusPtr h = std::make_shared<RangeServerStatus>(nxt, endpoints);
    m_rs_status.push_back(h);
    return h;
  }

  void check_assignment(uint32_t t_ms = 0){
    if(!initialized){
      initialized = true;
      initialize_cols();
      timer_assignment_checkin(30000);
      return;

    } else if(t_ms) {
      timer_assignment_checkin(t_ms);
      return;

    } else if(!assign_ranges()) {
      timer_assignment_checkin(10000);
      return;
    }
    


    // for rangeserver cid-rid state

    timer_assignment_checkin(60000);

  }

  void timer_assignment_checkin(uint32_t t_ms = 10000) {
    auto set_in = std::chrono::milliseconds(t_ms);
    
    std::lock_guard<std::mutex> lock(m_mutex);
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

  bool initialized = false;

  void initialize_cols(){
    std::vector<int64_t> cols;
    EnvMngrRoleState::get()->get_active_columns(cols);
    if(cols.size() == 0){
      initialized = false;
      return; 
    }

    int64_t last_id=1;
    auto c_it = std::find_if(cols.begin(), cols.end(),  
                [](const int64_t& cid_set){return cid_set == 1;});
    m_root_mngr = c_it != cols.end();
    if(m_root_mngr) {
      //  m_last_cid =  columns.data (last_cid)
      if(m_last_cid == 0){
        while(initialize_col(++last_id, true));
        m_last_cid = last_id-1;
      }
    }

    bool till_end = *cols.begin() == 0;
    if(till_end){
      cols.erase(cols.begin());
      last_id = *cols.end();
    }

    while(cols.size() > 0){
      initialize_col(*cols.begin());
      cols.erase(cols.begin());
    }

    if(till_end)
      while(initialize_col(++last_id));

  }

  bool initialize_col(int64_t cid, bool only_exists_chk=false) {

    bool exists = Column::exists(cid);
    if(cid != 1 && !exists)
      return false;
    if(only_exists_chk)
      return true;

    ColumnPtr col = EnvMngrColumns::get()->get_column(cid, true);
    if(!exists && !col->create()) {
      HT_ERRORF("Unable to create column=%d", cid);
    }

    // File::ColumnAssignments assignments.data (rs(endpoints)[rid])

    // for(auto assignment : assignments) {
    //   RangeServerStatusPtr rs = rs_set(assignment.endpoints)
    //   for(auto rid : assignment.ranges) {
    //     if(rs->has_range(cid, rid))
    //       continue;
    //     RangePtr range = col->get_range(rid, true);
    //     asio::post(*EnvIoCtx::io()->ptr(), [rs, range](){ 
    //       EnvRangeServers::get()->assign_range(rs, range); 
    //     });
    //   }
    // }

    // col->load_last_rid() // column.data (last_range)


    int64_t last_rid = -1; //col->get_last_rid();
    if(last_rid == -1){
      int err = Error::OK;
      FS::IdEntries_t entries;
      col->ranges_by_fs(err, entries);

      if(entries.size() == 0) 
        entries.push_back(1); // initialize 1st range
 
      for(auto rid : entries)
        col->get_range(rid, true);
      
    } else {
      for(int64_t rid=1; rid<=last_rid; rid++)
        col->get_range(rid, true);
    }
    
    return true;
  }
  
  bool assign_ranges(){

    RangePtr range;
    for(;;){
      if((range = EnvMngrColumns::get()->get_next_unassigned()) == nullptr)
        break;

      RangeServerStatusPtr rs;
      
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

  void assign_range(RangeServerStatusPtr rs, RangePtr range, 
                    Files::RsDataPtr last_rs){
    if(last_rs == nullptr){
      assign_range(rs, range);
      return;
    }

    client::ClientConPtr conn = EnvClients::get()->rs_service->get_connection(
        last_rs->endpoints, std::chrono::milliseconds(30000), 1);

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

  void next_rs(Files::RsDataPtr &last_rs, RangeServerStatusPtr &rs_set){
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);
    
    if(m_rs_status.size() == 0)
      return;

    if(last_rs->endpoints.size() > 0) {
       for(auto rs : m_rs_status) {
          if(rs->failures < 255 && has_endpoint(rs->endpoints, last_rs->endpoints)){
            rs_set = rs;
            last_rs = nullptr;
            break;
          }
       }
    } else 
      last_rs = nullptr;
    
    size_t num_rs;
    size_t avg_ranges;
    RangeServerStatusPtr rs;

    while(rs_set == nullptr && m_rs_status.size() > 0){
    
      avg_ranges = 0;
      num_rs = 0;
      // avg_resource_ratio = 0;
      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++) {
        avg_ranges = avg_ranges*num_rs + (*it)->total_ranges;
        // resource_ratio = avg_resource_ratio*num_rs + (*it)->resource();
        avg_ranges /= ++num_rs;
        // avg_resource_ratio /= num_rs;
      }

      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
        rs = *it;
        if(avg_ranges < rs->total_ranges) // *avg_resource_ratio
          continue;

        if(rs->failures == 255){
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

  std::mutex          m_mutex;
  std::mutex          m_mutex_rs_status;
  
  TimerPtr            m_assign_timer; 

  RangeServerStatusList m_rs_status;

  int64_t             m_last_cid = 0;
  bool                m_root_mngr = false;
};


}}}

#endif // swc_lib_manager_RangeServers_h