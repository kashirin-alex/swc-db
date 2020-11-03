
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/manager/MngdColumns.h"
#include "swcdb/manager/Protocol/Mngr/req/ColumnUpdate.h"


namespace SWC { namespace Manager {

using ColumnMngFunc = Comm::Protocol::Mngr::Params::ColumnMng::Function;


MngdColumns::MngdColumns()
    : m_run(true), m_schemas_set(false), m_cid_active(false), 
      m_cid_begin(DB::Schema::NO_CID), m_cid_end(DB::Schema::NO_CID),
      m_expected_ready(false),
      cfg_schema_replication(
        Env::Config::settings()->get<Config::Property::V_GUINT8>(
          "swc.mngr.schema.replication")),
      cfg_delay_cols_init(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
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
  if(Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS)) {
    if(!m_schemas_set) {
      std::scoped_lock lock(m_mutex);
      if(!m_schemas_set)
        err = Error::MNGR_NOT_INITIALIZED;
    }
    return true;
  }
  return false;
}

bool MngdColumns::has_active() {
  return m_cid_active;
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

bool MngdColumns::expected_ready() {
  std::scoped_lock lock(m_mutex_columns);
  return m_expected_ready;
}

void MngdColumns::columns_ready(int& err) {
  {
    std::shared_lock lock(m_mutex);
    if(!m_cid_active)
      return;
  }
  if(!expected_ready()) {
    err = Error::MNGR_NOT_INITIALIZED;
    return;
  }
  Env::Mngr::columns()->state(err);
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
    if(col) {
      col->state(err);
    } else {
      err = expected_ready()
        ? Error::COLUMN_NOT_EXISTS
        : Error::MNGR_NOT_INITIALIZED;
    }
  }
  return col;
}

void MngdColumns::change_active(const cid_t cid_begin, const cid_t cid_end, 
                                bool has_cols) {
  if(!has_cols) {
    std::scoped_lock lock(m_mutex);
    if(m_cid_active) {
      m_cid_active = false;
      {
        std::scoped_lock lock(m_mutex_columns);
        m_expected_ready = false;
      }
      Env::Mngr::columns()->reset();
      if(!Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS))
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

  //if(Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS)) 
  //  (scheduled on column changes ) + chk(cid) LOAD_ACK
}


void MngdColumns::require_sync() {
  Env::Mngr::rangers()->sync();

  if(!columns_load()) {
    auto schema = DB::Schema::make();
    update(
      ColumnMngFunc::INTERNAL_LOAD_ALL, 
      schema
    );
  }
}

void MngdColumns::action(const ColumnReq::Ptr& req) {
  if(m_actions.activating(req))
    Env::Mngr::post([this, req]() { run(req); });
}

void MngdColumns::set_expect(const std::vector<cid_t>& columns, 
                             bool initial) {
  if(!initial && 
     Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS))
    return;

  if(!is_active(columns.front()) && !is_active(columns.back()))
    return Env::Mngr::role()->req_mngr_inchain(
      std::make_shared<Comm::Protocol::Mngr::Req::ColumnUpdate>(columns));

  SWC_LOGF(LOG_DEBUG, "Expected Columns to Load size=%lu", columns.size());
  std::scoped_lock lock(m_mutex_columns);
  m_expected_load = columns; // fill if in-batches
  m_expected_ready = false;
}

void MngdColumns::update_status(
                    ColumnMngFunc func, 
                    DB::Schema::Ptr& schema, int err, bool initial) {
  bool schemas_mngr = Env::Mngr::role()->is_active_role(
    DB::Types::MngrRole::SCHEMAS);

  if(!initial && schemas_mngr)
    return update_status_ack(func, schema, err);

  err = Error::OK;
  if(!is_active(schema->cid))
    return update(func, schema, err);

  bool do_update = false;
  switch(func) {

    case ColumnMngFunc::DELETE: {
      remove(err, schema->cid);
      do_update = err;
      break;  
    } 

    case ColumnMngFunc::CREATE:
    case ColumnMngFunc::INTERNAL_LOAD: {
      if(Env::Mngr::columns()->is_an_initialization(err, schema) && !err) {
        if(!schemas_mngr)
          Env::Mngr::schemas()->replace(schema);
        Env::Mngr::rangers()->assign_ranges();
      }
      if(!err) {
        std::scoped_lock lock(m_mutex_columns);
        if(!m_expected_ready) {
          auto it = std::find(
            m_expected_load.begin(), m_expected_load.end(), schema->cid);
          if(it !=  m_expected_load.end()) {
            m_expected_load.erase(it);
            m_expected_ready = m_expected_load.empty();
          }
        }
      }
      do_update = true;
      break;
    }

    case ColumnMngFunc::MODIFY: {
      do_update = true;
      if(!schemas_mngr) {
        auto existing = Env::Mngr::schemas()->get(schema->cid);
        do_update = !existing || !existing->equal(schema);
        if(do_update)
          Env::Mngr::schemas()->replace(schema);
      }
      do_update = do_update && !Env::Mngr::rangers()->update(schema, true);
    }
    
    default:
      break;
  }

  if(do_update) {
    auto co_func = (ColumnMngFunc)(((uint8_t)func)+1);
    schemas_mngr
      ? update_status_ack(co_func, schema, err)
      : update(co_func, schema, err);
  }
}

void MngdColumns::remove(int &err, cid_t cid, rgrid_t rgrid) {
  Column::Ptr col = Env::Mngr::columns()->get_column(err, cid);
  if(!col || col->finalize_remove(err, rgrid)) {
    Env::Mngr::columns()->remove(err, cid);
    DB::Schema::Ptr schema = Env::Mngr::schemas()->get(cid);
    if(schema)
      update(
        ColumnMngFunc::INTERNAL_ACK_DELETE,
        schema, err);
    if(!Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS))
      Env::Mngr::schemas()->remove(cid);
  }
  err = Error::OK;
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
    // initialize / recover sys-columns
    for(cid_t cid=1; cid <= Common::Files::Schema::SYS_CID_END; ++cid) {
      if(std::find(entries.begin(), entries.end(), cid) == entries.end()) {
        Column::create(err, cid);
        entries.push_back(cid);
      }
    }

