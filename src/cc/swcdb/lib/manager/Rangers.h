
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_Rangers_h
#define swc_lib_manager_Rangers_h
#include <memory>

#include "swcdb/lib/db/Files/RgrData.h"
#include "Ranger.h"

#include "swcdb/lib/db/Files/Schema.h"

#include "swcdb/lib/db/Protocol/Rgr/req/RangeLoad.h"
#include "swcdb/lib/db/Protocol/Rgr/req/RangeIsLoaded.h"
#include "swcdb/lib/db/Protocol/Rgr/req/AssignIdNeeded.h"
#include "swcdb/lib/db/Protocol/Rgr/req/ColumnUpdate.h"
#include "swcdb/lib/db/Protocol/Rgr/req/ColumnDelete.h"

#include "swcdb/lib/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/lib/db/Protocol/Mngr/req/RgrUpdate.h"
#include "swcdb/lib/db/Protocol/Mngr/req/ColumnUpdate.h"


namespace SWC { namespace server { namespace Mngr {

class Rangers;
typedef std::shared_ptr<Rangers> RangersPtr;
}}

namespace Env {
class Rangers {
  
  public:

  static void init() {
    m_env = std::make_shared<Rangers>();
  }

  static server::Mngr::RangersPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_rangers;
  }

  Rangers() 
    : m_rangers(std::make_shared<server::Mngr::Rangers>()) {}

  virtual ~Rangers(){}

  private:
  server::Mngr::RangersPtr               m_rangers = nullptr;
  inline static std::shared_ptr<Rangers> m_env = nullptr;
};
}



namespace server { namespace Mngr {


class Rangers {

  struct ColumnActionReq {
    Protocol::Mngr::Params::ColumnMng params;
    std::function<void(int)>    cb;
  };
  struct ColumnFunction {
    Protocol::Mngr::Params::ColumnMng::Function func;
    int64_t cid;
  };

