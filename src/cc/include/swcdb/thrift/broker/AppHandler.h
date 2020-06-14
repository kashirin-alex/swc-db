/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_app_thriftbroker_AppHandler_h
#define swc_app_thriftbroker_AppHandler_h

#include "swcdb/db/client/sql/SQL.h"

namespace SWC { 
namespace thrift = apache::thrift;
namespace Thrift {

void exception(int err, const std::string& msg = "") {
  Exception e;
  e.__set_code(err);
  e.__set_message(msg + " - " + Error::get_text(err));
  SWC_LOG_OUT(LOG_DEBUG);
  e.printTo(std::cout);
  std::cout << SWC_LOG_OUT_END;
  throw e;
}

void set(const SpecFlags& flags, DB::Specs::Flags& dbspec) {
  if(flags.__isset.limit)
    dbspec.limit = flags.limit;
  if(flags.__isset.offset)
    dbspec.offset = flags.offset;
  if(flags.__isset.max_versions)
    dbspec.max_versions = flags.max_versions;
  if(flags.__isset.max_buffer)
    dbspec.max_buffer = flags.max_buffer;
  if(flags.__isset.options)
    dbspec.options = flags.options;
}

void set(const Key& key, DB::Cell::Key& dbkey) {
  dbkey.read(key);
}

void set(const SpecKey& spec, DB::Specs::Key& dbspec) {
  for(auto& fraction : spec) {
    if((uint8_t)fraction.comp > 0x8)
      exception(
        Error::INCOMPATIBLE_OPTIONS, 
        "Key ext-fraction-comp(" + 
        Condition::to_string((Condition::Comp)fraction.comp, true) + 
        ")"
      );
    dbspec.add(fraction.f, (Condition::Comp)fraction.comp);
  }
}

void set(const SpecValue& spec, DB::Specs::Value& dbspec) {
  dbspec.set(spec.v, (Condition::Comp)spec.comp);
}

void set(const SpecTimestamp& spec, DB::Specs::Timestamp& dbspec) {
  dbspec.set(spec.ts, (Condition::Comp)spec.comp);
}

void set(const SpecInterval& intval, DB::Specs::Interval& dbintval) {
  if(intval.__isset.range_begin)
    set(intval.range_begin, dbintval.range_begin);

  if(intval.__isset.range_end)
    set(intval.range_end, dbintval.range_end);

  if(intval.__isset.range_offset)
    set(intval.range_offset, dbintval.range_offset);

  if(intval.__isset.offset_key)
    set(intval.offset_key, dbintval.offset_key);

  if(intval.__isset.offset_rev)
    dbintval.offset_rev = intval.offset_rev;

  if(intval.__isset.key_start)
    set(intval.key_start, dbintval.key_start);

  if(intval.__isset.key_finish)
    set(intval.key_finish, dbintval.key_finish);

  if(intval.__isset.value)
    set(intval.value, dbintval.value);

  if(intval.__isset.ts_start)
    set(intval.ts_start, dbintval.ts_start);

  if(intval.__isset.ts_finish)
    set(intval.ts_finish, dbintval.ts_finish);

  if(intval.__isset.flags)
    set(intval.flags, dbintval.flags);
}




class AppHandler final : virtual public BrokerIf {
  public:

  virtual ~AppHandler() { }


