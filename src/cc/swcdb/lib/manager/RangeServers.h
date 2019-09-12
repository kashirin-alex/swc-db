
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServers_h
#define swc_lib_manager_RangeServers_h
#include <memory>

#include "swcdb/lib/db/Files/RsData.h"
#include "RsStatus.h"

#include "swcdb/lib/db/Files/Schema.h"

#include "swcdb/lib/db/Protocol/req/RsRangeLoad.h"
#include "swcdb/lib/db/Protocol/req/RsRangeIsLoaded.h"
#include "swcdb/lib/db/Protocol/req/RsIdReqNeeded.h"
#include "swcdb/lib/db/Protocol/req/RsColumnDelete.h"
#include "swcdb/lib/db/Protocol/req/RsUpdateSchema.h"

#include "swcdb/lib/db/Protocol/params/ColumnMng.h"
#include "swcdb/lib/db/Protocol/req/MngrUpdateRangeServers.h"
#include "swcdb/lib/db/Protocol/req/MngrColumnUpdate.h"


namespace SWC { namespace server { namespace Mngr {

class RangeServers;
typedef std::shared_ptr<RangeServers> RangeServersPtr;
}}

namespace Env {
class RangeServers {
  
  public:

  static void init() {
    m_env = std::make_shared<RangeServers>();
  }

  static server::Mngr::RangeServersPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_rangeservers;
  }

  RangeServers() 
    : m_rangeservers(std::make_shared<server::Mngr::RangeServers>()) {}

  virtual ~RangeServers(){}

  private:
  server::Mngr::RangeServersPtr               m_rangeservers = nullptr;
  inline static std::shared_ptr<RangeServers> m_env = nullptr;
};
}



namespace server { namespace Mngr {


class RangeServers {

  struct ColumnActionReq {
    Protocol::Params::ColumnMng params;
    std::function<void(int)>    cb;
  };
  struct ColumnFunction {
    Protocol::Params::ColumnMng::Function func;
    int64_t cid;
  };

  public:
  RangeServers()
    : m_assign_timer(
        std::make_shared<asio::high_resolution_timer>(*Env::IoCtx::io()->ptr())
      ),
      cfg_rs_failures(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.RS.remove.failures")),
      cfg_delay_rs_chg(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.delay.onRangeServerChange")),
      cfg_delay_cols_init(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.delay.afterColumnsInit")),
      cfg_chk_assign(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.interval.check")),
      cfg_assign_due(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.due")) { 
  }

  void new_columns() {
    {
      std::lock_guard<std::mutex> lock(m_mutex_columns);
      m_columns_set = false;
    }
    check_assignment_timer(500);
  }
  
  void require_sync() {
    rs_changes(m_rs_status, true);
    
    if(m_root_mngr)
      columns_load();
    else
      column_update(
        Protocol::Params::ColumnMng::Function::INTERNAL_LOAD_ALL, 
        DB::Schema::make(-1, "")
      );
        
  }

  virtual ~RangeServers(){}


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
         Env::RangeServers::get()->column_actions_run();
      }
    );
  }

  void update_status(Protocol::Params::ColumnMng::Function func, DB::SchemaPtr schema, 
                     int err, bool initial=false){
    HT_ASSERT(schema->cid != 0);

    if(!initial && m_root_mngr) {
      update_status_ack(func, schema, err);
      return;
    }

    if(manage(schema->cid)){

      int err = Error::OK;
      if(func == Protocol::Params::ColumnMng::Function::DELETE) {
        column_delete(err, schema->cid);
        
      } else if(func == Protocol::Params::ColumnMng::Function::CREATE || 
                func == Protocol::Params::ColumnMng::Function::INTERNAL_LOAD) {

        auto co_func = (
          Protocol::Params::ColumnMng::Function)(((uint8_t)func)+1);

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
        
      } else if (func == Protocol::Params::ColumnMng::Function::MODIFY){

        if(m_root_mngr) {
          if(!update_schema_rs(schema, true))
            update_status(
              Protocol::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY, 
              schema, err);

        } else if(!update_schema(schema)) {
          column_update(
            Protocol::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY, 
            schema, err);
        }
      }
      return;
    }
    
    column_update(func, schema, err);
  }

  // RangeServer Actions
  void update_status(RsStatusList new_rs_status, bool sync_all){
    if(m_root_mngr && !sync_all)
      return;

    RsStatusList changed;

    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      RsStatusPtr h;
      bool found;
      bool chg;

      for(auto& rs_new : new_rs_status){
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
              Env::MngrColumns::get()->change_rs(h->rs_id, rs_new->rs_id);

            h->rs_id = rs_new->rs_id;
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
          if(rs_new->state == RsStatus::State::ACK) {
            if(rs_new->state != h->state) {
              h->state = RsStatus::State::ACK;
              chg = true;
            }
          } else {
            Env::MngrColumns::get()->set_rs_unassigned(h->rs_id);
            m_rs_status.erase(it);
            chg = true;
          }

          if(chg && !sync_all)
            changed.push_back(rs_new);
          found = true;
          break;
        }

        if(!found){
          if(rs_new->state == RsStatus::State::ACK){
            rs_new->init_queue();
            m_rs_status.push_back(rs_new);
            if(!sync_all)
              changed.push_back(rs_new);
          }
        }
      }
    }
    if(!changed.empty())
      std::cout << " update_status: ";

