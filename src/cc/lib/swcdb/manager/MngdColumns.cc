
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Semaphore.h"
#include "swcdb/manager/MngdColumns.h"
#include "swcdb/manager/Protocol/Mngr/req/ColumnUpdate.h"

#include "swcdb/db/client/Query/Select.h"
#include "swcdb/db/client/Query/Update.h"


namespace SWC { namespace Manager {

using ColumnMngFunc = Comm::Protocol::Mngr::Params::ColumnMng::Function;


MngdColumns::MngdColumns()
    : m_run(true), m_schemas_set(false),
      m_cid_active(false),
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
  m_run.store(false);
}

void MngdColumns::reset(bool schemas_mngr) {
  if(schemas_mngr) {
    {
      Core::MutexSptd::scope lock(m_mutex_schemas);
      m_schemas_set = false;
    }
    {
      Core::MutexSptd::scope lock(m_mutex_active);
      if(m_cid_active)
        return;
    }
    Core::MutexSptd::scope lock(m_mutex_schemas);
    Env::Mngr::schemas()->reset();
  }
}


bool MngdColumns::is_schemas_mngr(int& err) {
  if(Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS)) {
    Core::MutexSptd::scope lock(m_mutex_schemas);
    if(!m_schemas_set)
      err = Error::MNGR_NOT_INITIALIZED;
    return true;
  }
  return false;
}

bool MngdColumns::has_active() {
  Core::MutexSptd::scope lock(m_mutex_active);
  return m_cid_active;
}

bool MngdColumns::is_active(cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex_active);
  return m_cid_active && cid &&
        (!m_cid_begin || m_cid_begin <= cid) &&
        (!m_cid_end   || m_cid_end >= cid);
}

bool MngdColumns::active(cid_t& cid_begin, cid_t& cid_end) {
  Core::MutexSptd::scope lock(m_mutex_active);
  if(m_cid_active) {
    cid_begin = m_cid_begin;
    cid_end = m_cid_end;
  }
  return m_cid_active;
}

bool MngdColumns::expected_ready() {
  Core::MutexSptd::scope lock(m_mutex_expect);
  return m_expected_ready;
}

void MngdColumns::columns_ready(int& err) {
  {
    Core::MutexSptd::scope lock(m_mutex_active);
    if(!m_cid_active)
      return;
  }
  {
    Core::MutexSptd::scope lock(m_mutex_expect);
    if(!m_expected_ready) {
      err = Error::MNGR_NOT_INITIALIZED;
      return;
    }
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

  if((col = Env::Mngr::columns()->get_column(err, cid))) {
    col->state(err);
  } else {
    err = expected_ready()
      ? Error::COLUMN_NOT_EXISTS
      : Error::MNGR_NOT_INITIALIZED;
  }
  return col;
}

void MngdColumns::change_active(const cid_t cid_begin, const cid_t cid_end,
                                bool has_cols) {
  if(!has_cols) {
    {
      Core::MutexSptd::scope lock(m_mutex_active);
      if(!m_cid_active)
        return;
      m_cid_active = false;
    }
    {
      Core::MutexSptd::scope lock(m_mutex_expect);
      m_expected_ready = false;
    }
    Core::MutexSptd::scope lock(m_mutex_schemas);
    Env::Mngr::columns()->reset();
    if(!Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS))
      Env::Mngr::schemas()->reset();
    return;
  }

  {
    Core::MutexSptd::scope lock(m_mutex_active);
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

  if(!columns_load())
    update(
      ColumnMngFunc::INTERNAL_LOAD_ALL, DB::Schema::make(), Error::OK, 0);
}

void MngdColumns::action(const ColumnReq::Ptr& req) {
  if(m_actions.push_and_is_1st(req))
    Env::Mngr::post([this]() { run_actions(); });
}

void MngdColumns::set_expect(cid_t cid_begin, cid_t cid_end,
                             const std::vector<cid_t>& columns,
                             bool initial) {
  if(!initial &&
     Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS))
    return;

  bool active;
  {
    Core::MutexSptd::scope lock(m_mutex_active);
    active = m_cid_active && m_cid_begin == cid_begin && m_cid_end == cid_end;
  }
  if(!active)
    return Env::Mngr::role()->req_mngr_inchain(
      std::make_shared<Comm::Protocol::Mngr::Req::ColumnUpdate>(
        cid_begin, cid_end, columns));

  size_t need;
  {
    int err;
    auto _cols = Env::Mngr::columns();
    Core::MutexSptd::scope lock(m_mutex_expect);
    for(auto cid : columns) {
      if(std::find(m_expected_load.begin(), m_expected_load.end(), cid)
                                              == m_expected_load.end() &&
         !_cols->get_column(err = Error::OK, cid))
        m_expected_load.push_back(cid);
    }
    if(!(need = m_expected_load.size())) {
      m_expected_ready = true;
      m_expected_load = {};
    }
  }
  if(need)
    SWC_LOGF(LOG_DEBUG,
      "Expected Columns to Load size=%lu for cid(begin=%lu end=%lu)",
      need, cid_begin, cid_end);
}