    std::atomic<int64_t> pending = 1;
    int32_t vol = entries.size()/(Env::Mngr::io()->get_size()/4 + 1) + 1;
    auto it = entries.begin();
    FS::IdEntries_t::iterator it_to;
    do {
      ++pending;
      it_to = it + vol < entries.end() ? (it + vol) : entries.end();
      Env::Mngr::post(
        [&pending, 
         entries=FS::IdEntries_t(it, it_to),
         replicas=cfg_schema_replication->get()]() { 
          DB::Schema::Ptr schema;
          int err;
          for(cid_t cid : entries) {
            schema = Common::Files::Schema::load(
              err = Error::OK, cid, replicas);
            if(!err)
              Env::Mngr::schemas()->add(err, schema);
            else
              SWC_LOGF(LOG_ERROR, "Schema cid=%lu err=%d(%s)", 
                       cid, err, Error::get_text(err));
          }
          --pending;
        }
      );
      it += vol;
    } while(it_to < entries.end());
    --pending;

    while(pending) // keep_locking
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    m_schemas_set = true;
  }
  return true;
}


bool MngdColumns::columns_load() {
  int err;
  for(;;) {
    if(is_schemas_mngr(err = Error::OK)) {
      if(err) // hold-on
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      else
        break;
    } else {
      return false;
    }
  }

  auto groups = Env::Clients::get()->mngrs_groups->get_groups();
  if(groups.empty()) {
    SWC_LOG(LOG_WARN, "Empty Managers Groups")
    return false;
  }

  std::vector<DB::Schema::Ptr> entries;
  Env::Mngr::schemas()->all(entries);
  if(entries.empty()) {
    SWC_LOG(LOG_WARN, "Empty Schema Entries")
    return false;
  }

  for(auto& g : groups) {
    if(!(g->role & DB::Types::MngrRole::COLUMNS))
      continue;

    std::vector<cid_t> columns;
    for(auto& schema : entries) {
      if((!g->cid_begin || g->cid_begin <= schema->cid) && 
         (!g->cid_end || g->cid_end >= schema->cid)) {
        columns.push_back(schema->cid);
        // ? in-batches
      }
    }
    std::sort(columns.begin(), columns.end());
  
    SWC_LOGF(LOG_DEBUG,
      "Set Expected Columns to Load for cid(begin=%lu end=%lu) size=%lu",
      g->cid_begin, g->cid_end, columns.size());
    set_expect(columns, true);
  }
  
  for(auto& schema : entries) {
    SWC_ASSERT(schema->cid != DB::Schema::NO_CID);
    update_status(
      ColumnMngFunc::INTERNAL_LOAD, schema,
      Error::OK, true
    );
  }
  return true;
}