  public:
  Rangers()
    : m_assign_timer(
        std::make_shared<asio::high_resolution_timer>(*Env::IoCtx::io()->ptr())
      ),
      cfg_rgr_failures(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.Rgr.remove.failures")),
      cfg_delay_rgr_chg(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.delay.onRangerChange")),
      cfg_delay_cols_init(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.delay.afterColumnsInit")),
      cfg_chk_assign(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.interval.check")),
      cfg_assign_due(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.due")) { 
  }

  void new_columns() {
    m_columns_set = false;
    check_assignment_timer(500);
  }
  
  void require_sync() {
    rs_changes(m_rgr_status, true);
    
    if(m_root_mngr)
      columns_load();
    else
      column_update(
        Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD_ALL, 
        DB::Schema::make(-1, "")
      );
        
  }

  virtual ~Rangers(){}

  void is_active(int& err, int64_t cid, bool for_schema=false) {
    if(!Env::MngrRole::get()->is_active(cid)){
      err = Error::MNGR_NOT_ACTIVE;
      return;
    }
    if(m_root_mngr && !m_columns_set) {
      err = Error::MNGR_NOT_INITIALIZED;
      return;  
    }
    if(for_schema)
      return;

    ColumnPtr col = Env::MngrColumns::get()->get_column(err, cid, false);
    if(col == nullptr) {
      err = Error::COLUMN_NOT_EXISTS;
      return;  
    }
    col->state(err);
  }

  // Columns Actions
  void column_action(ColumnActionReq new_req){
    {
      std::lock_guard<std::mutex> lock(m_mutex_columns);
      m_column_actions.push(new_req);
      if(m_column_actions.size() > 1)
        return;
    }
    
    asio::post(*Env::IoCtx::io()->ptr(), 
      [](){
         Env::Rangers::get()->column_actions_run();
      }
    );
  }

  void update_status(Protocol::Mngr::Params::ColumnMng::Function func, DB::SchemaPtr schema, 
                     int err, bool initial=false){
    HT_ASSERT(schema->cid != DB::Schema::NO_CID);

    if(!initial && m_root_mngr) {
      update_status_ack(func, schema, err);
      return;
    }

    if(manage(schema->cid)){

      int err = Error::OK;
      if(func == Protocol::Mngr::Params::ColumnMng::Function::DELETE) {
        column_delete(err, schema->cid);
        
      } else if(func == Protocol::Mngr::Params::ColumnMng::Function::CREATE || 
                func == Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD) {

        auto co_func = (
          Protocol::Mngr::Params::ColumnMng::Function)(((uint8_t)func)+1);

        if(Env::MngrColumns::get()->is_an_initialization(err, schema->cid) 
                                     && err == Error::OK) {
          {
            std::lock_guard<std::mutex> lock(m_mutex_columns);
            m_cid_pending_load.push_back({.func=co_func, .cid=schema->cid});
          }
          if(!m_root_mngr)
            Env::Schemas::get()->replace(schema);;
          assign_ranges();

        } else if(m_root_mngr){
          update_status(co_func, schema, err);

        } else {
          column_update(co_func, schema, err);
        }
        
      } else if (func == Protocol::Mngr::Params::ColumnMng::Function::MODIFY){

        if(m_root_mngr) {
          if(!update_schema_rgr(schema, true))
            update_status(
              Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY, 
              schema, err);

        } else if(!update_schema(schema)) {
          column_update(
            Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY, 
            schema, err);
        }
      }
      return;
    }
    
    column_update(func, schema, err);
  }

  // Ranger Actions
  void update_status(RangerList new_rgr_status, bool sync_all){
    if(m_root_mngr && !sync_all)
      return;

    RangerList changed;

    {
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
      RangerPtr h;
      bool found;
      bool chg;

      for(auto& rs_new : new_rgr_status){
        found = false;
        for(auto it=m_rgr_status.begin();it<m_rgr_status.end(); it++){
          h = *it;
          if(!has_endpoint(h->endpoints, rs_new->endpoints))
            continue;

          chg = false;
          if(rs_new->id != h->id){ 
            if(m_root_mngr)
              rs_new->id = rs_set(rs_new->endpoints, rs_new->id)->id;
          
            if(m_root_mngr && rs_new->id != h->id)
              Env::MngrColumns::get()->change_rgr(h->id, rs_new->id);

            h->id = rs_new->id;
            chg = true;
          }
          for(auto& endpoint: rs_new->endpoints){
            if(!has_endpoint(endpoint, h->endpoints)){
              h->set(rs_new->endpoints);
              chg = true;
              break;
            }
          }
          for(auto& endpoint: h->endpoints){
            if(!has_endpoint(endpoint, rs_new->endpoints)){
              h->set(rs_new->endpoints);
              chg = true;
              break;
            }
          }
          if(rs_new->state == Ranger::State::ACK) {
            if(rs_new->state != h->state) {
              h->state = Ranger::State::ACK;
              chg = true;
            }
          } else {
            Env::MngrColumns::get()->set_rgr_unassigned(h->id);
            m_rgr_status.erase(it);
            chg = true;
          }

          if(chg && !sync_all)
            changed.push_back(rs_new);
          found = true;
          break;
        }

        if(!found){
          if(rs_new->state == Ranger::State::ACK){
            rs_new->init_queue();
            m_rgr_status.push_back(rs_new);
            if(!sync_all)
              changed.push_back(rs_new);
          }
        }
      }
    }
    if(!changed.empty())
      std::cout << " update_status: ";

    rs_changes(sync_all ? m_rgr_status : changed, sync_all && !m_root_mngr);
  }

  void rs_get(const uint64_t id, EndPoints& endpoints){
    std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
    for(auto& rgr : m_rgr_status) {
      if(rgr->id == id){
        if(rgr->state == Ranger::State::ACK)
          endpoints = rgr->endpoints;
        break;
      }
    }
  }

  uint64_t rs_set_id(const EndPoints& endpoints, uint64_t opt_id=0){
    std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
    return rs_set(endpoints, opt_id)->id;
  }

  bool rs_ack_id(uint64_t id, const EndPoints& endpoints){
    bool ack = false;
    RangerPtr new_ack = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
    
      for(auto& h : m_rgr_status){
        if(has_endpoint(h->endpoints, endpoints) && id == h->id){
          if(h->state != Ranger::State::ACK)
            new_ack = h;
          h->state = Ranger::State::ACK;
          ack = true;
          break;
        }
      }
    }

    if(new_ack != nullptr) {
      RangerList hosts({new_ack});
      rs_changes(hosts);
    }
    return ack;
  }

  uint64_t rs_had_id(uint64_t id, const EndPoints& endpoints){
    bool new_id_required = false;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);

      for(auto& h : m_rgr_status){
        if(id == h->id){
          if(has_endpoint(h->endpoints, endpoints))
            return 0; // zero=OK
          new_id_required = true;
          break;
        }
      }
    }
    
    return rs_set_id(endpoints, new_id_required ? 0 : id);
  }

  void rs_shutdown(uint64_t id, const EndPoints& endpoints){
    RangerPtr removed = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
      for(auto it=m_rgr_status.begin();it<m_rgr_status.end(); it++){
        auto h = *it;
        if(has_endpoint(h->endpoints, endpoints)){
          removed = h;
          m_rgr_status.erase(it);
          removed->state = Ranger::State::REMOVED;
          Env::MngrColumns::get()->set_rgr_unassigned(removed->id);
          break;
        }
      }
    }
    if(removed != nullptr){
      RangerList hosts({removed});
      rs_changes(hosts);
    }   
  }

