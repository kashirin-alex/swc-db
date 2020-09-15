
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/manager/MngdColumns.h"
#include "swcdb/manager/Protocol/Mngr/req/ColumnUpdate.h"


namespace SWC { namespace Manager {



MngdColumns::MngdColumns()
    : m_run(true), m_schemas_set(false), m_cid_active(false), 
      m_cid_begin(DB::Schema::NO_CID), m_cid_end(DB::Schema::NO_CID),
      cfg_schema_replication(Env::Config::settings()->get<Property::V_GUINT8>(
        "swc.mngr.schema.replication")),
      cfg_delay_cols_init(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.ranges.assign.delay.afterColumnsInit")) { 
}

MngdColumns::~MngdColumns() {}

void MngdColumns::stop() {
  m_run = false;
}

void MngdColumns::reset(bool schemas_mngr) {
  if(schemas_mngr) {
    std::scoped_lock lock(m_mutex);
    m_schemas_set = false;
    if(!m_cid_active)
      Env::Mngr::schemas()->reset();
  }
}


bool MngdColumns::is_schemas_mngr(int& err) {
  if(Env::Mngr::role()->is_active_role(Types::MngrRole::SCHEMAS)) {
    if(!m_schemas_set)
      err = Error::MNGR_NOT_INITIALIZED; 
    return true;
  }
  return false;
}

bool MngdColumns::has_active() {
  return m_cid_active;
}

bool MngdColumns::has_cid_pending_load() {
  std::lock_guard lock(m_mutex_columns);
  return !m_cid_pending_load.empty();
}

bool MngdColumns::is_active(cid_t cid) {
  std::shared_lock lock(m_mutex);
  return m_cid_active && cid &&
        (!m_cid_begin || m_cid_begin <= cid) &&
        (!m_cid_end   || m_cid_end >= cid);
}

bool MngdColumns::active(cid_t& cid_begin, cid_t& cid_end) {
  std::shared_lock lock(m_mutex);
  if(!m_cid_active)
    return false;
  cid_begin = m_cid_begin;
  cid_end = m_cid_end;
  return true;
}

Column::Ptr MngdColumns::get_column(int& err, cid_t cid) {
  Column::Ptr col;
  if(!is_active(cid)) {
    err = Error::MNGR_NOT_ACTIVE;
    return col;
  }
  if(is_schemas_mngr(err) && err)
    return col;
    
  col = Env::Mngr::columns()->get_column(err, cid);
  if(!err) {
    if(col)
      col->state(err);
    else
      err = Error::COLUMN_NOT_EXISTS;
  }
  return col;
}

void MngdColumns::change_active(const cid_t cid_begin, const cid_t cid_end, 
                                bool has_cols) {
  if(!has_cols) {
    std::scoped_lock lock(m_mutex);
    if(m_cid_active) {
      m_cid_active = false;
      Env::Mngr::columns()->reset();
      if(!Env::Mngr::role()->is_active_role(Types::MngrRole::SCHEMAS))
        Env::Mngr::schemas()->reset();
    }
    return;
  }

  {
    std::scoped_lock lock(m_mutex);
    if(m_cid_active && cid_begin == m_cid_begin && cid_end == m_cid_end)
      return;
    m_cid_begin = cid_begin;
    m_cid_end = cid_end;
    m_cid_active = true;
  }

  if(m_run)
    Env::Mngr::rangers()->schedule_check(cfg_delay_cols_init->get());

  //if(Env::Mngr::role()->is_active_role(Types::MngrRole::SCHEMAS)) 
  //  (scheduled on column changes ) + chk(cid) LOAD_ACK
}


void MngdColumns::require_sync() {
  Env::Mngr::rangers()->sync();
    
  if(Env::Mngr::role()->is_active_role(Types::MngrRole::SCHEMAS)) {
    columns_load();
  } else {
    auto schema = DB::Schema::make();
    update(
      Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD_ALL, 
      schema
    );
  }
}

void MngdColumns::action(const ColumnReq::Ptr& new_req) {
  if(m_actions.push_and_is_1st(new_req))
    Env::IoCtx::post([this]() { actions_run(); });
}

void MngdColumns::update_status(
                    Protocol::Mngr::Params::ColumnMng::Function func, 
                    DB::Schema::Ptr& schema, int err, bool initial) {
  bool schemas_mngr = Env::Mngr::role()->is_active_role(
    Types::MngrRole::SCHEMAS);

  if(!initial && schemas_mngr)
    return update_status_ack(func, schema, err);

  err = Error::OK;
  if(!is_active(schema->cid))
    return update(func, schema, err);

  switch(func) {

    case Protocol::Mngr::Params::ColumnMng::Function::DELETE: {
      remove(err, schema->cid);
      break;  
    } 

    case Protocol::Mngr::Params::ColumnMng::Function::CREATE:
    case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD: {

      auto co_func = (
        Protocol::Mngr::Params::ColumnMng::Function)(((uint8_t)func)+1);

      if(Env::Mngr::columns()->is_an_initialization(err, schema) && !err) {
        {
          std::lock_guard lock(m_mutex_columns);
          m_cid_pending_load.emplace_back(co_func, schema->cid);
        }
        if(!schemas_mngr)
          Env::Mngr::schemas()->replace(schema);
        Env::Mngr::rangers()->assign_ranges();

      } else if(schemas_mngr) {
        update_status(co_func, schema, err);

      } else {
        update(co_func, schema, err);
      }
      break;
    }

    case Protocol::Mngr::Params::ColumnMng::Function::MODIFY: {
      if(schemas_mngr) {
        if(!Env::Mngr::rangers()->update(schema, true))
          update_status(
            Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY, 
            schema, err);

      } else if(!update(schema)) {
        update(
          Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY, 
          schema, err);
      }
    }
    
    default:
      break;
  }
}

void MngdColumns::load_pending(cid_t cid) {
  ColumnFunction pending;
  while(load_pending(cid, pending))
    update(pending.func, Env::Mngr::schemas()->get(pending.cid));
}

void MngdColumns::remove(int &err, cid_t cid, rgrid_t rgrid) {
  Column::Ptr col = Env::Mngr::columns()->get_column(err, cid);
  if(!col || col->finalize_remove(err, rgrid)) {
    Env::Mngr::columns()->remove(err, cid);
    DB::Schema::Ptr schema = Env::Mngr::schemas()->get(cid);
    if(schema)
      update(
        Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_DELETE,
        schema, err);
    if(!Env::Mngr::role()->is_active_role(Types::MngrRole::SCHEMAS))
      Env::Mngr::schemas()->remove(cid);
  }
}


void MngdColumns::print(std::ostream& out) {
  Env::Mngr::columns()->print(out);
}

bool MngdColumns::initialize() {
  if(m_schemas_set)
    return true;

  {
    std::scoped_lock lock1(m_mutex);
    if(m_schemas_set)
      return false;
    std::scoped_lock lock2(m_mutex_columns);
    
    int err = Error::OK;
    FS::IdEntries_t entries;
    Columns::columns_by_fs(err, entries); 
    if(err) {
      if(err != ENOENT)
        return false;
      err = Error::OK;
    }
    if(entries.empty()) { // initialize sys-columns
      for(cid_t cid=1; cid <= Files::Schema::SYS_CID_END; ++cid) {
        Column::create(err, cid);
        entries.push_back(cid);
      }
    }
    
    int32_t hdlrs = Env::IoCtx::io()->get_size()/4+1;
    int32_t vol = entries.size()/hdlrs+1;
    std::atomic<int64_t> pending = 0;
    while(!entries.empty()) {
      FS::IdEntries_t hdlr_entries;
      for(auto n=0; n < vol && !entries.empty(); ++n) {
        hdlr_entries.push_back(entries.front());
        entries.erase(entries.begin());
      }
      if(hdlr_entries.empty())
        break;

      ++pending;
      Env::IoCtx::post(
        [&pending, entries=hdlr_entries, 
         replicas=cfg_schema_replication->get()]() { 
          DB::Schema::Ptr schema;
          int err;
          for(cid_t cid : entries) {
            schema = Files::Schema::load(err = Error::OK, cid, replicas);
            if(!err)
              Env::Mngr::schemas()->add(err, schema);
            else
              SWC_LOGF(LOG_ERROR, "Schema cid=%lu err=%d(%s)", 
                       cid, err, Error::get_text(err));
          }
          --pending;
        }
      );
    }

    while(pending) // keep_locking
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    m_schemas_set = true;
  }
  
  columns_load();
  return true;
}


void MngdColumns::columns_load() {
  std::vector<DB::Schema::Ptr> entries;
  Env::Mngr::schemas()->all(entries);
  for(auto& schema : entries) {
    SWC_ASSERT(schema->cid != DB::Schema::NO_CID);
    update_status(Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD, schema,
                  Error::OK, true);
  }
}

void MngdColumns::columns_load_chk_ack() {
  std::lock_guard lock(m_mutex_columns);
  for(auto& ack : m_cid_pending_load) {
    if(ack.func == Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_LOAD) {
      update(
        Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD,  
        Env::Mngr::schemas()->get(ack.cid)
      );
    }
  }
}

bool MngdColumns::load_pending(cid_t cid, ColumnFunction &pending) {
  std::lock_guard lock(m_mutex_columns);

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

cid_t MngdColumns::get_next_cid() {
  cid_t cid = Files::Schema::SYS_CID_END;
  while(++cid && Env::Mngr::schemas()->get(cid));
  // if schema does exist on fs (? sanity-check) 
  return cid; // err !cid
}

void MngdColumns::create(int &err, DB::Schema::Ptr& schema) {
  cid_t cid = get_next_cid();
  if(!cid) {
    err = Error::COLUMN_REACHED_ID_LIMIT;
    return;
  } 
  if(schema->col_seq == Types::KeySeq::UNKNOWN || 
     schema->col_type == Types::Column::UNKNOWN) {
    err = Error::INVALID_ARGUMENT;
    return;
  }
  
  Column::create(err, cid);
  if(err)
    return;

  if(Types::is_counter(schema->col_type))
    schema->cell_versions = 1;

  auto schema_save = DB::Schema::make(schema);
  schema_save->cid = cid;
  if(!schema_save->revision)
    schema_save->revision = Time::now_ns();
    
  Files::Schema::save_with_validation(
    err, schema_save, cfg_schema_replication->get());
  if(!err) 
    Env::Mngr::schemas()->add(err, schema_save);

  if(!err) 
    schema = Env::Mngr::schemas()->get(schema_save->cid);
  else 
    Column::remove(err, cid);
}
  
void MngdColumns::update(int &err, DB::Schema::Ptr& schema, 
                         const DB::Schema::Ptr& old) {
  if(old->col_seq != schema->col_seq || 
     Types::is_counter(old->col_type) != Types::is_counter(schema->col_type) ||
     (schema->cid <= Files::Schema::SYS_CID_END && 
      schema->col_name.compare(old->col_name) != 0)) {
    err = Error::COLUMN_CHANGE_INCOMPATIBLE;
    return;
  }

  if(Types::is_counter(schema->col_type))
    schema->cell_versions = 1;

  if(schema->cid < Files::Schema::SYS_CID_END) { 
    //different values bad for range-colms
    schema->cell_versions = 1;
    schema->cell_ttl = 0;
  }

  auto schema_save = DB::Schema::make(schema);
  if(schema_save->cid == DB::Schema::NO_CID)
    schema_save->cid = old->cid;
  if(!schema_save->revision)
    schema_save->revision = Time::now_ns();

  if(schema_save->cid == DB::Schema::NO_CID)
    err = Error::COLUMN_SCHEMA_ID_EMPTY;
  if(schema_save->equal(old, false))
    err = Error::COLUMN_SCHEMA_NOT_DIFFERENT;
  if(err)
    return;

  Files::Schema::save_with_validation(
    err, schema_save, cfg_schema_replication->get());
  if(!err) {
    Env::Mngr::schemas()->replace(schema_save);
    schema = Env::Mngr::schemas()->get(schema_save->cid);
  }
}

void MngdColumns::remove(int &err, cid_t cid) {
  Column::Ptr col = Env::Mngr::columns()->get_column(err, cid);
  if(!col)
    return remove(err, cid, 0);
  if(!col->do_remove())
    return;
  SWC_LOGF(LOG_DEBUG, "DELETING cid=%lu", cid);
    
  std::vector<rgrid_t> rgrids;
  col->assigned(rgrids);
  if(rgrids.empty())
    return remove(err, cid, 0);

  Env::Mngr::rangers()->column_delete(cid, rgrids);
}

bool MngdColumns::update(DB::Schema::Ptr& schema) {
  DB::Schema::Ptr existing = Env::Mngr::schemas()->get(schema->cid);
  if(!existing || !existing->equal(schema)) {
    Env::Mngr::schemas()->replace(schema);
    return Env::Mngr::rangers()->update(schema, true);
  }
  return false;
}


void MngdColumns::update(Protocol::Mngr::Params::ColumnMng::Function func,
                         const DB::Schema::Ptr& schema, int err) {
  Env::Mngr::role()->req_mngr_inchain(
    std::make_shared<Protocol::Mngr::Req::ColumnUpdate>(func, schema, err));
}

void MngdColumns::update_status_ack(
                          Protocol::Mngr::Params::ColumnMng::Function func,
                          const DB::Schema::Ptr& schema, int err) {
  if(!err) switch(func) {
    case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_LOAD_ALL: {
      return columns_load();
    }
    case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_LOAD: {
      ColumnFunction pending;
      while(load_pending(schema->cid, pending));
      return;
    }
    case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_CREATE:
    case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_MODIFY:
      break;
    case Protocol::Mngr::Params::ColumnMng::Function::INTERNAL_ACK_DELETE: {
      std::scoped_lock lock(m_mutex);

      if(!Env::Mngr::schemas()->get(schema->cid)) {
        err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
      } else if(!m_run) {
        err = Error::SERVER_SHUTTING_DOWN;
      } else {
        Column::remove(err, schema->cid);
        if(!err)
          Env::Mngr::schemas()->remove(schema->cid);
      }
      break;
    }
    default:
      return;
  }

  auto co_func = (Protocol::Mngr::Params::ColumnMng::Function)(((uint8_t)func)-1);

  if(err)
    SWC_LOG_OUT(LOG_DEBUG,
      SWC_LOG_OSTREAM << "COLUMN-ACK func=" << co_func;
      Error::print(SWC_LOG_OSTREAM << ' ', err);
      schema->print(SWC_LOG_OSTREAM << ' ');
    );
                  
  for(ColumnReq::Ptr req;;) {
    {
      std::lock_guard lock(m_mutex_columns);
      auto it = std::find_if(m_cid_pending.begin(), m_cid_pending.end(),  
          [co_func, cid=schema->cid](const ColumnReq::Ptr& req)
          {return req->schema->cid == cid && req->function == co_func;});
      if(it == m_cid_pending.end())
        break;  
      req = *it;
      m_cid_pending.erase(it);
    }

    try {
      err ? req->response(err) : req->response_ok();
    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, 
        SWC_LOG_OSTREAM << "Column Pending cb func=" << func;
        Error::print(SWC_LOG_OSTREAM << ' ', err);
        req->schema->print(SWC_LOG_OSTREAM << ' ');
        SWC_LOG_OSTREAM << ' ' << e;
      );
    }
  }
}

void MngdColumns::actions_run() {  
  ColumnReq::Ptr req;
  int err;
  do {
    req = m_actions.front();

    if(!m_run) {
      err = Error::SERVER_SHUTTING_DOWN;

    } else if(!m_schemas_set) {
      err = Error::MNGR_NOT_INITIALIZED;

    } else if(req->schema->col_name.empty()) {
      err = Error::COLUMN_SCHEMA_NAME_EMPTY;

    } else {
      err = Error::OK;
      std::scoped_lock lock(m_mutex);
      DB::Schema::Ptr schema = Env::Mngr::schemas()->get(
        req->schema->col_name);
      if(!schema && req->schema->cid != DB::Schema::NO_CID)
        schema = Env::Mngr::schemas()->get(req->schema->cid);

      switch(req->function) {
        case Protocol::Mngr::Params::ColumnMng::Function::CREATE: {
          if(schema)
            err = Error::COLUMN_SCHEMA_NAME_EXISTS;
          else
            create(err, req->schema);  
          break;
        }
        case Protocol::Mngr::Params::ColumnMng::Function::MODIFY: {
          if(!schema) 
            err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
          else if(req->schema->cid != DB::Schema::NO_CID && 
                  schema->cid != req->schema->cid)
            err = Error::COLUMN_SCHEMA_NAME_EXISTS;
          else
            update(err, req->schema, schema);
          break;
        }
        case Protocol::Mngr::Params::ColumnMng::Function::DELETE: {
          if(!schema)
            err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
          else if(schema->cid != req->schema->cid ||
                  req->schema->col_name.compare(schema->col_name) != 0)
            err = Error::COLUMN_SCHEMA_NAME_NOT_CORRES;
          else if(schema->cid <= Files::Schema::SYS_CID_END)
            err = Error::COLUMN_SCHEMA_IS_SYSTEM;
          else
            req->schema = schema;
          break;
        }
        default:
          err = Error::NOT_IMPLEMENTED;
          break;
      }
    }

    if(!err) {
      {
        std::lock_guard lock(m_mutex_columns);
        m_cid_pending.push_back(req);
      }
      SWC_ASSERT(req->schema->cid != DB::Schema::NO_CID);
      update_status(req->function, req->schema, err, true);

    } else {
      try{
        req->response(err);
      } catch(...) {
        const Exception& e = SWC_CURRENT_EXCEPTION("");
        SWC_LOG_OUT(LOG_ERROR, 
          SWC_LOG_OSTREAM << "Column Action cb func=" << req->function;
          Error::print(SWC_LOG_OSTREAM << ' ', err);
          req->schema->print(SWC_LOG_OSTREAM << ' ');
          SWC_LOG_OSTREAM << ' ' << e;
        );
      }
    }
  } while(m_actions.pop_and_more());
}



}} // namespace Manager

#include "swcdb/manager/Protocol/Rgr/req/ColumnsUnload.cc"
#include "swcdb/manager/Protocol/Rgr/req/RangeUnload.cc"
