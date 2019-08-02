
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServers_h
#define swc_lib_manager_RangeServers_h
#include <memory>

#include "swcdb/lib/db/Columns/Columns.h"
#include "swcdb/lib/db/Protocol/req/LoadRange.h"
#include "swcdb/lib/db/Protocol/req/IsRangeLoaded.h"
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

  void init() {}

  virtual ~RangeServers(){}

  
  uint64_t rs_set_id(EndPoints endpoints, uint64_t opt_id=0){
    return rs_set(endpoints, opt_id)->rs_id;
  }

  bool rs_ack_id(uint64_t rs_id , EndPoints endpoints){
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

  uint64_t rs_had_id(uint64_t rs_id , EndPoints endpoints){
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

  void rs_shutdown(uint64_t rs_id , EndPoints endpoints){
    bool was_removed=false;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
        auto h = *it;
        if(has_endpoint(h->endpoints, endpoints)){
          m_rs_status.erase(it);
          {
            std::lock_guard<std::mutex> lock(m_mutex);
            set_unassigned(h);
          }
          was_removed = true;
          break;
        }
      }
    }

    if(was_removed)
      check_assignment(30000);
  }

  std::string to_string(){
    std::string s("RangeServers:\n");
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto h : m_rs_status)
        s.append(h->to_string());
    }
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      s.append(" Not-Assigned=(");
      for(auto col : m_not_assigned){
        s.append(col->to_string());
        s.append(", ");
      }
      s.append(")");

      if(m_root_mngr) {
        s.append("\n Last-CID=");
        s.append(std::to_string(m_last_cid));
      }
    }

    return s;
  }

  void range_loaded(RangeServerStatusPtr rs, client::ClientConPtr conn, 
                    DB::RangeBasePtr range, bool loaded, bool other_rs=false) {
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      cid_range_remove(rs->queued, range->cid, range->rid);
    }

    if(!loaded){
      std::lock_guard<std::mutex> lock(m_mutex);
      cid_ranges_add(m_not_assigned, range->cid, {range->rid});    

    } else if(!other_rs){
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      cid_ranges_add(rs->columns, range->cid, {range->rid});

    } else {
      bool found = false;
      if(conn != nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex_rs_status);
        for(auto host : m_rs_status){
          if(!has_endpoint(conn->endpoint_remote, host->endpoints))
            continue;
          cid_ranges_add(host->columns, range->cid, {range->rid});
          cid_range_remove(host->queued, range->cid, range->rid);
          found = true;
          break;
        }
      }
     if(!found) {
        // Req. this(conn) RS->unload
        RangeServerStatusPtr rs = rs_set({conn->endpoint_remote});
        cid_ranges_add(rs->columns, range->cid, {range->rid});
        cid_range_remove(rs->queued, range->cid, range->rid);
        
        HT_WARNF("RANGE-LOADED (cid=%d, rid=%d), for a nonexistent RANGESERVER (%s), new-rs_id=(%d) !", 
                  range->cid, range->rid, conn->endpoint_remote_str().c_str(), rs->rs_id);
     }
    }

    HT_DEBUGF("RANGE-LOADED state=%s other=%s rs_id=%d cid=%d rid=%d\n%s", 
              loaded?"TRUE":"FALSE", other_rs?"TRUE":"FALSE", 
              rs->rs_id, range->cid, range->rid, to_string().c_str());
  }

  private:


  RangeServerStatusPtr rs_set(EndPoints endpoints, uint64_t opt_id=0){
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);

    for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)) {
        if(h->ack) {
          // if h->endpoints != endpoints
          //    h->endpoints = endpoints;
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
    std::vector<int64_t> cols = EnvMngrRoleState::get()->get_active_columns();
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

    if(cid != 1 && !DB::Column::exists(cid))
      return false;
    if(only_exists_chk)
      return true;

    DB::ColumnPtr col = EnvColumns::get()->get_column(cid, true);
    if(!col->load()) {
      HT_ERRORF("Unable to load column=%d", cid);
    }

    // File::ColumnAssignments assignments.data (rs(endpoints)[rid])

    // for(auto assignment : assignments) {
    //   RangeServerStatusPtr rs = rs_set(assignment.endpoints)
    //   for(auto rid : assignment.ranges) {
    //     if(rs->has_range(cid, rid))
    //       continue;
    //     DB::RangeBasePtr range = col->get_range(rid, true);
    //     asio::post(*EnvIoCtx::io()->ptr(), [rs, range](){ 
    //       EnvRangeServers::get()->assign_range(rs, range); 
    //     });
    //   }
    // }

    // col->load_last_rid() // column.data (last_range)


    int64_t last_rid = col->get_last_rid();
    if(last_rid == -1){
      int err = Error::OK;
      FS::IdEntries_t entries;
      col->ranges_by_fs(err, entries);

      if(entries.size() == 0) 
        entries.push_back(1); // initialize 1st range
    
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        for(auto rid : entries)
          if(!rs_has_range(cid, rid))
            cid_ranges_add(m_not_assigned, cid, {rid});
      }

    } else {

      std::lock_guard<std::mutex> lock(m_mutex);
      for(int64_t rid=1; rid<=last_rid; rid++)
        if(!rs_has_range(cid, rid))
          cid_ranges_add(m_not_assigned, cid, {rid});
    }
    
    return true;
  }

  bool rs_has_range(int64_t cid, int64_t rid){
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);

    for(auto rs : m_rs_status){
      if(rs->has_range(cid, rid))
        return true;
    }
    return false;
  }

  static void cid_ranges_add(IdsColumnsRanges &cols, 
                             int64_t cid,  std::vector<int64_t> ranges){
    auto c_it = std::find_if(cols.begin(), cols.end(),  
                [cid](const IdsColumnRangesPtr& col){return col->cid == cid;});
    if(c_it != cols.end()){
      auto col = *c_it;
      for(auto rid : ranges) {
        auto r_it = std::find_if(col->ranges.begin(), col->ranges.end(),  
            [rid](const int64_t& rid_set){return rid_set == rid;});
        if(r_it != col->ranges.end())
          col->ranges.push_back(rid);
      }
    } else
      cols.push_back(std::make_shared<IdsColumnRanges>(cid, ranges));
  }
  
  static void cid_range_remove(IdsColumnsRanges &cols, 
                               int64_t cid, int64_t rid){
    for(auto it_c = cols.begin(); it_c < cols.end();){
      auto col = *it_c;
      if(col->cid == cid){
        for(auto it_r = col->ranges.begin(); it_r < col->ranges.end();){
          if((*it_r) == rid) {
            col->ranges.erase(it_r);
            break;
          }
          it_r++;
        }

        if(col->ranges.begin() == col->ranges.end())
          cols.erase(it_c);
        break;
      }
      it_c++;
    }
  }
  
  bool assign_ranges(){
    std::lock_guard<std::mutex> lock(m_mutex);

    DB::ColumnPtr  col;
    IdsColumnsRanges::iterator     col_it;
    std::vector<int64_t>::iterator r_it;
    
    while ((col_it = m_not_assigned.begin()) != m_not_assigned.end()) {
      auto nxt_col = *col_it;
      col = EnvColumns::get()->get_column(nxt_col->cid, true);

      while ((r_it = nxt_col->ranges.begin()) != nxt_col->ranges.end()) {

        DB::RangeBasePtr range = col->get_range(*r_it, true);
        if(range->deleted()) {
          nxt_col->ranges.erase(r_it);
          continue;
        }
        RangeServerStatusPtr rs = next_rs(range);
        if(rs == nullptr)
          return false;

        asio::post(*EnvIoCtx::io()->ptr(), [rs, range](){ 
          EnvRangeServers::get()->assign_range(rs, range); 
        });

        nxt_col->ranges.erase(r_it);
      };
      
      m_not_assigned.erase(col_it);
    };

    return true;
  }

  void assign_range(RangeServerStatusPtr rs, DB::RangeBasePtr range){
    
    client::ClientConPtr conn = EnvClients::get()->rs_service->get_connection(
                            rs->endpoints, std::chrono::milliseconds(5000), 1);      
    if(conn == nullptr){
        {
          std::lock_guard<std::mutex> lock(m_mutex_rs_status);
          rs->failures++;
        }
        EnvRangeServers::get()->range_loaded(rs, conn, range, false);
      return;
    }
    EnvClients::get()->rs_service->preserve(conn);
    
    Protocol::Req::Callback::LoadRange_t cb = [rs, conn, range](bool loaded){
      EnvRangeServers::get()->range_loaded(rs, conn, range, loaded); 
    };

    Files::RsDataPtr last_rs = range->get_last_rs();

    if(last_rs->endpoints.size() > 0 
       && !has_endpoint(rs->endpoints, last_rs->endpoints)){

      client::ClientConPtr old = EnvClients::get()->rs_service->get_connection(
        last_rs->endpoints, std::chrono::milliseconds(30000), 1);

      if(old != nullptr 
        && (std::make_shared<Protocol::Req::IsRangeLoaded>(
          old, range, [rs, old, conn, range, cb] (bool is_loaded) {
                       
            if(is_loaded) {
              EnvRangeServers::get()->range_loaded(rs, old, range, is_loaded, true);

            } else if(!(std::make_shared<Protocol::Req::LoadRange>(
                        conn, range, cb))->run()){
              cb(false);
            }
          }))->run()){

        EnvClients::get()->rs_service->preserve(old);
        return;
      }
    }

    if(!(std::make_shared<Protocol::Req::LoadRange>(conn, range, cb))->run())
      cb(false);
  }

  void set_unassigned(RangeServerStatusPtr host){
    for(auto col : host->columns) 
      cid_ranges_add(m_not_assigned, col->cid, col->ranges);
    
    for(auto col : host->queued) 
      cid_ranges_add(m_not_assigned, col->cid, col->ranges);
  }

  RangeServerStatusPtr next_rs(DB::RangeBasePtr range){
              
    RangeServerStatusPtr rs = nullptr;
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);
   
    size_t num_rs;
    size_t avg_ranges;
    while(rs == nullptr && m_rs_status.size() > 0){
    
      avg_ranges = 0;
      num_rs = 0;
      // avg_resource_ratio = 0;
      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++) {
        avg_ranges = avg_ranges*num_rs + (*it)->total_ranges();
        // resource_ratio = avg_resource_ratio*num_rs + (*it)->resource();
        avg_ranges /= ++num_rs;
        // avg_resource_ratio /= num_rs;
      }

      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
        rs = *it;
        if(avg_ranges < rs->total_ranges()) // *avg_resource_ratio
          continue;

        if(rs->failures == 255){
          set_unassigned(rs);
          m_rs_status.erase(it);
          rs = nullptr;
          continue;
        }
        break;
      }
    }

    if(rs != nullptr)
      cid_ranges_add(rs->queued, range->cid, {range->rid});
    return rs;
  }

  std::mutex          m_mutex;
  std::mutex          m_mutex_rs_status;
  
  TimerPtr            m_assign_timer; 

  IdsColumnsRanges    m_not_assigned;
  std::vector<RangeServerStatusPtr> m_rs_status;

  int64_t             m_last_cid = 0;
  bool                m_root_mngr = false;
};


}}}

#endif // swc_lib_manager_RangeServers_h