  std::string to_string(){
    std::string s(Env::MngrColumns::get()->to_string());

    s.append("\nRangers:");
    std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
    for(auto& h : m_rgr_status) {
      s.append("\n ");
      s.append(h->to_string());
    }
    return s;
  }

  void stop() {
    m_run = false;
    {
      std::lock_guard<std::mutex> lock(m_mutex_timer);
      m_assign_timer->cancel();
    }
    {
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
      for(auto& h : m_rgr_status)
        asio::post(*Env::IoCtx::io()->ptr(), [h](){ h->stop(); });
    }
  }

  void assign_range_chk_last(int err, RangerPtr rs_chk) {
    Protocol::Common::Req::ConnQueue::ReqBase::Ptr req;
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
        if(!rs_chk->pending_id_pop(req))
          return;
      }

      auto qreq = std::dynamic_pointer_cast<
        Protocol::Rgr::Req::AssignIdNeeded>(req);
      if(err == Error::OK) 
        range_loaded(qreq->rs_nxt, qreq->range, Error::RS_NOT_READY);
      else
        assign_range(qreq->rs_nxt, qreq->range);
    }
  }

  void assign_range(RangerPtr rgr, Range::Ptr range){
    rgr->put(std::make_shared<Protocol::Rgr::Req::RangeLoad>(rgr, range));
  }

  void range_loaded(RangerPtr rgr, Range::Ptr range, 
                    int err, bool failure=false, bool verbose=true) {
    bool run_assign = m_assignments-- > cfg_assign_due->get();           

    if(!range->deleted()) {
      if(err != Error::OK){
        rgr->total_ranges--;
        if(failure)
          rgr->failures++;

        range->set_state(Range::State::NOTSET, 0); 
        if(!run_assign) 
          check_assignment_timer(2000);

      } else {
        rgr->failures=0;
        range->set_state(Range::State::ASSIGNED, rgr->id); 
        range->clear_last_rgr();
        // adjust rgr->resource
        // ++ mng_inchain - req. MngrRsResource
      
        ColumnFunction pending;
        while(column_load_pending(range->cid, pending))
          column_update(pending.func, Env::Schemas::get()->get(pending.cid));
      }
      if(verbose)
        HT_INFOF("RANGE-STATUS %d(%s), %s", 
                  err, Error::get_text(err), range->to_string().c_str());
    }

    if(run_assign)
      assign_ranges();
  }

  void column_delete(int &err, int64_t cid, int64_t id) {
    ColumnPtr col = Env::MngrColumns::get()->get_column(err, cid, false);
    if(col == nullptr || col->finalize_remove(err, id)) {
      Env::MngrColumns::get()->remove(err, cid);
      DB::SchemaPtr schema = Env::Schemas::get()->get(cid);
      if(schema != nullptr)
        column_update(
          Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_DELETE,
          schema, err);
      if(!m_root_mngr)
        Env::Schemas::get()->remove(cid);
    }
  }

  void update_schema(int &err, RangerPtr rgr, 
                     DB::SchemaPtr schema, bool failure) {
                       
    if(err == Error::OK)
      Env::MngrColumns::get()->get_column(err, schema->cid, false)
                             ->change_rgr_schema(rgr->id, schema->revision);
    else if(failure) {
      rgr->failures++;
      return;
    }

    if(!update_schema_rgr(schema, false))
      column_update(
        Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY,
        schema, 
        err
      );
  }


  private:

  bool manage(int64_t cid){
    std::vector<int64_t> cols;
    Env::MngrRole::get()->get_active_columns(cols);
    if(cols.size() == 0){
      // if decommissioned
      if(m_columns_set){
        HT_INFO("Manager has been decommissioned");
        m_columns_set = false;
        m_root_mngr = false;
        Env::MngrColumns::get()->reset();
      }
      return false; 
    }

    if(*cols.begin() == 0 && *(cols.end()-1) < cid) // from till-end
      return true;

    return std::find_if(cols.begin(), cols.end(),  
          [cid](const int64_t& cid_set){return cid_set == cid;}) != cols.end();
  }