    rs_changes(sync_all ? m_rs_status : changed, sync_all && !m_root_mngr);
  }

  uint64_t rs_set_id(const EndPoints& endpoints, uint64_t opt_id=0){
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);
    return rs_set(endpoints, opt_id)->rs_id;
  }

  bool rs_ack_id(uint64_t rs_id, const EndPoints& endpoints){
    bool ack = false;
    RsStatusPtr new_ack = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
    
      for(auto& h : m_rs_status){
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

  uint64_t rs_had_id(uint64_t rs_id, const EndPoints& endpoints){
    bool new_id_required = false;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);

      for(auto& h : m_rs_status){
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

  void rs_shutdown(uint64_t rs_id, const EndPoints& endpoints){
    RsStatusPtr removed = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
        auto h = *it;
        if(has_endpoint(h->endpoints, endpoints)){
          removed = h;
          m_rs_status.erase(it);
          removed->state = RsStatus::State::REMOVED;
          Env::MngrColumns::get()->set_rs_unassigned(removed->rs_id);
          break;
        }
      }
    }
    if(removed != nullptr)
      rs_changes({removed});   
  }

  std::string to_string(){
    std::string s(Env::MngrColumns::get()->to_string());

    s.append("\nRangeServers:");
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);
    for(auto& h : m_rs_status) {
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
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto& h : m_rs_status)
        asio::post(*Env::IoCtx::io()->ptr(), [h](){ h->stop(); });
    }
  }


  void assign_range(RsStatusPtr rs, RangePtr range){
    rs->put(std::make_shared<Protocol::Req::RsRangeLoad>(rs, range));
  }

  void range_loaded(RsStatusPtr rs, RangePtr range, 
                    int err, bool failure=false) {
    bool run_assign = m_assignments-- > cfg_assign_due->get();           

    if(!range->deleted()) {
      if(err != Error::OK){
        {
          std::lock_guard<std::mutex> lock(m_mutex_rs_status);
          rs->total_ranges--;
          if(failure)
            rs->failures++;
        }
        range->set_state(Range::State::NOTSET, 0); 
        if(!run_assign) 
          check_assignment_timer(2000);

      } else {
        {
          std::lock_guard<std::mutex> lock(m_mutex_rs_status);
          rs->failures=0;
        }
        range->set_state(Range::State::ASSIGNED, rs->rs_id); 
        range->clear_last_rs();
        // adjust rs->resource
        // ++ mng_inchain - req. MngrRsResource
      
        ColumnFunction pending;
        while(column_load_pending(range->cid, pending))
          column_update(pending.func, Env::Schemas::get()->get(pending.cid));
      }

      HT_INFOF("RANGE-STATUS %d(%s), %s", 
                err, Error::get_text(err), range->to_string().c_str());
    }

    if(run_assign)
      assign_ranges();
  }

  void column_delete(int &err, int64_t cid, int64_t rs_id) {
    ColumnPtr col = Env::MngrColumns::get()->get_column(err, cid, false);
    if(col == nullptr || col->finalize_remove(err, rs_id)) {
      Env::MngrColumns::get()->remove(err, cid);
      DB::SchemaPtr schema = Env::Schemas::get()->get(cid);
      if(schema != nullptr)
        column_update(
          Protocol::Params::ColumnMng::Function::INTERNAL_ACK_DELETE,
          schema, err);
      if(!m_root_mngr)
        Env::Schemas::get()->remove(cid);
    }
  }

  void update_schema(int &err, RsStatusPtr rs, 
                     DB::SchemaPtr schema, bool failure) {
                       
    if(err == Error::OK)
      Env::MngrColumns::get()->get_column(err, schema->cid, false)
                             ->change_rs_schema(rs->rs_id, schema->revision);
    else if(failure) {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      rs->failures++;
      return;
    }

    if(!update_schema_rs(schema, false))
      column_update(
        Protocol::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY,
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
      std::lock_guard<std::mutex> lock1(m_mutex);
      std::lock_guard<std::mutex> lock2(m_mutex_columns);
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
          Env::RangeServers::get()->check_assignment();
        }
    }); 

    if(t_ms > 10000) {
      std::cout << to_string() << "\n";
    }
    HT_DEBUGF("RS ranges check_assignment scheduled in ms=%d", t_ms);
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
    {
      std::lock_guard<std::mutex> lock1(m_mutex);
      std::lock_guard<std::mutex> lock2(m_mutex_columns);
      
      if(m_columns_set){
        if(m_root_mngr)
          columns_load_chk_ack();
        return true;
      }
    }

    m_root_mngr = manage(1);
    DB::SchemaPtr schema;
    {
      std::lock_guard<std::mutex> lock1(m_mutex);
      std::lock_guard<std::mutex> lock2(m_mutex_columns);
    
      std::vector<int64_t> cols;
      Env::MngrRole::get()->get_active_columns(cols);
      if(cols.size() == 0){
        m_columns_set = false;
        return false; 
      }

      if(!m_root_mngr)
        return true;
      if(m_columns_set)
        return true;


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
      for(auto cid : entries) {
        err = Error::OK;
        schema = Files::Schema::load(err, cid);
        if(err == Error::OK)
          Env::Schemas::get()->add(err, schema);
        if(err !=  Error::OK)
          HT_WARNF("Schema cid=%d err=%d(%s)", cid, err, Error::get_text(err));
      }

      m_columns_set = true;
    }

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
      [](){ Env::RangeServers::get()->assign_ranges_run(); }
    );
  }

  void assign_ranges_run() {
    int err;
    RangePtr range;

    for(;;){
      std::lock_guard<std::mutex> lock(m_mutex_assign);
      {
        std::lock_guard<std::mutex> lock(m_mutex_rs_status);
        if(m_rs_status.empty() || !m_run){
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
      Files::RsDataPtr last_rs = range->get_last_rs(err);
      RsStatusPtr rs = nullptr;
      next_rs(last_rs, rs);
      if(rs == nullptr){
        m_runs_assign = false;
        check_assignment_timer();
        return;
      }

      range->set_state(Range::State::QUEUED, rs->rs_id);
      assign_range(rs, range, last_rs);
      if(++m_assignments > cfg_assign_due->get()){
        m_runs_assign = false;
        return;
      }
    }

    // balance/check-assigments if not runs :for rangeserver cid-rid state
    check_assignment_timer(cfg_chk_assign->get());
  }

  void next_rs(Files::RsDataPtr &last_rs, RsStatusPtr &rs_set){
    std::lock_guard<std::mutex> lock(m_mutex_rs_status);

    if(last_rs->endpoints.size() > 0) {
       for(auto& rs : m_rs_status) {
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

        if(rs->failures >= cfg_rs_failures->get()){
          m_rs_status.erase(it);
          Env::MngrColumns::get()->set_rs_unassigned(rs->rs_id);
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

    RsStatusPtr rs_last = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto& rs_chk : m_rs_status) {
        if(has_endpoint(rs_chk->endpoints, last_rs->endpoints)){
          rs_last = rs_chk;
          break;
        }
      }
    }
    if(rs_last == nullptr){
      rs_last = std::make_shared<RsStatus>(0, last_rs->endpoints);
      rs_last->init_queue();
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      std::cout <<  " assign_range, rs_last " << rs_last->to_string() << "\n";
      m_rs_status.push_back(rs_last);
    }

    rs_last->put(std::make_shared<Protocol::Req::RsIdReqNeeded>(rs, range));
  }

  RsStatusPtr rs_set(const EndPoints& endpoints, uint64_t opt_id=0){

    for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)) {
        if(h->state == RsStatus::State::ACK) {
          h->set(endpoints);
          return h;
        } else {
          Env::MngrColumns::get()->set_rs_unassigned(h->rs_id);
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
      for(auto& h : m_rs_status){
        if(nxt == h->rs_id) {
          ok = false;
          break;
        };
      }
    } while(!ok);

    RsStatusPtr h = std::make_shared<RsStatus>(nxt, endpoints);
    h->init_queue();
    m_rs_status.push_back(h);
    return h;
  }
  
  void rs_changes(RsStatusList hosts, bool sync_all=false){
    {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      if(hosts.size() > 0){
        Env::MngrRole::get()->req_mngr_inchain(
          std::make_shared<Protocol::Req::MngrUpdateRangeServers>(
          hosts, sync_all));

        std::cout << " rs_changes: \n";
        for(auto& h : hosts)
          std::cout << " " << h->to_string() << "\n";
      }
    }
    
    if(Env::MngrRole::get()->has_active_columns())
      check_assignment_timer(cfg_delay_rs_chg->get());
  }
  
  void columns_load(){
    std::vector<DB::SchemaPtr> entries;
    Env::Schemas::get()->all(entries);
    for(auto& schema : entries) {
      HT_ASSERT(schema->cid != 0);
      update_status(Protocol::Params::ColumnMng::Function::INTERNAL_LOAD, schema,
                    Error::OK, true);
    }
  }

  void columns_load_chk_ack(){
    for(auto& ack : m_cid_pending_load){
      if(ack.func == Protocol::Params::ColumnMng::Function::INTERNAL_ACK_LOAD){
        column_update(
          Protocol::Params::ColumnMng::Function::INTERNAL_LOAD,  
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
    HT_ASSERT(schema_save->cid != 0);
    
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

    DB::SchemaPtr schema_save = DB::Schema::make(
      old->cid,
      schema, 
      schema->revision != 0 ? 
      schema->revision : Time::now_ns());

    HT_ASSERT(schema_save->cid != 0);

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
    
    std::vector<uint64_t> rs_ids;
    col->assigned(rs_ids);
    if(rs_ids.empty()){
      column_delete(err, cid, 0);
      return;
    }

    for(auto rs_id : rs_ids){
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);
      for(auto& rs : m_rs_status){
        if(rs_id != rs->rs_id)
          continue;
        rs->total_ranges--;
        rs->put(std::make_shared<Protocol::Req::RsColumnDelete>(rs, cid));
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
        err = m_columns_set ? Error::OK : Error::MNGR_NOT_INITIALIZED;
      }
      if(!m_run)
        err = Error::SERVER_SHUTTING_DOWN;
      else if(req.params.schema->col_name.length() == 0)
        err = Error::COLUMN_SCHEMA_NAME_EMPTY;

      if(err == Error::OK){
        std::lock_guard<std::mutex> lock(m_mutex);
        DB::SchemaPtr schema = Env::Schemas::get()->get(req.params.schema->col_name);

        switch(req.params.function){
          case Protocol::Params::ColumnMng::Function::CREATE: {
            if(schema != nullptr)
              err = Error::COLUMN_SCHEMA_NAME_EXISTS;
            else
              column_create(err, req.params.schema);  
            break;
          }
          case Protocol::Params::ColumnMng::Function::MODIFY: {
            if(schema == nullptr) 
              err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
            else
              column_update_schema(err, req.params.schema, schema);
            break;
          }
          case Protocol::Params::ColumnMng::Function::DELETE: {
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
        HT_ASSERT(req.params.schema->cid != 0);
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
      return update_schema_rs(schema, true);
    }
    return false;
  }

  bool update_schema_rs(DB::SchemaPtr schema, bool ack_required) {
    std::vector<uint64_t> rs_ids;
    int err = Error::OK;
    Env::MngrColumns::get()->get_column(err, schema->cid, false)
                           ->need_schema_sync(schema->revision, rs_ids);
    bool undergo = false;
    for(auto& rs_id : rs_ids) {
      std::lock_guard<std::mutex> lock(m_mutex_rs_status);

      for(auto& rs : m_rs_status) {
        if(rs->failures < cfg_rs_failures->get() 
          && rs->state == RsStatus::State::ACK && rs->rs_id == rs_id) {
          undergo = true;
          if(ack_required){
            rs->put(
              std::make_shared<Protocol::Req::RsUpdateSchema>(rs, schema));
          }
        }
      }
    }
    return undergo;
  }

  void column_update(Protocol::Params::ColumnMng::Function func,
                     DB::SchemaPtr schema, int err=Error::OK){
    Env::MngrRole::get()->req_mngr_inchain(
      std::make_shared<Protocol::Req::MngrColumnUpdate>(func, schema, err));
  }

  void update_status_ack(Protocol::Params::ColumnMng::Function func,
                         DB::SchemaPtr schema, int err){
    if(err == Error::OK){
      switch(func){
        case Protocol::Params::ColumnMng::Function::INTERNAL_LOAD_ALL: {
          columns_load();
          return;
        }
        case Protocol::Params::ColumnMng::Function::INTERNAL_ACK_LOAD: {
          ColumnFunction pending;
          while(column_load_pending(schema->cid, pending));
          return;
        }
        case Protocol::Params::ColumnMng::Function::INTERNAL_ACK_CREATE: {
          break; 
        }
        case Protocol::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY: {
          break; 
        }
        case Protocol::Params::ColumnMng::Function::INTERNAL_ACK_DELETE: {
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

    auto co_func = (Protocol::Params::ColumnMng::Function)(((uint8_t)func)-1);

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
  bool                          m_columns_set = false;
  std::queue<ColumnActionReq>   m_column_actions;
  std::vector<ColumnActionReq>  m_cid_pending;
  std::vector<ColumnFunction>   m_cid_pending_load;

  std::mutex                    m_mutex_rs_status;
  RsStatusList                  m_rs_status;

  std::mutex                    m_mutex_assign;
  bool                          m_runs_assign = false;
  std::atomic<int>              m_assignments = 0; 

  std::atomic<bool>             m_root_mngr = false;
  std::atomic<bool>             m_run = true; 
  

  const gInt32tPtr cfg_rs_failures;
  const gInt32tPtr cfg_delay_rs_chg;
  const gInt32tPtr cfg_delay_cols_init;
  const gInt32tPtr cfg_chk_assign;
  const gInt32tPtr cfg_assign_due;
  
};

}} // namespace server



void Protocol::Req::RsRangeLoad::loaded(int err, bool failure) {
  if(schema != nullptr && err == Error::OK)
    Env::MngrColumns::get()->get_column(err, range->cid, false)
                           ->change_rs_schema(rs->rs_id, schema->revision);
                           
  else if(err == Error::COLUMN_SCHEMA_MISSING)
    Env::MngrColumns::get()->get_column(err, range->cid, false)
                           ->remove_rs_schema(rs->rs_id);

  Env::RangeServers::get()->range_loaded(rs, range, err, failure);
}

void Protocol::Req::RsUpdateSchema::updated(int err, bool failure) {
  Env::RangeServers::get()->update_schema(err, rs, schema, failure);
  
 if(failure)
    request_again();
}

void Protocol::Req::RsIdReqNeeded::rsp(int err) {
  if(err == Error::OK) 
  // RsId assignment on the way, put range back as not assigned 
    Env::RangeServers::get()->range_loaded(
      rs, range, Error::RS_NOT_LOADED_RANGE);
  else
    Env::RangeServers::get()->assign_range(rs, range);
}

void Protocol::Req::RsColumnDelete::remove(int err) {
  Env::RangeServers::get()->column_delete(err, cid, rs->rs_id);  
}


}

#endif // swc_lib_manager_RangeServers_h