  /* SQL SCHEMAS/COLUMNS */
  void sql_list_columns(Schemas& _return, const std::string& sql) {

    int err = Error::OK;
    std::vector<DB::Schema::Ptr> dbschemas;  
    std::string message;
    client::SQL::parse_list_columns(err, sql, dbschemas, message, "list");
    if(err) 
      exception(err, message);

    if(dbschemas.empty()) { // get all schema
      std::promise<int> res;
      Protocol::Mngr::Req::ColumnList::request(
        [&dbschemas, await=&res]
        (client::ConnQueue::ReqBase::Ptr req, int error, 
         const Protocol::Mngr::Params::ColumnListRsp& rsp) {
          if(!error)
            dbschemas = rsp.schemas;
          await->set_value(error);
        },
        300000
      );
      if(err = res.get_future().get()) {
        message.append(Error::get_text(err));
        message.append("\n");
        exception(err, message);
      }
    }
    
    _return.resize(dbschemas.size());
    uint32_t c = 0;
    for(auto& dbschema : dbschemas) {
      auto& schema = _return[c++];
      schema.__set_cid(dbschema->cid);
      schema.__set_col_name(dbschema->col_name);
      schema.__set_col_type(
        (ColumnType::type)(uint8_t)dbschema->col_type);

      schema.__set_cell_versions(dbschema->cell_versions);
      schema.__set_cell_ttl(dbschema->cell_ttl);

      schema.__set_blk_encoding(
        (EncodingType::type)(uint8_t)dbschema->blk_encoding);
      schema.__set_blk_size(dbschema->blk_size);
      schema.__set_blk_cells(dbschema->blk_cells);

      schema.__set_cs_replication(dbschema->cs_replication);
      schema.__set_cs_size(dbschema->cs_size);
      schema.__set_cs_max(dbschema->cs_max);
      
      schema.__set_log_rollout_ratio(dbschema->log_rollout_ratio);
      
      schema.__set_compact_percent(dbschema->compact_percent);

      schema.__set_revision(dbschema->revision);
    }
  }

  void sql_mng_column(const std::string& sql) {
    int err = Error::OK;
    std::string message;
    DB::Schema::Ptr schema;
    Protocol::Mngr::Req::ColumnMng::Func func;
    client::SQL::parse_column_schema(
      err, sql, 
      &func, 
      schema, message);
    if(err)
      exception(err, message);

    std::promise<int> res;
    Protocol::Mngr::Req::ColumnMng::request(
      func,
      schema,
      [await=&res]
      (client::ConnQueue::ReqBase::Ptr req, int error) {
        await->set_value(error);
      },
      300000
    );
    
    if(err = res.get_future().get()) 
      exception(err, message);

    if(schema->cid != DB::Schema::NO_CID)
      Env::Clients::get()->schemas->remove(schema->cid);
    else
      Env::Clients::get()->schemas->remove(schema->col_name);
  }

  void sql_compact_columns(CompactResults& _return, const std::string& sql) {
    
    int err = Error::OK;
    std::vector<DB::Schema::Ptr> dbschemas;  
    std::string message;
    client::SQL::parse_list_columns(err, sql, dbschemas, message, "compact");
    if(err) 
      exception(err, message);

    if(dbschemas.empty()) { // get all schema
      std::promise<int> res;
      Protocol::Mngr::Req::ColumnList::request(
        [&dbschemas, await=&res]
        (client::ConnQueue::ReqBase::Ptr req, int error, 
         const Protocol::Mngr::Params::ColumnListRsp& rsp) {
          if(!error)
            dbschemas = rsp.schemas;
          await->set_value(error);
        },
        300000
      );
      if(err = res.get_future().get()) {
        message.append(Error::get_text(err));
        message.append("\n");
        exception(err, message);
      }
    }
    
    std::mutex mutex;
    std::promise<void> res;
    for(auto& schema : dbschemas) {
      Protocol::Mngr::Req::ColumnCompact::request(
        schema->cid,
        [&mutex, &_return, await=&res, cid=schema->cid, sz=dbschemas.size()]
        (client::ConnQueue::ReqBase::Ptr req, 
         const Protocol::Mngr::Params::ColumnCompactRsp& rsp) {
          std::lock_guard lock(mutex);
          auto& r = _return.emplace_back();
          r.cid=cid;
          r.err=rsp.err;
          if(_return.size() == sz)
            await->set_value();
        },
        300000
      );
    }
    res.get_future().wait();
  }
  