  void check_assignment_timer(uint32_t t_ms = 10000) {
    if(!m_run)
      return;

    std::lock_guard<std::mutex> lock(m_mutex_timer);

    auto set_in = std::chrono::milliseconds(t_ms);
    auto set_on = m_assign_timer->expires_from_now();
    if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
      return;
    m_assign_timer->cancel();
    m_assign_timer->expires_from_now(set_in);

    m_assign_timer->async_wait(
      [](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          Env::Rangers::get()->check_assignment();
        }
    }); 

    if(t_ms > 10000) {
      std::cout << to_string() << "\n";
    }
    HT_DEBUGF("Rangers ranges check_assignment scheduled in ms=%d", t_ms);
  }

  void check_assignment(){
    if(!m_run)
      return;

     if(!initialize_cols()){
      check_assignment_timer(cfg_delay_cols_init->get());
      return;
    }

    // if(m_root_mngr) (scheduled on column changes ) + chk(cid) LOAD_ACK
    assign_ranges();
  }

  bool initialize_cols(){
    if(m_columns_set){
      if(m_root_mngr)
        columns_load_chk_ack();
      return true;
    }

    std::vector<int64_t> cols;
    Env::MngrRole::get()->get_active_columns(cols);
    if(cols.size() == 0) {
      m_columns_set = false;
      return false; 
    }
    m_root_mngr = manage(1);
    if(!m_root_mngr || m_columns_set)
      return true;

    {
      std::lock_guard<std::mutex> lock1(m_mutex);
      std::lock_guard<std::mutex> lock2(m_mutex_columns);
    
      int err = Error::OK;
      FS::IdEntries_t entries;
      Columns::columns_by_fs(err, entries); 
      if(err !=  Error::OK) {
        if(err != ENOENT)
          return false;
        err = Error::OK;
      }
      if(entries.empty()){ // initialize sys-columns
        for(int cid=1;cid<=3;cid++){
          Column::create(err, cid);
          entries.push_back(cid);
        }
      }
      
      int32_t hdlrs = Env::IoCtx::io()->get_size()/4+1;
      int32_t vol = entries.size()/hdlrs+1;
      std::atomic<int64_t> pending = 0;
      while(!entries.empty()) {
        FS::IdEntries_t hdlr_entries;
        for(auto n=0; n < vol && !entries.empty(); n++) {
          hdlr_entries.push_back(entries.front());
          entries.erase(entries.begin());
        }
        if(hdlr_entries.empty())
          break;

        pending++;
        asio::post(*Env::IoCtx::io()->ptr(), 
          [&pending, entries=hdlr_entries]() { 
            DB::SchemaPtr schema;
            for(auto cid : entries) {
              int err = Error::OK;
              schema = Files::Schema::load(err, cid);
              if(err == Error::OK)
                Env::Schemas::get()->add(err, schema);
              if(err !=  Error::OK)
                HT_WARNF("Schema cid=%d err=%d(%s)", cid, err, Error::get_text(err));
            }
            pending--;
          }
        );
      }

      while(pending > 0) // keep_locking
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    m_columns_set = true;

    columns_load();
    return true;
  }

  void assign_ranges() {
    {
      std::lock_guard<std::mutex> lock(m_mutex_assign);
      if(m_runs_assign) 
        return;
      m_runs_assign = true;
    }
    
    asio::post(*Env::IoCtx::io()->ptr(), 
      [](){ Env::Rangers::get()->assign_ranges_run(); }
    );
  }

  void assign_ranges_run() {
    int err;
    Range::Ptr range;

    for(;;){
      std::lock_guard<std::mutex> lock(m_mutex_assign);
      {
        std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
        if(m_rgr_status.empty() || !m_run){
          m_runs_assign = false;
          check_assignment_timer();
          return;
        }
      }

      if((range = Env::MngrColumns::get()->get_next_unassigned()) == nullptr){
        m_runs_assign = false;
        break;
      }

      err = Error::OK;
      Files::RgrDataPtr last_rgr = range->get_last_rgr(err);
      RangerPtr rgr = nullptr;
      next_rgr(last_rgr, rgr);
      if(rgr == nullptr){
        m_runs_assign = false;
        check_assignment_timer();
        return;
      }

      range->set_state(Range::State::QUEUED, rgr->id);
      assign_range(rgr, range, last_rgr);
      if(++m_assignments > cfg_assign_due->get()){
        m_runs_assign = false;
        return;
      }
    }

    // balance/check-assigments if not runs :for ranger cid-rid state
    check_assignment_timer(cfg_chk_assign->get());
  }

  void next_rgr(Files::RgrDataPtr &last_rgr, RangerPtr &rs_set){
    std::lock_guard<std::mutex> lock(m_mutex_rgr_status);

    if(last_rgr->endpoints.size() > 0) {
       for(auto& rgr : m_rgr_status) {
          if(rgr->state == Ranger::State::ACK
            && rgr->failures < cfg_rgr_failures->get() 
            && has_endpoint(rgr->endpoints, last_rgr->endpoints)){
            rs_set = rgr;
            last_rgr = nullptr;
            break;
          }
       }
    } else 
      last_rgr = nullptr;
    
    size_t num_rgr;
    size_t avg_ranges;
    RangerPtr rgr;

    while(rs_set == nullptr && m_rgr_status.size() > 0){
      avg_ranges = 0;
      num_rgr = 0;
      // avg_resource_ratio = 0;
      for(auto it=m_rgr_status.begin();it<m_rgr_status.end(); it++) {
        rgr = *it;
        if(rgr->state != Ranger::State::ACK)
          continue;
        avg_ranges = avg_ranges*num_rgr + rgr->total_ranges;
        // resource_ratio = avg_resource_ratio*num_rgr + rgr->resource();
        avg_ranges /= ++num_rgr;
        // avg_resource_ratio /= num_rgr;
      }

      for(auto it=m_rgr_status.begin();it<m_rgr_status.end(); it++){
        rgr = *it;
        if(rgr->state != Ranger::State::ACK || avg_ranges < rgr->total_ranges)
          continue;

        if(rgr->failures >= cfg_rgr_failures->get()){
          m_rgr_status.erase(it);
          Env::MngrColumns::get()->set_rgr_unassigned(rgr->id);
          continue;
        }
        rs_set = rgr;
        break;
      }
    }

    if(rs_set != nullptr)
      rs_set->total_ranges++;
    return;
  }

  void assign_range(RangerPtr rgr, Range::Ptr range, 
                    Files::RgrDataPtr last_rgr){
    if(last_rgr == nullptr){
      assign_range(rgr, range);
      return;
    }

    bool id_due;
    RangerPtr rs_last = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
      for(auto& rs_chk : m_rgr_status) {
        if(has_endpoint(rs_chk->endpoints, last_rgr->endpoints)){
          rs_last = rs_chk;
          id_due = rs_last->state == Ranger::State::AWAIT;
          rs_last->state = Ranger::State::AWAIT;
          break;
        }
      }
    }
    if(rs_last == nullptr){
      rs_last = std::make_shared<Ranger>(0, last_rgr->endpoints);
      rs_last->init_queue();
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
      std::cout <<  " assign_range, rs_last " << rs_last->to_string() << "\n";
      m_rgr_status.push_back(rs_last);
      rs_last->state = Ranger::State::AWAIT;
      id_due = false;
    }
    
    auto req = std::make_shared<Protocol::Rgr::Req::AssignIdNeeded>(
      rs_last, rgr, range);
    if(id_due)
      rs_last->pending_id(req);
    else
      rs_last->put(req);
  }

  RangerPtr rs_set(const EndPoints& endpoints, uint64_t opt_id=0){

    for(auto it=m_rgr_status.begin();it<m_rgr_status.end(); it++){
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)) {
        if(h->state == Ranger::State::ACK) {
          h->set(endpoints);
          return h;
        } else {
          Env::MngrColumns::get()->set_rgr_unassigned(h->id);
          m_rgr_status.erase(it);
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
      for(auto& h : m_rgr_status){
        if(nxt == h->id) {
          ok = false;
          break;
        };
      }
    } while(!ok);

    RangerPtr h = std::make_shared<Ranger>(nxt, endpoints);
    h->init_queue();
    m_rgr_status.push_back(h);
    return h;
  }
  
  void rs_changes(RangerList& hosts, bool sync_all=false){
    {
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
      if(hosts.size() > 0){
        Env::MngrRole::get()->req_mngr_inchain(
          std::make_shared<Protocol::Mngr::Req::RgrUpdate>(
          hosts, sync_all));

        std::cout << " rs_changes: \n";
        for(auto& h : hosts)
          std::cout << " " << h->to_string() << "\n";
      }
    }
    
    if(Env::MngrRole::get()->has_active_columns())
      check_assignment_timer(cfg_delay_rgr_chg->get());
  }
  
  void columns_load(){
    std::vector<DB::SchemaPtr> entries;
    Env::Schemas::get()->all(entries);
    for(auto& schema : entries) {
      HT_ASSERT(schema->cid != DB::Schema::NO_CID);
      update_status(Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD, schema,
                    Error::OK, true);
    }
  }

  void columns_load_chk_ack(){
    std::lock_guard<std::mutex> lock(m_mutex_columns);
    for(auto& ack : m_cid_pending_load){
      if(ack.func == Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_LOAD){
        column_update(
          Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD,  
          Env::Schemas::get()->get(ack.cid)
        );
      }
    }
  }

  bool column_load_pending(int64_t cid, ColumnFunction &pending) {
    std::lock_guard<std::mutex> lock(m_mutex_columns);

    auto it = std::find_if(m_cid_pending_load.begin(), 
                           m_cid_pending_load.end(),  
                          [cid](const ColumnFunction& pending) 
                          {return pending.cid == cid;});
    if(it == m_cid_pending_load.end())
      return false;
    pending = *it;
    m_cid_pending_load.erase(it);
    return true;
  }

  int64_t get_next_cid() {
    int64_t cid = 4;
    while(Env::Schemas::get()->get(++cid) != nullptr);
    // if schema does exist on fs (? sanity-check) 
    return cid;
  }

  void column_create(int &err, DB::SchemaPtr &schema){
    int64_t cid = get_next_cid();
    if(cid < 0) {
      err = Error::COLUMN_REACHED_ID_LIMIT;
      return;
    } 
    
    Column::create(err, cid);
    if(err != Error::OK)
      return;

    DB::SchemaPtr schema_save = DB::Schema::make(
      cid, schema, schema->revision != 0 ? schema->revision : Time::now_ns());
    HT_ASSERT(schema_save->cid != DB::Schema::NO_CID);
    
    Files::Schema::save_with_validation(err, schema_save);
    if(err == Error::OK) 
      Env::Schemas::get()->add(err, schema_save);

    if(err == Error::OK) 
      schema = Env::Schemas::get()->get(schema_save->cid);
    else 
      Column::remove(err, cid);
  }
  
  void column_update_schema(int &err, DB::SchemaPtr &schema, 
                            DB::SchemaPtr old) {

    if(old->col_name.compare(schema->col_name) != 0 
      && Env::Schemas::get()->get(schema->col_name) != nullptr){
      err = Error::COLUMN_SCHEMA_NAME_EXISTS;
      return;
    }

    DB::SchemaPtr schema_save = DB::Schema::make(
      schema->cid == DB::Schema::NO_CID? old->cid: schema->cid,
      schema, 
      schema->revision != 0 ? 
      schema->revision : Time::now_ns());

    HT_ASSERT(schema_save->cid != DB::Schema::NO_CID);

    if(schema->equal(schema_save, false))
      err = Error::COLUMN_SCHEMA_NOT_DIFFERENT;

    Files::Schema::save_with_validation(err, schema_save);
    if(err == Error::OK) {
      Env::Schemas::get()->replace(schema_save);
      schema = Env::Schemas::get()->get(schema_save->cid);
    }
  }

  void column_delete(int &err, int64_t cid) {

    ColumnPtr col = Env::MngrColumns::get()->get_column(err, cid, false);
    if(col == nullptr){
      column_delete(err, cid, 0);
      return;
    }
    if(!col->do_remove())
      return;
    HT_DEBUGF("DELETING cid=%d", cid);
    
    std::vector<uint64_t> rgr_ids;
    col->assigned(rgr_ids);
    if(rgr_ids.empty()){
      column_delete(err, cid, 0);
      return;
    }

    for(auto id : rgr_ids){
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);
      for(auto& rgr : m_rgr_status){
        if(id != rgr->id)
          continue;
        rgr->total_ranges--;
        rgr->put(std::make_shared<Protocol::Rgr::Req::ColumnDelete>(rgr, cid));
      }
    }
  }

  void column_actions_run(){
    
    ColumnActionReq req;
    int err;
    for(;;){
      {
        std::lock_guard<std::mutex> lock(m_mutex_columns);
        req = m_column_actions.front();
      }

      if(!m_run)
        err = Error::SERVER_SHUTTING_DOWN;

      else if(!m_columns_set)
        err = Error::MNGR_NOT_INITIALIZED;

      else if(req.params.schema->col_name.length() == 0)
        err = Error::COLUMN_SCHEMA_NAME_EMPTY;

      else {
        err = Error::OK;
        std::lock_guard<std::mutex> lock(m_mutex);
        DB::SchemaPtr schema = Env::Schemas::get()->get(req.params.schema->col_name);
        if(schema == nullptr && req.params.schema->cid != DB::Schema::NO_CID)
          schema = Env::Schemas::get()->get(req.params.schema->cid);

        switch(req.params.function){
          case Protocol::Mngr::Params::ColumnMng::Function::CREATE: {
            if(schema != nullptr)
              err = Error::COLUMN_SCHEMA_NAME_EXISTS;
            else
              column_create(err, req.params.schema);  
            break;
          }
          case Protocol::Mngr::Params::ColumnMng::Function::MODIFY: {
            if(schema == nullptr) 
              err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
            else
              column_update_schema(err, req.params.schema, schema);
            break;
          }
          case Protocol::Mngr::Params::ColumnMng::Function::DELETE: {
            if(schema == nullptr) 
              err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
            else 
              req.params.schema = schema;
            break;
          }
          default:
            err = Error::NOT_IMPLEMENTED;
            break;
        }
      }

      if(err == Error::OK){
        {
          std::lock_guard<std::mutex> lock(m_mutex_columns);
          m_cid_pending.push_back(req);
        }
        HT_ASSERT(req.params.schema->cid != DB::Schema::NO_CID);
        update_status(req.params.function, req.params.schema, err, true);

      } else {
        try{
          req.cb(err);
        } catch (std::exception &e) {
          HT_ERRORF("Column Action cb err=%s func=%d %s", 
                    e.what(), req.params.function, 
                    req.params.schema->to_string().c_str());
        }
      }
      
      {
        std::lock_guard<std::mutex> lock(m_mutex_columns);
        m_column_actions.pop();
        if(m_column_actions.empty())
          return;
      }
    }
  }

  bool update_schema(DB::SchemaPtr schema) {
    DB::SchemaPtr existing = Env::Schemas::get()->get(schema->cid);
    if(existing == nullptr || !existing->equal(schema)) {
      Env::Schemas::get()->replace(schema);
      return update_schema_rgr(schema, true);
    }
    return false;
  }

  bool update_schema_rgr(DB::SchemaPtr schema, bool ack_required) {
    std::vector<uint64_t> rgr_ids;
    int err = Error::OK;
    Env::MngrColumns::get()->get_column(err, schema->cid, false)
                           ->need_schema_sync(schema->revision, rgr_ids);
    bool undergo = false;
    for(auto& id : rgr_ids) {
      std::lock_guard<std::mutex> lock(m_mutex_rgr_status);

      for(auto& rgr : m_rgr_status) {
        if(rgr->failures < cfg_rgr_failures->get() 
          && rgr->state == Ranger::State::ACK && rgr->id == id) {
          undergo = true;
          if(ack_required){
            rgr->put(
              std::make_shared<Protocol::Rgr::Req::ColumnUpdate>(rgr, schema));
          }
        }
      }
    }
    return undergo;
  }

  void column_update(Protocol::Mngr::Params::ColumnMng::Function func,
                     DB::SchemaPtr schema, int err=Error::OK){
    Env::MngrRole::get()->req_mngr_inchain(
      std::make_shared<Protocol::Mngr::Req::ColumnUpdate>(func, schema, err));
  }

  void update_status_ack(Protocol::Mngr::Params::ColumnMng::Function func,
                         DB::SchemaPtr schema, int err){
    if(err == Error::OK){
      switch(func){
        case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD_ALL: {
          columns_load();
          return;
        }
        case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_LOAD: {
          ColumnFunction pending;
          while(column_load_pending(schema->cid, pending));
          return;
        }
        case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_CREATE: {
          break; 
        }
        case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY: {
          break; 
        }
        case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_DELETE: {
          std::lock_guard<std::mutex> lock(m_mutex);

          if(Env::Schemas::get()->get(schema->cid) == nullptr)
            err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
          else if(!m_run)
            err = Error::SERVER_SHUTTING_DOWN;
          else {  
            Column::remove(err, schema->cid);
            if(err == Error::OK)
              Env::Schemas::get()->remove(schema->cid);  
          }
          break;
        }
        default:
          return;
      }
    }

    auto co_func = (Protocol::Mngr::Params::ColumnMng::Function)(((uint8_t)func)-1);

    if(err != Error::OK)
      HT_DEBUGF("COLUMN-ACK %s func=%d, err=%d(%s)", 
                schema->to_string().c_str(), co_func, 
                err, Error::get_text(err));
                  
    ColumnActionReq req;
    for(;;){
      {
        std::lock_guard<std::mutex> lock(m_mutex_columns);
        auto it = std::find_if(m_cid_pending.begin(), m_cid_pending.end(),  
            [co_func, cid=schema->cid](const ColumnActionReq& req)
            {return req.params.schema->cid == cid 
                    && req.params.function == co_func;});
            
        if(it == m_cid_pending.end())
          break;  
        req = *it;
        m_cid_pending.erase(it);
      }

      try {
        req.cb(err);
      } catch (std::exception &e) {
        HT_ERRORF("Column Pending func=%d cb err=%s %s", 
                  func, e.what(), req.params.schema->to_string().c_str());
      }
    }
  }

  std::mutex                    m_mutex_timer;
  TimerPtr                      m_assign_timer; 

  std::mutex                    m_mutex;

  std::mutex                    m_mutex_columns;
  std::queue<ColumnActionReq>   m_column_actions;
  std::vector<ColumnActionReq>  m_cid_pending;
  std::vector<ColumnFunction>   m_cid_pending_load;

  std::mutex                    m_mutex_rgr_status;
  RangerList                    m_rgr_status;

  std::mutex                    m_mutex_assign;
  bool                          m_runs_assign = false;
  std::atomic<int>              m_assignments = 0; 

  std::atomic<bool>             m_columns_set = false;
  std::atomic<bool>             m_root_mngr = false;
  std::atomic<bool>             m_run = true; 
  

  const gInt32tPtr cfg_rgr_failures;
  const gInt32tPtr cfg_delay_rgr_chg;
  const gInt32tPtr cfg_delay_cols_init;
  const gInt32tPtr cfg_chk_assign;
  const gInt32tPtr cfg_assign_due;
  
};

}} // namespace server