void MngdColumns::update_status(ColumnMngFunc func,
                                const DB::Schema::Ptr& schema, int err,
                                uint64_t req_id, bool initial) {
  bool schemas_mngr = Env::Mngr::role()->is_active_role(
    DB::Types::MngrRole::SCHEMAS);

  if(!initial && schemas_mngr)
    return update_status_ack(func, schema, err, req_id);

  err = Error::OK;
  if(!is_active(schema->cid))
    return update(func, schema, err, req_id);

  bool do_update = false;
  switch(func) {

    case ColumnMngFunc::DELETE: {
      auto col = get_column(err, schema->cid);
      if(err &&
         err != Error::COLUMN_NOT_EXISTS &&
         err != Error::COLUMN_NOT_READY) {
        do_update = true;

      } else if(!col) {
        err = Error::OK;
        remove(schema, 0, req_id);
        do_update = true;

      } else if(!col->do_remove()) {
        do_update = true;

      } else {
        SWC_LOGF(LOG_DEBUG, "DELETING cid=%lu", schema->cid);
        std::vector<rgrid_t> rgrids;
        col->assigned(rgrids);
        do_update = rgrids.empty();
        if(do_update)
          remove(schema, 0, req_id);
        else
          Env::Mngr::rangers()->column_delete(schema, req_id, rgrids);
      }
      break;
    }

    case ColumnMngFunc::CREATE:
    case ColumnMngFunc::INTERNAL_LOAD: {
      bool init = Env::Mngr::columns()->is_an_initialization(err, schema);
      if(init || !err) {
        if(!schemas_mngr)
          Env::Mngr::schemas()->replace(schema);
        {
          Core::MutexSptd::scope lock(m_mutex_expect);
          if(!m_expected_ready) {
            auto it = std::find(
              m_expected_load.begin(), m_expected_load.end(), schema->cid);
            if(it != m_expected_load.end()) {
              m_expected_load.erase(it);
              if(m_expected_load.empty()) {
                m_expected_load = {};
                m_expected_ready = true;
              }
              init = true;
              SWC_LOGF(LOG_DEBUG,
                "Expected Column(%lu) Loaded remain=%lu",
                schema->cid, m_expected_load.size());
            }
          }
        }
      }
      if(init)
        Env::Mngr::rangers()->assign_ranges();

      do_update = true;
      break;
    }

    case ColumnMngFunc::MODIFY: {
      do_update = true;
      auto col = Env::Mngr::columns()->get_column(err, schema->cid);
      err = Error::OK;
      if(col && !schemas_mngr) {
        auto existing = Env::Mngr::schemas()->get(schema->cid);
        if((do_update = !existing || !existing->equal(schema)))
          Env::Mngr::schemas()->replace(schema);
      }
      do_update = col && do_update &&
                  !Env::Mngr::rangers()->update(col, schema, req_id, true);
      break;
    }

    default:
      break;
  }

  if(do_update) {
    auto co_func = ColumnMngFunc(uint8_t(func)+1);
    schemas_mngr
      ? update_status_ack(co_func, schema, err, req_id)
      : update(co_func, schema, err, req_id);
  }
}

