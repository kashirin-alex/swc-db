/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Semaphore.h"
#include "swcdb/manager/MngdColumns.h"
#include "swcdb/manager/Protocol/Mngr/req/ColumnUpdate.h"

#include "swcdb/db/client/Query/Select/Scanner.h"
#include "swcdb/db/client/Query/Update/Committer.h"
#include "swcdb/db/client/Query/Select/Handlers/Common.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"


namespace SWC { namespace Manager {

using ColumnMngFunc = Comm::Protocol::Mngr::Params::ColumnMng::Function;


MngdColumns::MngdColumns()
    : cfg_schema_replication(
        Env::Config::settings()->get<Config::Property::V_GUINT8>(
          "swc.mngr.schema.replication")),
      cfg_schemas_store_cap(
        Env::Config::settings()->get<Config::Property::V_GUINT64>(
          "swc.mngr.schemas.store.from.capacity")),
      cfg_schemas_store_blksz(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.schemas.store.block.size")),
      cfg_schemas_store_encoder(
        Env::Config::settings()->get<Config::Property::V_GENUM>(
          "swc.mngr.schemas.store.block.encoder")),
      cfg_delay_cols_init(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.ranges.assign.delay.afterColumnsInit")),
      m_run(true), m_schemas_set(false),
      m_cid_active(false),
      m_cid_begin(DB::Schema::NO_CID), m_cid_end(DB::Schema::NO_CID),
      m_expected_remain(STATE_COLUMNS_NOT_INITIALIZED),
      m_last_used_cid(DB::Types::SystemColumn::SYS_CID_END) {
}

void MngdColumns::stop() {
  m_run.store(false);
  {
    Core::MutexSptd::scope lock(m_mutex_active);
    m_expected_remain = STATE_COLUMNS_NOT_INITIALIZED;
  }
  bool expecting = false;
  size_t pending = 0;
  for(size_t c=0; ; ++c) {
    if(m_actions.empty() && !m_columns_load) {
      {
        Core::MutexSptd::scope lock(m_mutex_active);
        expecting = !m_expected_load.empty();
      }
      {
        Core::MutexSptd::scope lock(m_mutex_actions);
        pending = m_actions_pending.size();
      }
      if(!expecting && !pending)
        break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if(c % 1000 == 0)
      SWC_LOGF(LOG_DEBUG,
        "Stop-Waiting colums load=%s actions=%lu expecting=%s pending=%lu",
        m_columns_load ? "True" : "False", m_actions.size(),
        expecting ? "True" : "False", pending);
  }
}

void MngdColumns::create_schemas_store() {
  int err = Error::OK;
  if(is_schemas_mngr(err) && (!err || err == Error::SERVER_SHUTTING_DOWN) &&
     Env::Mngr::schemas()->size() >= cfg_schemas_store_cap->get()) {
    Env::Mngr::schemas()->store_create(
      err,
      cfg_schema_replication->get(),
      cfg_schemas_store_blksz->get(),
      DB::Types::Encoder(cfg_schemas_store_encoder->get())
    );
    SWC_LOG_OUT(LOG_INFO,
      SWC_LOG_OSTREAM
        << "Create Schemas-Store schemas=" << Env::Mngr::schemas()->size();
      Error::print(SWC_LOG_OSTREAM << ' ', err);
    );
  }
}

void MngdColumns::reset(bool schemas_mngr) {
  if(schemas_mngr) {
    {
      Core::MutexSptd::scope lock(m_mutex_schemas);
      m_schemas_set.store(false);
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


SWC_CAN_INLINE
bool MngdColumns::is_schemas_mngr(int& err) {
  if(Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS)) {
    if(m_schemas_set) {
      Core::MutexSptd::scope lock(m_mutex_schemas);
      if(m_schemas_set) {
        if(!m_run)
          err = Error::SERVER_SHUTTING_DOWN;
        return true;
      }
    }
    err = Error::MNGR_NOT_INITIALIZED;
    return true;
  }
  return false;
}

SWC_CAN_INLINE
bool MngdColumns::has_active() noexcept {
  Core::MutexSptd::scope lock(m_mutex_active);
  return m_cid_active;
}

SWC_CAN_INLINE
bool MngdColumns::is_active(cid_t cid) noexcept {
  Core::MutexSptd::scope lock(m_mutex_active);
  return m_cid_active && cid &&
        (!m_cid_begin || m_cid_begin <= cid) &&
        (!m_cid_end   || m_cid_end >= cid);
}

bool MngdColumns::active(cid_t& cid_begin, cid_t& cid_end) noexcept {
  Core::MutexSptd::scope lock(m_mutex_active);
  if(m_cid_active) {
    cid_begin = m_cid_begin;
    cid_end = m_cid_end;
  }
  return m_cid_active;
}

SWC_CAN_INLINE
bool MngdColumns::expected_ready() noexcept {
  Core::MutexSptd::scope lock(m_mutex_active);
  return m_expected_remain == 0;
}

void MngdColumns::columns_ready(int& err) {
  {
    Core::MutexSptd::scope lock(m_mutex_active);
    if(!m_cid_active)
      return;
    if(m_expected_remain > 0) {
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
      m_expected_remain = STATE_COLUMNS_NOT_INITIALIZED;
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
    m_expected_remain = STATE_COLUMNS_NOT_INITIALIZED;
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

SWC_CAN_INLINE
void MngdColumns::action(const ColumnReq::Ptr& req) {
  struct Task {
    MngdColumns* ptr;
    SWC_CAN_INLINE
    Task(MngdColumns* ptr) noexcept : ptr(ptr) { }
    void operator()() { ptr->run_actions(); }
  };
  if(m_actions.push_and_is_1st(req))
    Env::Mngr::post(Task(this));
}

void MngdColumns::set_expect(cid_t cid_begin, cid_t cid_end, uint64_t total,
                             Core::Vector<cid_t>&& columns,
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
      Comm::Protocol::Mngr::Req::ColumnUpdate::Ptr(
        new Comm::Protocol::Mngr::Req::ColumnUpdate(
          cid_begin, cid_end, total, std::move(columns))
        )
      );

  size_t need = STATE_COLUMNS_NOT_INITIALIZED;
  if(m_run) {
    int err;
    auto _cols = Env::Mngr::columns();
    Core::MutexSptd::scope lock(m_mutex_active);
    if(m_expected_remain == 0 ||  m_expected_remain == need || total == need)
      m_expected_remain = total;
    for(auto cid : columns) {
      if(std::find(m_expected_load.cbegin(), m_expected_load.cend(), cid)
                                              == m_expected_load.cend() &&
         !_cols->get_column(err = Error::OK, cid)) {
        m_expected_load.push_front(cid);
      } else {
        --m_expected_remain;
      }
    }
    need = m_expected_remain;
  }
  if(need) {
    if(need == STATE_COLUMNS_NOT_INITIALIZED) {
      SWC_LOGF(LOG_DEBUG,
        "Expected Columns to Load CANCELLED for cid(begin=%lu end=%lu)",
        cid_begin, cid_end);
    } else {
      SWC_LOGF(LOG_DEBUG,
        "Expected Columns to Load size=%lu for cid(begin=%lu end=%lu)",
        need, cid_begin, cid_end);
    }
  }
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

  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_OSTREAM << "ColumnUpdate Request func=" << int(func)
                    << " req_id=" << req_id;
    schema->print(SWC_LOG_OSTREAM << ' ');
  );

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
        Core::Vector<rgrid_t> rgrids;
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
      bool init = m_run &&
                  Env::Mngr::columns()->is_an_initialization(err, schema);
      if(init || !err || !m_run) {
        if(!schemas_mngr)
          Env::Mngr::schemas()->replace(schema);
        {
          Core::MutexSptd::scope lock(m_mutex_active);
          if(m_expected_load.remove(schema->cid)) {
            init = m_run;
            if(m_expected_remain != STATE_COLUMNS_NOT_INITIALIZED) {
              --m_expected_remain;
              SWC_LOGF(LOG_DEBUG, "Expected Column(%lu) Loaded remain=%lu",
                                  schema->cid, m_expected_remain);
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

  Env::Mngr::rangers()->wait_health_check(schema->cid);

  Env::Mngr::columns()->remove(schema->cid);
  if(!Env::Mngr::role()->is_active_role(DB::Types::MngrRole::SCHEMAS))
    Env::Mngr::schemas()->remove(schema->cid);

  if(DB::Types::SystemColumn::is_master(schema->cid))
    return update(
      ColumnMngFunc::INTERNAL_ACK_DELETE, schema, Error::OK, req_id);

  auto hdlr = client::Query::Select::Handlers::Common::make(
    Env::Clients::get(),
    [this, req_id, schema]
    (const client::Query::Select::Handlers::Common::Ptr& hdlr) {
      cid_t meta_cid = DB::Types::SystemColumn::get_sys_cid(
        schema->col_seq,
        DB::Types::SystemColumn::get_range_type(schema->cid)
      );
      DB::Cells::Result cells_meta;
      DB::Cells::Result cells_rgrdata;
      int err = hdlr->state_error;
      if(!err) {
        auto col = hdlr->get_columnn(meta_cid);
        if(!(err = col->error()) && !col->empty())
          col->get_cells(cells_meta);

        col = hdlr->get_columnn(DB::Types::SystemColumn::SYS_RGR_DATA);
        if(!col->error() && !col->empty())
          col->get_cells(cells_rgrdata);
      }
      if(err && cells_meta.empty()) {
        SWC_LOGF(LOG_WARN,
        "Column(cid=%lu meta_cid=%lu) "
        "Range MetaData might remained, result-err=%d(%s)",
        schema->cid, meta_cid, err, Error::get_text(err));
      }

      if(err || (cells_meta.empty() && cells_rgrdata.empty()))
        return update(
          ColumnMngFunc::INTERNAL_ACK_DELETE, schema, Error::OK, req_id);

      if(!cells_meta.empty()) {
        SWC_LOG_OUT(LOG_INFO,
          SWC_LOG_OSTREAM << "Column(cid=" << schema->cid
            << " meta_cid=" << meta_cid << ')'
            << " deleting Range MetaData, remained(" << cells_meta.size() << ')'
            << " cells=[";
          for(auto cell : cells_meta)
            cell->key.print(SWC_LOG_OSTREAM << "\n\t");
          SWC_LOG_OSTREAM << "\n]";
        );
      }

      auto updater = client::Query::Update::Handlers::Common::make(
        Env::Clients::get(),
        [this, req_id, schema]
        (const client::Query::Update::Handlers::Common::Ptr& hdlr) {
          cid_t meta_cid = DB::Types::SystemColumn::get_sys_cid(
            schema->col_seq,
            DB::Types::SystemColumn::get_range_type(schema->cid)
          );
          int err = hdlr->error();
          auto col = hdlr->get(meta_cid);
          if(col && (err || col->error())) {
            SWC_LOGF(LOG_WARN,
              "Column(cid=%lu meta_cid=%lu) "
              "Range MetaData might remained, "
              "updater-err=%d(%s) colm-err=%d(%s)",
              schema->cid, meta_cid,
              err, Error::get_text(err),
              col->error(), Error::get_text(col->error())
            );
          }
          col = hdlr->get(DB::Types::SystemColumn::SYS_RGR_DATA);
          if(col && (err || col->error())) {
            SWC_LOGF(LOG_WARN,
              "Column(cid=%lu rgrdata_cid=%lu) "
              "Range RgrData might remained, "
              "updater-err=%d(%s) colm-err=%d(%s)",
              schema->cid, DB::Types::SystemColumn::SYS_RGR_DATA,
              err, Error::get_text(err),
              col->error(), Error::get_text(col->error())
            );
          }
          return update(
            ColumnMngFunc::INTERNAL_ACK_DELETE, schema, Error::OK, req_id);
        },
        Env::Mngr::io()
      );

      updater->completion.increment();
      if(!cells_meta.empty()) {
        auto& col = updater->create(
          meta_cid, schema->col_seq, 1, 0, DB::Types::Column::SERIAL);
        for(auto cell : cells_meta) {
          cell->flag = DB::Cells::DELETE;
          col->add(*cell);
          updater->commit_or_wait(col.get(), 1);
        }
      }
      if(!cells_rgrdata.empty()) {
        auto& col = updater->create(
          DB::Types::SystemColumn::SYS_RGR_DATA,
          DB::Types::KeySeq::VOLUME, 1, 0, DB::Types::Column::PLAIN);
        for(auto cell : cells_rgrdata) {
          cell->flag = DB::Cells::DELETE;
          col->add(*cell);
          updater->commit_or_wait(col.get(), 1);
        }
      }
      updater->response(Error::OK);
    },
    false,
    Env::Mngr::io()
  );

  DB::Specs::Interval spec(DB::Types::Column::SERIAL);
  spec.flags.set_only_keys();
  auto& key_intval = spec.key_intervals.add();
  key_intval.start.reserve(2);
  key_intval.start.add(std::to_string(schema->cid), Condition::EQ);
  key_intval.start.add("", Condition::GE);

  cid_t meta_cid = DB::Types::SystemColumn::get_sys_cid(
    schema->col_seq, DB::Types::SystemColumn::get_range_type(schema->cid));
  if(DB::Types::SystemColumn::is_rgr_data_on_fs(schema->cid)) {
    hdlr->scan(schema->col_seq, meta_cid, std::move(spec));
  } else {
    hdlr->completion.increment();
    hdlr->scan(schema->col_seq, meta_cid, spec);
    spec.values.col_type = DB::Types::Column::PLAIN;
    hdlr->scan(
      DB::Types::KeySeq::VOLUME,
      DB::Types::SystemColumn::SYS_RGR_DATA,
      std::move(spec)
    );
    if(hdlr->completion.is_last())
      hdlr->response(Error::OK);
  }
}


void MngdColumns::print(std::ostream& out) {
  Env::Mngr::columns()->print(out);
}

bool MngdColumns::initialize() {
  Core::MutexSptd::scope lock(m_mutex_schemas);
  if(m_schemas_set)
    return false;

  SWC_LOG(LOG_INFO, "Load-Schemas START");
  int err = Error::OK;
  try {
    if(Env::Mngr::schemas()->store_load(err)) {
      m_schemas_set.store(true);
      SWC_LOGF(LOG_INFO, "Load-Schemas FINISH schemas=%lu",
                          Env::Mngr::schemas()->size());
      return true;
    }
    if(err) {
      SWC_LOG_OUT(LOG_WARN,
        SWC_LOG_OSTREAM << "Problem loading Schemas-Store ";
        Error::print(SWC_LOG_OSTREAM, err);
      );
    }
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

  FS::IdEntries_t entries;
  Columns::columns_by_fs(err = Error::OK, entries);
  if(err || !m_run) {
    if(err != ENOENT)
      return false;
    err = Error::OK;
  }
  // initialize / recover sys-columns
  for(cid_t cid=1; cid <= DB::Types::SystemColumn::SYS_CID_END; ++cid) {
    if(std::find(entries.cbegin(), entries.cend(), cid) == entries.cend()) {
      Column::create(err, cid);
      entries.push_back(cid);
    }
  }

  uint32_t sz = Env::Mngr::res().concurrency() / 2;
  Core::Semaphore pending(sz ? sz : 1, 1);
  int32_t vol = entries.size()/pending.available() + 1;
  auto it = entries.cbegin();
  FS::IdEntries_t::const_iterator it_to;

  struct Task {
    MngdColumns*                    ptr;
    Core::Semaphore*                pending;
    FS::IdEntries_t::const_iterator it_begin;
    FS::IdEntries_t::const_iterator it_end;
    uint8_t                         replicas;
    SWC_CAN_INLINE
    Task(MngdColumns* ptr,
         Core::Semaphore* pending,
         FS::IdEntries_t::const_iterator it_begin,
         FS::IdEntries_t::const_iterator it_end,
         uint8_t replicas) noexcept
        : ptr(ptr), pending(pending),
          it_begin(it_begin), it_end(it_end),
          replicas(replicas) {
    }
    void operator()() {
      DB::Schema::Ptr schema;
      int err;
      for(cid_t cid; ptr->m_run && it_begin != it_end; ++it_begin) {
        cid = *it_begin;
        SWC_LOGF(LOG_DEBUG, "Schema Loading cid=%lu", cid);
        schema = Common::Files::Schema::load(err=Error::OK, cid, replicas);
        if(!err)
          Env::Mngr::schemas()->add(err, schema);
        else
          SWC_LOGF(LOG_ERROR, "Schema cid=%lu err=%d(%s)",
                   cid, err, Error::get_text(err));
      }
      pending->release();
    }
  };
  do {
    pending.acquire();
    it_to = entries.cend() - it > vol ? (it + vol) : entries.cend();
    Env::Mngr::post(
      Task(this, &pending, it, it_to, cfg_schema_replication->get()));
    it += it_to - it;
  } while(m_run && it_to != entries.cend());

  pending.release();
  pending.wait_all();

  if(m_run) {
    m_schemas_set.store(true);
    SWC_LOGF(LOG_INFO, "Load-Schemas FINISH schemas=%lu",
                        Env::Mngr::schemas()->size());
    return true;
  } else {
    Env::Mngr::schemas()->reset();
    SWC_LOG(LOG_INFO, "Load-Schemas CANCLLED");
    return false;
  }
}


bool MngdColumns::columns_load() {
  if(m_columns_load.running())
    return true;
  for(int err;;) {
    if(is_schemas_mngr(err = Error::OK) && m_run) {
      if(err) // hold-on
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      else
        break;
    } else {
      m_columns_load.stop();
      return false;
    }
  }

  auto groups = Env::Clients::get()->managers.groups->get_groups();
  if(groups.empty())
    SWC_LOG(LOG_WARN, "Empty Managers Groups")

  std::vector<DB::Schema::Ptr> entries;
  Env::Mngr::schemas()->all(entries);
  if(entries.empty())
    SWC_LOG(LOG_WARN, "Empty Schema Entries")

  if(!m_run || groups.empty() || entries.empty()) {
    m_columns_load.stop();
    return false;
  }

  std::sort(groups.begin(), groups.end(), []
    (const client::Mngr::Group::Ptr& g1, const client::Mngr::Group::Ptr& g2) {
      return (!g1->cid_begin || g1->cid_begin < g2->cid_begin) &&
             (g2->cid_end && g1->cid_end < g2->cid_end); });

  while(!Env::Mngr::role()->are_all_active(groups) && m_run)
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // hold-on

  std::sort(entries.begin(), entries.end(),
    [](const DB::Schema::Ptr& s1, const DB::Schema::Ptr& s2) {
      return s1->cid < s2->cid; });

  auto it = entries.cbegin();
  auto it_batch = it;
  size_t g_batches;
  for(auto& g : groups) {
    if(!m_run) {
      m_columns_load.stop();
      return false;
    }
    if(!(g->role & DB::Types::MngrRole::COLUMNS))
      continue;
    uint64_t total = 0;
    for(auto itc = it;
        itc != entries.cend() &&
        (!g->cid_begin || g->cid_begin <= (*itc)->cid) &&
        (!g->cid_end || g->cid_end >= (*itc)->cid); ++itc) {
      ++total;
    }
    g_batches = 0;
    Core::Vector<cid_t> columns;

    make_batch:
      it_batch = it;
      columns.reserve(1000);
      for(;it != entries.cend() && columns.size() < 1000 &&
           (!g->cid_begin || g->cid_begin <= (*it)->cid) &&
           (!g->cid_end || g->cid_end >= (*it)->cid); ++it) {
        columns.push_back((*it)->cid);
      }
      if(++g_batches > 1 && columns.empty())
        continue;

      int64_t sz = columns.size();
      SWC_LOGF(LOG_DEBUG,
        "Set Expected Columns Load cid(begin=%lu end=%lu) %lu(%ld/%lu)",
        g->cid_begin, g->cid_end, g_batches, sz, total);
      set_expect(g->cid_begin, g->cid_end, total, std::move(columns), true);
      columns.clear();

      for(; it_batch != it; ++it_batch) {
        update_status(
          ColumnMngFunc::INTERNAL_LOAD, *it_batch, Error::OK, 0, true);
      }
      if(!m_run) {
        // Cancel Expectations
        set_expect(
          g->cid_begin, g->cid_end,
          STATE_COLUMNS_NOT_INITIALIZED, Core::Vector<cid_t>(), true
        );
        m_columns_load.stop();
        return false;
      }
      if(it != entries.cend() && sz == 1000)
        goto make_batch;
  }

  m_columns_load.stop();
  return true;
}

cid_t MngdColumns::get_next_cid() {
  for(cid_t cid; ;) {
    cid = m_last_used_cid.add_rslt(1);
    if(cid <= DB::Types::SystemColumn::SYS_CID_END) {
      cid = DB::Types::SystemColumn::SYS_CID_END;
      m_last_used_cid.store(++cid);
    }
    if(!Env::Mngr::schemas()->get(cid)) {
      // if schema does exist on fs (? sanity-check)
      return cid; // err !cid
    }
  }
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

  if(!err) {
    schema = Env::Mngr::schemas()->get(schema_save->cid);
  } else {
    Column::remove(err, cid);
    if(m_last_used_cid > cid)
      m_last_used_cid.store(cid - 1);
  }
}

void MngdColumns::update(int &err, DB::Schema::Ptr& schema,
                         const DB::Schema::Ptr& old) {
  if(old->col_seq != schema->col_seq ||
     DB::Types::is_counter(old->col_type)
      != DB::Types::is_counter(schema->col_type) ||
     (schema->cid <= DB::Types::SystemColumn::SYS_CID_END &&
      !Condition::str_eq(schema->col_name, old->col_name) )) {
    err = Error::COLUMN_CHANGE_INCOMPATIBLE;
    return;
  }

  if(DB::Types::is_counter(schema->col_type))
    schema->cell_versions = 1;

  if(schema->cid < DB::Types::SystemColumn::SYS_CID_END) {
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

SWC_CAN_INLINE
void MngdColumns::update(ColumnMngFunc func,
                         const DB::Schema::Ptr& schema, int err,
                         uint64_t req_id) {
  Env::Mngr::role()->req_mngr_inchain(
    Comm::Protocol::Mngr::Req::ColumnUpdate::Ptr(
      new Comm::Protocol::Mngr::Req::ColumnUpdate(func, schema, err, req_id)
    )
  );
}

void MngdColumns::update_status_ack(ColumnMngFunc func,
                                    const DB::Schema::Ptr& schema, int err,
                                    uint64_t req_id) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_OSTREAM << "ColumnUpdate ACK Request func=" << int(func)
                    << " req_id=" << req_id;
    Error::print(SWC_LOG_OSTREAM << ' ', err);
    schema->print(SWC_LOG_OSTREAM << ' ');
  );

  switch(func) {
    case ColumnMngFunc::INTERNAL_LOAD_ALL: {
      columns_load();
      return;
    }
    case ColumnMngFunc::INTERNAL_ACK_LOAD:
      if(err && m_run) {
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
      if(!err && !m_schemas_set)
        err = Error::MNGR_NOT_ACTIVE;
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
        if(!err) {
          Env::Mngr::schemas()->remove(schema->cid);
          if(m_last_used_cid > schema->cid)
            m_last_used_cid.store(schema->cid - 1);
        }
      }
      break;
    }
    default:
      return;
  }

  auto co_func = ColumnMngFunc(uint8_t(func)-1);

  if(err)
    SWC_LOG_OUT(LOG_DEBUG,
      SWC_LOG_OSTREAM << "COLUMN-ACK func=" << int(co_func);
      Error::print(SWC_LOG_OSTREAM << ' ', err);
      schema->print(SWC_LOG_OSTREAM << ' ');
    );

  ColumnReq::Ptr pending = nullptr;
  {
    Core::MutexSptd::scope lock(m_mutex_actions);
    auto it = m_actions_pending.find(req_id);
    if(it != m_actions_pending.cend()) {
      pending = std::move(it->second);
      m_actions_pending.erase(it);
    }
  }

  if(pending) {
    pending->response(err);
  } else {
    SWC_LOG_OUT(LOG_WARN,
      SWC_LOG_OSTREAM
        << "Missing Pending Req-Ack for func=" << co_func
        << " req_id=" << req_id;
      schema->print(SWC_LOG_OSTREAM << ' ');
    );
  }
}

void MngdColumns::run_actions() {
  int err;
  ColumnReq::Ptr req;
  do {
    err = Error::OK;
    req = std::move(m_actions.front());

    SWC_LOG_OUT(LOG_DEBUG,
      SWC_LOG_OSTREAM << "ColumnMng Request func=" << int(req->function);
      req->schema->print(SWC_LOG_OSTREAM << ' ');
    );

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
                  !Condition::str_eq(req->schema->col_name, schema->col_name))
            err = Error::COLUMN_SCHEMA_NAME_NOT_CORRES;
          else if(schema->cid <= DB::Types::SystemColumn::SYS_CID_END)
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