  /* SQL QUERY */
  client::Query::Select::Ptr sync_select(const std::string& sql) {
    auto req = std::make_shared<client::Query::Select>();
    int err = Error::OK;
    std::string message;
    uint8_t display_flags = 0;
    client::SQL::parse_select(err, sql, req->specs, display_flags, message);
    if(!err) {
      req->scan(err);
      if(!err)
        req->wait();
    }
    if(err) 
      exception(err, message);
    return req;
  }
  
  void sql_query(CellsGroup& _return, const std::string& sql, 
                 const CellsResult::type rslt) {
    switch(rslt) {
      case CellsResult::ON_COLUMN : {
        sql_select_rslt_on_column(_return.ccells, sql);
        _return.__isset.ccells = true;
        break;
      }
      case CellsResult::ON_KEY : {
        sql_select_rslt_on_key(_return.kcells, sql);
        _return.__isset.kcells = true;
        break;
      }
      case CellsResult::ON_FRACTION : {
        sql_select_rslt_on_fraction(_return.fcells, sql);
        _return.__isset.fcells = true;
        break;
      }
      default : {
        sql_select(_return.cells, sql);
        _return.__isset.cells = true;
        break;
      }
    }
  }

  void sql_select(Cells& _return, const std::string& sql) {
    auto req = sync_select(sql);
    
    int err = Error::OK;
    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err);
  }

  void sql_select_rslt_on_column(CCells& _return, const std::string& sql) {
    auto req = sync_select(sql);

    int err = Error::OK;
    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err);
  }