void MngdColumns::remove(const DB::Schema::Ptr& schema,
                         rgrid_t rgrid, uint64_t req_id) {
  int err = Error::OK;
  auto col = Env::Mngr::columns()->get_column(err, schema->cid);
  if(col && !col->finalize_remove(err, rgrid))
    return;

  Env::Mngr::columns()->remove(schema->cid);
  if(!Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS))
    Env::Mngr::schemas()->remove(schema->cid);

  if(DB::Types::MetaColumn::is_master(schema->cid))
    return update(
      ColumnMngFunc::INTERNAL_ACK_DELETE, schema, Error::OK, req_id);

  cid_t meta_cid = DB::Types::MetaColumn::get_sys_cid(
    schema->col_seq, DB::Types::MetaColumn::get_range_type(schema->cid));
  auto col_spec = DB::Specs::Column::make_ptr(
    meta_cid, {DB::Specs::Interval::make_ptr()});
  auto& intval = col_spec->intervals.front();
  auto& key_intval = intval->key_intervals.add();
  key_intval->start.add(std::to_string(schema->cid), Condition::EQ);
  key_intval->start.add("", Condition::GE);
  intval->flags.set_only_keys();

  auto selector = std::make_shared<client::Query::Select>(
    [this, req_id, schema, meta_cid]
    (const client::Query::Select::Result::Ptr& result) {
      DB::Cells::Result cells;
      int err = result->err;
      if(!err) {
        auto col = result->get_columnn(meta_cid);
        if(!(err = col->error()) && !col->empty())
          col->get_cells(cells);
      }
      if(err) {
        SWC_LOGF(LOG_WARN,
        "Column(cid=%lu meta_cid=%lu) "
        "Range MetaData might remained, result-err=%d(%s)",
        schema->cid, meta_cid, err, Error::get_text(err));
      }
      if(err || cells.empty())
        return update(
          ColumnMngFunc::INTERNAL_ACK_DELETE, schema, Error::OK, req_id);

      SWC_LOG_OUT(LOG_INFO,
        SWC_LOG_OSTREAM << "Column(cid=" << schema->cid
          << " meta_cid=" << meta_cid << ')'
          << " deleting Range MetaData, remained(" << cells.size() << ')'
          << " cells=[";
        for(auto cell : cells)
          cell->key.print(SWC_LOG_OSTREAM << "\n\t");
        SWC_LOG_OSTREAM << "\n]";
      );

      auto updater = std::make_shared<client::Query::Update>(
        [this, req_id, schema, meta_cid]
        (const client::Query::Update::Result::Ptr& res) {
          int err = res->error();
          if(err) {
            SWC_LOGF(LOG_WARN,
              "Column(cid=%lu meta_cid=%lu) "
              "Range MetaData might remained, update-err=%d(%s)",
              schema->cid, meta_cid, err, Error::get_text(err));
          }
          return update(
            ColumnMngFunc::INTERNAL_ACK_DELETE, schema, Error::OK, req_id);
        },
        Env::Mngr::io()
      );
      updater->columns->create(
        meta_cid, schema->col_seq, 1, 0, DB::Types::Column::PLAIN);
      auto col = updater->columns->get_col(meta_cid);
      for(auto cell : cells) {
        cell->flag = DB::Cells::DELETE;
        col->add(*cell);
        updater->commit_or_wait(col);
      }
      updater->commit_if_need();
    },
    false,
    Env::Mngr::io()
  );

  selector->specs.columns.push_back(col_spec);
  selector->scan(err);

  if(err) {
    SWC_LOGF(LOG_WARN,
      "Column(cid=%lu meta_cid=%lu) "
      "Range MetaData might remained, scan-err=%d(%s)",
      schema->cid, meta_cid, err, Error::get_text(err));
    return update(
      ColumnMngFunc::INTERNAL_ACK_DELETE, schema, Error::OK, req_id);
  }
}