cid_t MngdColumns::get_next_cid() {
  cid_t cid = Common::Files::Schema::SYS_CID_END;
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
  if(schema->col_seq == DB::Types::KeySeq::UNKNOWN || 
     schema->col_type == DB::Types::Column::UNKNOWN) {
    err = Error::INVALID_ARGUMENT;
    return;
  }
  
  Column::create(err, cid);
  if(err)
    return;

  if(DB::Types::is_counter(schema->col_type))
    schema->cell_versions = 1;

  auto schema_save = DB::Schema::make(schema);
  schema_save->cid = cid;
  if(!schema_save->revision)
    schema_save->revision = Time::now_ns();
    
  Common::Files::Schema::save_with_validation(
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
     DB::Types::is_counter(old->col_type) 
      != DB::Types::is_counter(schema->col_type) ||
     (schema->cid <= Common::Files::Schema::SYS_CID_END && 
      schema->col_name.compare(old->col_name) != 0)) {
    err = Error::COLUMN_CHANGE_INCOMPATIBLE;
    return;
  }

  if(DB::Types::is_counter(schema->col_type))
    schema->cell_versions = 1;

  if(schema->cid < Common::Files::Schema::SYS_CID_END) { 
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

  Common::Files::Schema::save_with_validation(
    err, schema_save, cfg_schema_replication->get());
  if(!err) {
    Env::Mngr::schemas()->replace(schema_save);
    schema = Env::Mngr::schemas()->get(schema_save->cid);
  }
}

void MngdColumns::remove(int &err, cid_t cid) {
  auto col = get_column(err, cid);
  if(err && 
     err != Error::COLUMN_NOT_EXISTS &&
     err != Error::COLUMN_NOT_READY)
    return;

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

void MngdColumns::update(ColumnMngFunc func,
                         const DB::Schema::Ptr& schema, int err) {
  Env::Mngr::role()->req_mngr_inchain(
    std::make_shared<Comm::Protocol::Mngr::Req::ColumnUpdate>(
      func, schema, err));
}

void MngdColumns::update_status_ack(
                          ColumnMngFunc func,
                          const DB::Schema::Ptr& schema, int err) {
  switch(func) {
    case ColumnMngFunc::INTERNAL_LOAD_ALL: {
      columns_load();
      return;
    }
    case ColumnMngFunc::INTERNAL_ACK_LOAD:
      if(err)
        update(ColumnMngFunc::INTERNAL_LOAD, schema, Error::OK);
      return;
    case ColumnMngFunc::INTERNAL_ACK_CREATE:
    case ColumnMngFunc::INTERNAL_ACK_MODIFY:
      break;
    case ColumnMngFunc::INTERNAL_ACK_DELETE: {
      if(err)
        break;
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

  auto co_func = (ColumnMngFunc)(((uint8_t)func)-1);

  if(err)
    SWC_LOG_OUT(LOG_DEBUG,
      SWC_LOG_OSTREAM << "COLUMN-ACK func=" << co_func;
      Error::print(SWC_LOG_OSTREAM << ' ', err);
      schema->print(SWC_LOG_OSTREAM << ' ');
    );
  
  ColumnReq::Ptr pending;
  {
    std::scoped_lock lock(m_mutex);
    pending = m_action_pending;
    m_action_pending = nullptr;
  }

  if(!pending) {
    SWC_LOG_OUT(LOG_WARN,
      SWC_LOG_OSTREAM << "Missing Pending Req-Ack for func=" << co_func;
      schema->print(SWC_LOG_OSTREAM << "");
    );

  } else if(pending->schema->cid == schema->cid &&
            pending->function == co_func) {
    pending->response(err);

  } else {
    SWC_LOGF(LOG_WARN,
      "Not-Corresponding Pending Req-Ack func(%d==%d) cid(%lu==%lu)",
      co_func, pending->function, schema->cid, pending->schema->cid);
  }

  ColumnReq::Ptr req;
  if(!m_actions.deactivating(&req))
    run(req);
}

void MngdColumns::run(ColumnReq::Ptr req) {
  int err = Error::OK;
  do_procees:

  if(!m_run) {
    err = Error::SERVER_SHUTTING_DOWN;

  } else if(req->expired(1000)) {
    err = Error::REQUEST_TIMEOUT;

  } else if(req->schema->col_name.empty()) {
    err = Error::COLUMN_SCHEMA_NAME_EMPTY;

  } else if(!is_schemas_mngr(err) || err) {
    if(!err)
      err = Error::MNGR_NOT_ACTIVE;

  } else {
    std::scoped_lock lock(m_mutex);
    DB::Schema::Ptr schema = Env::Mngr::schemas()->get(
      req->schema->col_name);
    if(!schema && req->schema->cid != DB::Schema::NO_CID)
      schema = Env::Mngr::schemas()->get(req->schema->cid);

    switch(req->function) {
      case ColumnMngFunc::CREATE: {
        if(schema)
          err = Error::COLUMN_SCHEMA_NAME_EXISTS;
        else
          create(err, req->schema);
        break;
      }
      case ColumnMngFunc::MODIFY: {
        if(!schema)
          err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
        else if(req->schema->cid != DB::Schema::NO_CID &&
                schema->cid != req->schema->cid)
          err = Error::COLUMN_SCHEMA_NAME_EXISTS;
        else
          update(err, req->schema, schema);
        break;
      }
      case ColumnMngFunc::DELETE: {
        if(!schema)
          err = Error::COLUMN_SCHEMA_NAME_NOT_EXISTS;
        else if(schema->cid != req->schema->cid ||
                req->schema->col_name.compare(schema->col_name) != 0)
          err = Error::COLUMN_SCHEMA_NAME_NOT_CORRES;
        else if(schema->cid <= Common::Files::Schema::SYS_CID_END)
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

  if(err) {
    req->response(err);
    if(!m_actions.deactivating(&req))
      goto do_procees;

  } else {
    {
      std::scoped_lock lock(m_mutex_columns);
      m_action_pending = req;
    }
    update_status(req->function, req->schema, err, true);
  }
}



}} // namespace Manager

#include "swcdb/manager/Protocol/Rgr/req/ColumnsUnload.cc"
#include "swcdb/manager/Protocol/Rgr/req/RangeUnload.cc"