  void sql_select_rslt_on_key(KCells& _return, const std::string& sql) {
    auto req = sync_select(sql);
    
    int err = Error::OK;
    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err);
  }

  void sql_select_rslt_on_fraction(FCells& _return, const std::string& sql) {
    auto req = sync_select(sql);
    
    int err = Error::OK;
    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err);
  }
  
  /* SQL UPDATE */
  void sql_update(const std::string& sql, const int64_t updater_id) {
    
    client::Query::Update::Ptr req = nullptr;
    if(updater_id)
      updater(updater_id, req);
    else
      req = std::make_shared<client::Query::Update>();

    std::string message;
    uint8_t display_flags = 0;
    int err = Error::OK;
    client::SQL::parse_update(
      err, sql, 
      *req->columns.get(), *req->columns_onfractions.get(), 
      display_flags, message
    );
    if(err) 
      exception(err, message);
      
    if(updater_id) {
      req->commit_or_wait();
    } else {
      req->commit();
      req->wait();
    }
    if(err = req->result->error())
      exception(err);
  }


  /* SPECS SCAN QUERY */
  client::Query::Select::Ptr sync_select(const SpecScan& spec) {
    auto req = std::make_shared<client::Query::Select>();
    int err = Error::OK;
    
    if(spec.__isset.flags)
      set(spec.flags, req->specs.flags);

    DB::Schema::Ptr schema;
    DB::Specs::Interval::Ptr dbintval;

    for(auto& col : spec.columns) {
      schema = Env::Clients::get()->schemas->get(err, col.cid);
      if(!schema)
        exception(err, "cid=" + std::to_string(col.cid));

      req->specs.columns.push_back(DB::Specs::Column::make_ptr(col.cid));
      auto& dbcol = req->specs.columns.back();

      for(auto& intval : col.intervals) {
        dbintval = DB::Specs::Interval::make_ptr();
        set(intval, *dbintval.get());
        dbcol->intervals.push_back(dbintval);
      }
    }

    if(!err) {
      req->scan(err);
      if(!err)
        req->wait();
    }
    if(err) 
      exception(err);
    return req;
  }

  void scan_rslt_on(CellsGroup& _return, const SpecScan& specs, 
                    const CellsResult::type rslt) {
    switch(rslt) {
      case CellsResult::ON_COLUMN : {
        scan_rslt_on_column(_return.ccells, specs);
        _return.__isset.ccells = true;
        break;
      }
      case CellsResult::ON_KEY : {
        scan_rslt_on_key(_return.kcells, specs);
        _return.__isset.kcells = true;
        break;
      }
      case CellsResult::ON_FRACTION : {
        scan_rslt_on_fraction(_return.fcells, specs);
        _return.__isset.fcells = true;
        break;
      }
      default : {
        scan(_return.cells, specs);
        _return.__isset.cells = true;
        break;
      }
    }
  }

  void scan(Cells& _return, const SpecScan& specs) {
    auto req = sync_select(specs);
    
    int err = Error::OK;
    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err);
  }

  void scan_rslt_on_column(CCells& _return, const SpecScan& specs) {
    auto req = sync_select(specs);

    int err = Error::OK;
    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err);
  }


  void scan_rslt_on_key(KCells& _return, const SpecScan& specs) {
    auto req = sync_select(specs);
    
    int err = Error::OK;
    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err);
  }

  void scan_rslt_on_fraction(FCells& _return, const SpecScan& specs) {
    auto req = sync_select(specs);
    
    int err = Error::OK;
    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err);
  }

  /* UPDATER */
  int64_t updater_create(const int32_t buffer_size) {
    std::lock_guard lock(m_mutex);

    int64_t id = 1;
    for(auto it = m_updaters.begin();
        it != m_updaters.end();
        it = m_updaters.find(++id)
    );
    m_updaters[id] = std::make_shared<client::Query::Update>();
    if(buffer_size)
      m_updaters[id]->buff_sz = buffer_size;
    return id;
  }

  void updater_close(const int64_t id) {
    client::Query::Update:: Ptr req;
    {
      std::lock_guard lock(m_mutex);
    
      auto it = m_updaters.find(id);
      if(it == m_updaters.end())
        exception(ERANGE, "Updater ID not found");
      req = it->second;
      m_updaters.erase(it);
    }
    updater_close(req);
  }

  /* UPDATE */
  void update(const UCCells& cells, const int64_t updater_id) {
    client::Query::Update::Ptr req = nullptr;
    if(updater_id)
      updater(updater_id, req);
    else
      req = std::make_shared<client::Query::Update>();

    int err = Error::OK;
    size_t cells_bytes;
    DB::Cells::Cell dbcell;
    cid_t cid;
    // req->columns_onfractions
    for(auto& col_cells : cells) {
              // req->columns_onfractions
      auto col = req->columns->get_col(cid = col_cells.first);
      if(col == nullptr) {
        auto schema = Env::Clients::get()->schemas->get(err, cid);
        if(err) 
          exception(err);
        req->columns->create(schema);
        col = req->columns->get_col(cid);
      }
      for(auto& cell : col_cells.second) {
        dbcell.flag = (uint8_t)cell.f;
        dbcell.key.read(cell.k);
        dbcell.control = 0;
        if(cell.__isset.ts) 
          dbcell.set_timestamp(cell.ts);
        if(cell.__isset.ts_desc) 
          dbcell.set_time_order_desc(cell.ts_desc);
        dbcell.set_value(cell.v);

        col->add(dbcell);
        req->commit_or_wait(col);
      }
    }

    if(updater_id) {
      req->commit_or_wait();
    } else {
      req->commit();
      req->wait();
    }
    if(err = req->result->error())
      exception(err);
  }

  void disconnected() {
    updater_close();
  }

  private:

  void updater_close() {
    client::Query::Update:: Ptr req;
    for(;;) {
      {
        std::lock_guard lock(m_mutex);
        auto it = m_updaters.begin();
        if(it == m_updaters.end()) 
          break;
        m_updaters.erase(it);
        req = it->second;
      }
      try { updater_close(req); } catch(...) {}
    }
  }

  void updater(const int64_t id, client::Query::Update::Ptr& req) {
    std::lock_guard lock(m_mutex);

    auto it = m_updaters.find(id);
    if(it == m_updaters.end())
      exception(ERANGE, "Updater ID not found");
    req = it->second;
  }

  void updater_close(client::Query::Update:: Ptr req) {
    req->commit_if_need();
    req->wait();
    int err;
    if(err = req->result->error())
      exception(err);
  }

  static void process_results(
          int& err, client::Query::Select::Result::Ptr result,
          bool with_value, Cells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells;

    size_t c;
    for(cid_t cid : result->get_cids()) {
      cells.free();
      result->get_cells(cid, cells);

      schema = Env::Clients::get()->schemas->get(err, cid);
      c = _return.size();
      _return.resize(c+cells.size());

      for(auto dbcell : cells) {
        auto& cell = _return[c++];
        
        cell.c = schema->col_name;
        dbcell->key.convert_to(cell.k);
        cell.ts = dbcell->timestamp;
        if(cell.__isset.v = with_value) {
          if(dbcell->vlen)
            cell.v = std::string((const char*)dbcell->value, dbcell->vlen);
        }
      }
    }                            
  }
  
  static void process_results(
          int& err, client::Query::Select::Result::Ptr result,
          bool with_value, CCells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells; 

    for(cid_t cid : result->get_cids()) {
      cells.free();
      result->get_cells(cid, cells); 

      schema = Env::Clients::get()->schemas->get(err, cid);
      if(err)
        return;
      auto& list = _return[schema->col_name];     
      list.resize(cells.size());

      uint32_t c = 0;
      for(auto dbcell : cells) {
        auto& cell = list[c++];
        
        dbcell->key.convert_to(cell.k);
        cell.ts = dbcell->timestamp;
        if(cell.__isset.v = with_value) {
          if(dbcell->vlen)
            cell.v = std::string((const char*)dbcell->value, dbcell->vlen);
        }
      }
    }
  }

  static void process_results(
          int& err, client::Query::Select::Result::Ptr result,
          bool with_value, KCells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells;

    for(cid_t cid : result->get_cids()) {
      cells.free();
      result->get_cells(cid, cells); 

      schema = Env::Clients::get()->schemas->get(err, cid);
      if(err)
        return;

      for(auto dbcell : cells) {
        auto it = std::find_if(_return.begin(), _return.end(), 
                              [dbcell](const kCells& key_cells)
                              {return dbcell->key.equal(key_cells.k);});
        if(it == _return.end()) {
          _return.emplace_back();
          it = _return.end()-1;
          dbcell->key.convert_to(it->k);
        }
        
        auto& cell = it->cells.emplace_back();
        cell.c = schema->col_name;
        cell.ts = dbcell->timestamp;
        if(cell.__isset.v = with_value) {
          if(dbcell->vlen)
            cell.v = std::string((const char*)dbcell->value, dbcell->vlen);
        }
      }
    }
  }

  static void process_results(
          int& err, client::Query::Select::Result::Ptr result,
          bool with_value, FCells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells;
    
    std::vector<std::string> key;

    for(cid_t cid : result->get_cids()) {
      cells.free();
      result->get_cells(cid, cells); 

      schema = Env::Clients::get()->schemas->get(err, cid);
      if(err)
        return;
      
      FCells* fraction_cells;
      
      for(auto dbcell : cells) {
        fraction_cells = &_return;
        key.clear();
        dbcell->key.convert_to(key);
        for(auto& f : key) 
          fraction_cells = &fraction_cells->f[f];
        
        fraction_cells->__isset.cells = true;
        auto& cell = fraction_cells->cells.emplace_back();
        cell.c = schema->col_name;
        cell.ts = dbcell->timestamp;
        if(cell.__isset.v = with_value) {
          if(dbcell->vlen)
            cell.v = std::string((const char*)dbcell->value, dbcell->vlen);
        }
      }
    }
  }

  std::mutex m_mutex;
  std::unordered_map<
    int64_t, client::Query::Update::Ptr> m_updaters;
};




}}

#endif // swc_app_thriftbroker_AppHandler_h