void MngdColumns::print(std::ostream& out) {
  Env::Mngr::columns()->print(out);
}

bool MngdColumns::initialize() {
  Core::MutexSptd::scope lock(m_mutex_schemas);
  if(m_schemas_set)
    return false;

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

  Core::Semaphore pending(Env::Mngr::io()->get_size()/4 + 1, 1);
  int32_t vol = entries.size()/pending.available() + 1;
  auto it = entries.begin();
  FS::IdEntries_t::iterator it_to;
  do {
    pending.acquire();
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
        pending.release();
      }
    );
    it += vol;
  } while(it_to < entries.end());

  pending.release();
  pending.wait_all();

  m_schemas_set = true;
  return true;
}


bool MngdColumns::columns_load() {
  if(m_columns_load.running())
    return true;
  for(int err;;) {
    if(is_schemas_mngr(err = Error::OK)) {
      if(err) // hold-on
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      else
        break;
    } else {
      m_columns_load.stop();
      return false;
    }
  }

  auto groups = Env::Clients::get()->mngrs_groups->get_groups();
  if(groups.empty())
    SWC_LOG(LOG_WARN, "Empty Managers Groups")

  std::vector<DB::Schema::Ptr> entries;
  Env::Mngr::schemas()->all(entries);
  if(entries.empty())
    SWC_LOG(LOG_WARN, "Empty Schema Entries")

  if(groups.empty() || entries.empty()) {
    m_columns_load.stop();
    return false;
  }

  std::sort(groups.begin(), groups.end(), []
    (const client::Mngr::Group::Ptr& g1, const client::Mngr::Group::Ptr& g2) {
      return (!g1->cid_begin || g1->cid_begin < g2->cid_begin) &&
             (g2->cid_end && g1->cid_end < g2->cid_end); });

  while(!Env::Mngr::role()->are_all_active(groups))
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // hold-on

  std::sort(entries.begin(), entries.end(),
    [](const DB::Schema::Ptr& s1, const DB::Schema::Ptr& s2) {
      return s1->cid < s2->cid; });

  auto it = entries.begin();
  auto it_batch = it;
  size_t g_batches;
  for(auto& g : groups) {
    if(!(g->role & DB::Types::MngrRole::COLUMNS))
      continue;
    g_batches = 0;
    std::vector<cid_t> columns;

    make_batch:
      it_batch = it;
      for(;it < entries.end() && columns.size() < 1000 &&
           (!g->cid_begin || g->cid_begin <= (*it)->cid) &&
           (!g->cid_end || g->cid_end >= (*it)->cid); ++it) {
        columns.push_back((*it)->cid);
      }
    if(++g_batches > 1 && columns.empty())
      continue;

    SWC_LOGF(LOG_DEBUG,
      "Set Expected Columns Load cid(begin=%lu end=%lu) batch=%lu size=%lu",
      g->cid_begin, g->cid_end, g_batches, it - it_batch);
    set_expect(g->cid_begin, g->cid_end, columns, true);

    for(; it_batch < it; ++it_batch) {
      update_status(
        ColumnMngFunc::INTERNAL_LOAD, *it_batch, Error::OK, 0, true);
    }
    if(it != entries.end() && columns.size() == 1000) {
      columns.clear();
      goto make_batch;
    }
  }

  m_columns_load.stop();
  return true;
}

cid_t MngdColumns::get_next_cid() {
  cid_t cid = Common::Files::Schema::SYS_CID_END;
  while(++cid && Env::Mngr::schemas()->get(cid));
  // if schema does exist on fs (? sanity-check)
  return cid; // err !cid
}