void Protocol::Rgr::Req::RangeLoad::loaded(int err, bool failure, 
                                           const DB::Cells::Interval& intval) {
  std::cout << " Protocol::Rgr::Req::RangeLoad::loaded" << "\n";
  auto col = Env::MngrColumns::get()->get_column(err, range->cid, false);
  if(col == nullptr){
    Env::Rangers::get()->range_loaded(
      rgr, range, Error::COLUMN_MARKED_REMOVED, failure);
    return;
  }
  if(schema != nullptr && err == Error::OK)
    col->change_rgr_schema(rgr->id, schema->revision);
                           
  else if(err == Error::COLUMN_SCHEMA_MISSING)
    col->remove_rgr_schema(rgr->id);

  Env::Rangers::get()->range_loaded(rgr, range, err, failure, false);
  col->sort(range, intval);
  HT_INFOF("RANGE-STATUS %d(%s), %s", 
            err, Error::get_text(err), range->to_string().c_str());
}

void Protocol::Rgr::Req::ColumnUpdate::updated(int err, bool failure) {
  std::cout << " Protocol::Rgr::Req::ColumnUpdate::updated" << "\n";
  Env::Rangers::get()->update_schema(err, rgr, schema, failure);
  
 if(failure)
    request_again();
}

void Protocol::Rgr::Req::AssignIdNeeded::rsp(int err) {
  std::cout << " Protocol::Rgr::Req::AssignIdNeeded::rsp" << "\n";
  
  if(err == Error::OK) 
    // RsId assignment on the way, put range back as not assigned 
    Env::Rangers::get()->range_loaded(
      rs_nxt, range, Error::RS_NOT_READY);
  else
    Env::Rangers::get()->assign_range(
      rs_nxt, range);

    // the same cond to reqs pending_id
  Env::Rangers::get()->assign_range_chk_last(err, rs_chk);
}

void Protocol::Rgr::Req::ColumnDelete::remove(int err) {
  Env::Rangers::get()->column_delete(err, cid, rgr->id);  
}


}

#endif // swc_lib_manager_Rangers_h