void MngdColumns::create(int &err, DB::Schema::Ptr& schema) {
  cid_t cid;
  if(schema->cid == DB::Schema::NO_CID) {
    cid = get_next_cid();
    if(!cid) {
      err = Error::COLUMN_REACHED_ID_LIMIT;
      return;
    }
  } else {
    cid = schema->cid;
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
      schema->col_name.compare(old->col_name))) {
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

void MngdColumns::update(ColumnMngFunc func,
                         const DB::Schema::Ptr& schema, int err,
                         uint64_t req_id) {
  Env::Mngr::role()->req_mngr_inchain(
    std::make_shared<Comm::Protocol::Mngr::Req::ColumnUpdate>(
      func, schema, err, req_id));
}

void MngdColumns::update_status_ack(ColumnMngFunc func,
                                    const DB::Schema::Ptr& schema, int err,
                                    uint64_t req_id) {
  switch(func) {
    case ColumnMngFunc::INTERNAL_LOAD_ALL: {
      columns_load();
      return;
    }
    case ColumnMngFunc::INTERNAL_ACK_LOAD:
      if(err) {
        auto existing = Env::Mngr::schemas()->get(schema->cid);
        if(existing)
          update_status(
            ColumnMngFunc::INTERNAL_LOAD, existing, Error::OK, req_id, true);
      }
      return;
    case ColumnMngFunc::INTERNAL_ACK_CREATE:
    case ColumnMngFunc::INTERNAL_ACK_MODIFY:
      break;
    case ColumnMngFunc::INTERNAL_ACK_DELETE: {
      if(err)
        break;
      Core::MutexSptd::scope lock(m_mutex_schemas);
      if(!m_schemas_set) {
        err = Error::MNGR_NOT_ACTIVE;
      } else if(!Env::Mngr::schemas()->get(schema->cid)) {
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

  auto co_func = ColumnMngFunc(uint8_t(func)-1);

  if(err)
    SWC_LOG_OUT(LOG_DEBUG,
      SWC_LOG_OSTREAM << "COLUMN-ACK func=" << co_func;
      Error::print(SWC_LOG_OSTREAM << ' ', err);
      schema->print(SWC_LOG_OSTREAM << ' ');
    );

  ColumnReq::Ptr pending = nullptr;
  {
    Core::MutexSptd::scope lock(m_mutex_actions);
    auto it = m_actions_pending.find(req_id);
    if(it != m_actions_pending.end()) {
      pending = it->second;
      m_actions_pending.erase(it);
    }
  }

  if(pending) {
    pending->response(err);
  } else {
    SWC_LOG_OUT(LOG_WARN,
      SWC_LOG_OSTREAM
        << "Missing Pending Req-Ack for func=" << co_func << " req_id=" << req_id;
      schema->print(SWC_LOG_OSTREAM << ' ');
    );
  }
}

void MngdColumns::run_actions() {
  int err;
  ColumnReq::Ptr req;
  do {
    err = Error::OK;
    req = m_actions.front();

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
      Core::MutexSptd::scope lock(m_mutex_schemas);
      DB::Schema::Ptr schema = Env::Mngr::schemas()->get(
        req->schema->col_name);
      if(!schema && req->schema->cid != DB::Schema::NO_CID)
        schema = Env::Mngr::schemas()->get(req->schema->cid);

      if(!m_schemas_set)
        err = Error::MNGR_NOT_ACTIVE;

      if(!err) switch(req->function) {
        case ColumnMngFunc::CREATE: {
          if(schema)
            err = req->schema->cid == schema->cid
              ? Error::COLUMN_SCHEMA_ID_EXISTS
              : Error::COLUMN_SCHEMA_NAME_EXISTS;
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
                  req->schema->col_name.compare(schema->col_name))
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
      if(err != Error::REQUEST_TIMEOUT)
        req->response(err);

    } else {
      req->id = Time::now_ns();
      {
        Core::MutexSptd::scope lock(m_mutex_actions);
        m_actions_pending[req->id] = req;
        SWC_LOG_OUT(LOG_DEBUG, SWC_LOG_OSTREAM
          << "Pending Requests=" << m_actions_pending.size(); );
      }
      update_status(req->function, req->schema, Error::OK, req->id, true);
    }

  } while(m_actions.pop_and_more());

}



}} // namespace Manager

#include "swcdb/manager/Protocol/Rgr/req/ColumnsUnload.cc"
#include "swcdb/manager/Protocol/Rgr/req/RangeUnload.cc"
