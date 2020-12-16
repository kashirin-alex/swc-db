/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/ranger/db/Range.h"

#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/ranger/callbacks/RangeQueryUpdate.h"



namespace SWC { namespace Ranger {

Range::Range(const ColumnCfg::Ptr& cfg, const rid_t rid)
            : cfg(cfg), rid(rid), 
              blocks(cfg->key_seq), 
              m_path(DB::RangeBase::get_path(cfg->cid, rid)),
              m_interval(cfg->key_seq),
              m_state(State::NOTLOADED), 
              m_compacting(COMPACT_NONE), m_require_compact(false),
              m_q_run_add(false), m_q_run_scan(false), 
              m_inbytes(0) {
  Env::Rgr::in_process(1);
  Env::Rgr::res().more_mem_usage(size_of());
}

void Range::init() {
  blocks.init(shared_from_this());
}

Range::~Range() {
  Env::Rgr::res().less_mem_usage(size_of());
  Env::Rgr::in_process(-1);
}

size_t Range::size_of() const {
  return sizeof(*this) + sizeof(RangePtr);
}

const std::string Range::get_path(const std::string suff) const {
  std::string s(m_path);
  s.append(suff);
  return s;
}

const std::string Range::get_path_cs(const csid_t csid) const {
  std::string s(m_path);
  s.append(CELLSTORES_DIR);
  s.append("/");
  s.append(std::to_string(csid));
  s.append(".cs");
  return s;
}

const std::string Range::get_path_cs_on(const std::string folder, 
                                        const csid_t csid) const {
  std::string s(m_path);
  s.append(folder);
  s.append("/");
  s.append(std::to_string(csid));
  s.append(".cs");
  return s;
}

SWC_SHOULD_INLINE
Common::Files::RgrData::Ptr Range::get_last_rgr(int &err) {
  return Common::Files::RgrData::get_rgr(
    err, DB::RangeBase::get_path_ranger(m_path));
}

void Range::get_interval(DB::Cells::Interval& interval) {
  m_mutex_intval.lock();
  interval.copy(m_interval); 
  m_mutex_intval.unlock();
}

void Range::get_interval(DB::Cell::Key& key_begin, DB::Cell::Key& key_end) {
  m_mutex_intval.lock();
  key_begin.copy(m_interval.key_begin);
  key_end.copy(m_interval.key_end);
  m_mutex_intval.unlock();
}

void Range::get_key_end(DB::Cell::Key& key) {
  m_mutex_intval.lock();
  key.copy(m_interval.key_end);
  m_mutex_intval.unlock();
}
  
bool Range::is_any_begin() {
  Core::MutexAtomic::scope lock(m_mutex_intval);
  return m_interval.key_begin.empty();
}

bool Range::is_any_end() {
  Core::MutexAtomic::scope lock(m_mutex_intval);
  return m_interval.key_end.empty();
}

uint24_t Range::known_interval_count() {
  Core::MutexAtomic::scope lock(m_mutex_intval);
  return m_interval.key_end.empty()
          ? m_interval.key_begin.count 
          : m_interval.key_end.count;
}

bool Range::align(const DB::Cells::Interval& interval) {
  Core::MutexAtomic::scope lock(m_mutex_intval);
  return m_interval.align(interval);
}
  
bool Range::align(const DB::Cell::Key& key) {
  Core::MutexAtomic::scope lock(m_mutex_intval);
  return m_interval.align(key);
}

void Range::schema_update(bool compact) {
  blocks.schema_update();
  if(compact)
    compact_require(true);
}

void Range::set_state(Range::State new_state) {
  std::scoped_lock lock(m_mutex);
  m_state = new_state;
}

bool Range::is_loaded() {
  std::shared_lock lock(m_mutex);
  return m_state == State::LOADED;
}

bool Range::deleted() { 
  std::shared_lock lock(m_mutex);
  return m_state == State::DELETED;
}

void Range::state(int& err) const {
  if(m_state != State::LOADED) {
    err = m_state == State::DELETED 
      ? Error::COLUMN_MARKED_REMOVED
      : Error::RGR_NOT_LOADED_RANGE;
  }
}

void Range::add(Range::ReqAdd* req) {
  if(m_q_add.push_and_is_1st(req))
    Env::Rgr::post([ptr=shared_from_this()](){ ptr->run_add_queue(); } );
}

void Range::scan(const ReqScan::Ptr& req) {
  {
    std::scoped_lock lock(m_mutex);
    if(m_compacting == COMPACT_APPLYING) {
      m_q_run_scan = true;
      if(req)
        m_q_scan.push_and_is_1st(req);
      return;
    }
    blocks.processing_increment();
 }

 if(req) {
  blocks.scan(std::move(req));

 } else {
    int err;
    ReqScan::Ptr qreq;
    do {
      if(!(qreq = std::move(m_q_scan.front()))->expired()) {
        state(err = Error::OK);
        if(err) {
          qreq->response(err);
        } else {
          blocks.processing_increment();
          Env::Rgr::post(
            [qreq, ptr=shared_from_this()]() {
              ptr->blocks.scan(std::move(qreq));
              ptr->blocks.processing_decrement();
            }
          );
        }
      }
    } while(m_q_scan.pop_and_more());
  }
  blocks.processing_decrement();
}

void Range::scan_internal(const ReqScan::Ptr& req) {
  blocks.scan(std::move(req));
}

void Range::load(const Callback::RangeLoad::Ptr& req) {
  blocks.processing_increment();

  bool need;
  {
    std::scoped_lock lock(m_mutex);
    need = m_state == State::NOTLOADED;
    if(need)
      m_state = State::LOADING;
  }

  int err = Env::Rgr::is_shuttingdown() ||
            (Env::Rgr::is_not_accepting() &&
             DB::Types::MetaColumn::is_data(cfg->cid))
          ? Error::SERVER_SHUTTING_DOWN : Error::OK;
  if(!need || err)
    return loaded(err, req);

  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-STARTED", cfg->cid, rid);

  if(!Env::FsInterface::interface()->exists(err, get_path(CELLSTORES_DIR))) {
    if(!err)
      internal_create_folders(err);
    if(err)
      return loaded(err, req);
      
    internal_take_ownership(err, req);
  } else {
    last_rgr_chk(err, req);
  }
}

void Range::internal_take_ownership(int &err, const Callback::RangeLoad::Ptr& req) {
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-TAKE OWNERSHIP", cfg->cid, rid);

  if(Env::Rgr::is_shuttingdown() || 
     (Env::Rgr::is_not_accepting() && 
      DB::Types::MetaColumn::is_data(cfg->cid))) {
    return loaded(Error::SERVER_SHUTTING_DOWN, req);
  }

  Env::Rgr::rgr_data()->set_rgr(
    err, DB::RangeBase::get_path_ranger(m_path), cfg->file_replication());

  err ? loaded(err, req) : load(err, req);
}

void Range::internal_unload(bool completely) {
  {
    std::scoped_lock lock(m_mutex);
    if(m_state != State::LOADED && !blocks.range)
      return;
    m_state = State::UNLOADING;
  }
  SWC_LOGF(LOG_DEBUG, "UNLOADING RANGE(%lu/%lu)", cfg->cid, rid);

  blocks.commitlog.stopping = true;

  wait();
  wait_queue();

  blocks.unload();

  int err = Error::OK;
  if(completely) // whether to keep RANGER_FILE
    Env::FsInterface::interface()->remove(
      err, DB::RangeBase::get_path_ranger(m_path));

  {
    std::scoped_lock lock(m_mutex);
    m_state = State::NOTLOADED;
  }
  SWC_LOGF(LOG_INFO, "UNLOADED RANGE(%lu/%lu) error=%d(%s)", 
                      cfg->cid, rid, err, Error::get_text(err));
}

void Range::remove(const Callback::ColumnDelete::Ptr& req) {
  {
    std::scoped_lock lock(m_mutex);
    if(m_state == State::DELETED)
      return req->removed(shared_from_this());
    m_state = State::DELETED;
  }
  SWC_LOGF(LOG_DEBUG, "REMOVING RANGE(%lu/%lu)", cfg->cid, rid);

  blocks.commitlog.stopping = true;

  on_change(true, [req, range=shared_from_this()]
    (const client::Query::Update::Result::Ptr&) {
      int err = Error::OK;
      
      range->wait();
      range->wait_queue();
      range->blocks.remove(err);

      Env::FsInterface::interface()->rmdir(err, range->get_path(""));
      req->removed(range);
    }
  );
}

void Range::internal_remove(int& err) {
  {
    std::scoped_lock lock(m_mutex);
    if(m_state == State::DELETED)
      return;
    m_state = State::DELETED;
  }
  SWC_LOGF(LOG_DEBUG, "REMOVING RANGE(%lu/%lu)", cfg->cid, rid);

  blocks.commitlog.stopping = true;
  
  wait();
  wait_queue();
  blocks.remove(err);

  Env::FsInterface::interface()->rmdir(err, get_path(""));  

  SWC_LOG_OUT(LOG_INFO, print(SWC_LOG_OSTREAM << "REMOVED RANGE "); );
}

void Range::wait_queue() {
  while(!m_q_add.empty() || !m_q_scan.empty())
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

bool Range::compacting() {
  std::shared_lock lock(m_mutex);
  return m_compacting != COMPACT_NONE;
}

void Range::compacting(uint8_t state) {
  bool do_q_run_add;
  bool do_q_run_scan;
  {
    std::scoped_lock lock(m_mutex);
    m_compacting = state;
    m_cv.notify_all();
    if(m_compacting == COMPACT_APPLYING) 
      return;

    if((do_q_run_add = m_q_run_add && state < COMPACT_PREPARING))
      m_q_run_add = false;
    if((do_q_run_scan = m_q_run_scan))
      m_q_run_scan = false;
  }
  if(do_q_run_add)
    Env::Rgr::post([ptr=shared_from_this()](){ ptr->run_add_queue(); });
  if(do_q_run_scan)
    Env::Rgr::post([ptr=shared_from_this()](){ ptr->scan(nullptr); });
}

bool Range::compacting_ifnot_applying(uint8_t state) {
  bool do_q_run_add;
  bool do_q_run_scan;
  {
    std::scoped_lock lock(m_mutex);
    if(m_compacting == COMPACT_APPLYING) 
      return false;
    m_compacting = state;
    m_cv.notify_all();

    if((do_q_run_add = m_q_run_add && state < COMPACT_PREPARING))
      m_q_run_add = false;
    if((do_q_run_scan = m_q_run_scan))
      m_q_run_scan = false;
  }
  if(do_q_run_add)
    Env::Rgr::post([ptr=shared_from_this()](){ ptr->run_add_queue(); });
  if(do_q_run_scan)
    Env::Rgr::post([ptr=shared_from_this()](){ ptr->scan(nullptr); });
  return true;
}

bool Range::compact_possible(bool minor) {
  std::scoped_lock lock(m_mutex);
  if(m_state != State::LOADED || m_compacting != COMPACT_NONE ||
     (!minor && !m_require_compact && blocks.processing()))
    return false;
  m_compacting = COMPACT_CHECKING;
  return true;
}

void Range::compact_require(bool require) {
  std::scoped_lock lock(m_mutex);
  m_require_compact = require;
}

bool Range::compact_required() {
  std::shared_lock lock(m_mutex);
  return m_require_compact;
}

void Range::on_change(bool removal,
                      const client::Query::Update::Cb_t& cb, 
                      const DB::Cell::Key* old_key_begin) {
  if(cfg->range_type == DB::Types::Range::MASTER) {
    // update manager-root
    // Mngr::RangeUpdated
    cb(nullptr);
    return;
  }

  std::scoped_lock lock(m_mutex);
    
  auto updater = std::make_shared<client::Query::Update>(cb, Env::Rgr::io());
  // Env::Rgr::updater(); require an updater with cb on cell-base

  updater->columns->create(
    cfg->meta_cid, cfg->key_seq, 1, 0, DB::Types::Column::PLAIN);
  auto col = updater->columns->get_col(cfg->meta_cid);

  DB::Cells::Cell cell;
  auto cid_f(std::to_string(cfg->cid));

  if(removal) {
    cell.flag = DB::Cells::DELETE;
    m_mutex_intval.lock();
    cell.key.copy(m_interval.key_begin);
    m_mutex_intval.unlock();
    cell.key.insert(0, cid_f);
    col->add(cell);

  } else {
    cell.flag = DB::Cells::INSERT;
    DB::Cell::KeyVec aligned_min;
    DB::Cell::KeyVec aligned_max;
    bool chg;

    m_mutex_intval.lock();
    cell.key.copy(m_interval.key_begin);
    DB::Cell::Key key_end(m_interval.key_end);
    if(cfg->range_type == DB::Types::Range::DATA) { 
      // only DATA until MASTER/META aligned on cells value min/max
      aligned_min.copy(m_interval.aligned_min);
      aligned_max.copy(m_interval.aligned_max);
    }
    chg = old_key_begin && !old_key_begin->equal(m_interval.key_begin);
    m_mutex_intval.unlock();

    cell.key.insert(0, cid_f);
    key_end.insert(0, cid_f);
    if(cfg->range_type == DB::Types::Range::DATA) { 
      aligned_min.insert(0, cid_f);
      aligned_max.insert(0, cid_f);
    }

    cell.own = true;
    cell.vlen = key_end.encoded_length() 
                + Serialization::encoded_length_vi64(rid)
                + aligned_min.encoded_length()
                + aligned_max.encoded_length() ;

    cell.value = new uint8_t[cell.vlen];
    uint8_t * ptr = cell.value;
    key_end.encode(&ptr);
    Serialization::encode_vi64(&ptr, rid);
    aligned_min.encode(&ptr);
    aligned_max.encode(&ptr);

    cell.set_time_order_desc(true);
    col->add(cell);

    if(chg) {
      SWC_ASSERT(!old_key_begin->empty()); 
      // remove begin-any should not happen

      cell.free();
      cell.flag = DB::Cells::DELETE;
      cell.key.copy(*old_key_begin);
      cell.key.insert(0, std::to_string(cfg->cid));
      col->add(cell);
    }
  }
  updater->commit(col);
      
  // INSERT master-range(col-{1,4}), key[cid+m_interval(data(cid)+key)], value[rid]
  // INSERT meta-range(col-{5,8}), key[cid+m_interval(key)], value[rid]
}

void Range::apply_new(int &err,
                      CellStore::Writers& w_cellstores, 
                      CommitLog::Fragments::Vec& fragments_old,
                      const client::Query::Update::Cb_t& cb) {
  {
    std::scoped_lock lock(m_mutex);
    blocks.apply_new(err, w_cellstores, fragments_old);
    if(err)
      return;
  }
  if(cb)
    expand_and_align(true, cb);
}

void Range::expand_and_align(bool w_chg_chk,
                             const client::Query::Update::Cb_t& cb) {
  DB::Cell::Key     old_key_begin;
  DB::Cell::Key     key_end;
  DB::Cell::KeyVec  aligned_min;
  DB::Cell::KeyVec  aligned_max; 

  m_mutex_intval.lock();
  if(w_chg_chk) {
    old_key_begin.copy(m_interval.key_begin);
    key_end.copy(m_interval.key_end);
    aligned_min.copy(m_interval.aligned_min);
    aligned_max.copy(m_interval.aligned_max);
  }

  m_interval.free();
  if(cfg->range_type == DB::Types::Range::DATA)
    blocks.expand_and_align(m_interval);
  else
    blocks.expand(m_interval);

  bool intval_chg = !w_chg_chk || 
                    !m_interval.key_begin.equal(old_key_begin) ||
                    !m_interval.key_end.equal(key_end) ||
                    !m_interval.aligned_min.equal(aligned_min) ||
                    !m_interval.aligned_max.equal(aligned_max);
  m_mutex_intval.unlock();

  intval_chg 
    ? on_change(false, cb, w_chg_chk ? &old_key_begin : nullptr)
    : cb(nullptr);
}

void Range::internal_create_folders(int& err) {
  Env::FsInterface::interface()->mkdirs(err, get_path(LOG_DIR));
  Env::FsInterface::interface()->mkdirs(err, get_path(LOG_TMP_DIR));
  Env::FsInterface::interface()->mkdirs(err, get_path(CELLSTORES_DIR));
}

void Range::internal_create(int &err, const CellStore::Writers& w_cellstores) {
  Env::Rgr::rgr_data()->set_rgr(
    err, DB::RangeBase::get_path_ranger(m_path), cfg->file_replication());
  if(err)
    return;

  auto fs = Env::FsInterface::interface();
  for(auto& cs : w_cellstores) {
    fs->rename(
      err, 
      cs->smartfd->filepath(), 
      get_path_cs(cs->csid)
    );
    if(err)
      return;
        
    blocks.cellstores.add(
      CellStore::Read::make(
        err, cs->csid, shared_from_this(), cs->interval, true)
    );
    if(err)
      return;
  }

  RangeData::save(err, blocks.cellstores);
  fs->remove(err, DB::RangeBase::get_path_ranger(m_path));
  err = Error::OK;
}

void Range::internal_create(int &err, CellStore::Readers::Vec& mv_css) {
  Env::Rgr::rgr_data()->set_rgr(
    err, DB::RangeBase::get_path_ranger(m_path), cfg->file_replication());
  if(err)
    return;

  blocks.cellstores.move_from(err, mv_css);
  if(mv_css.empty())
    err = Error::CANCELLED;
  if(!err) {
    RangeData::save(err, blocks.cellstores);
    Env::FsInterface::interface()->remove(
      err = Error::OK, DB::RangeBase::get_path_ranger(m_path));
    err = Error::OK;
  }
}

void Range::print(std::ostream& out, bool minimal) {
  cfg->print(out << '(');
  out << " rid=" << rid << " state=";
  {
    std::shared_lock lock(m_mutex);
    out << m_state;
  }
  blocks.print(out << ' ', minimal);
  prev_range_end.print(out << " prev=");
  m_mutex_intval.lock();
  m_interval.print(out << ' ');
  m_mutex_intval.unlock();
  out << ')'; 
}

void Range::last_rgr_chk(int &err, const Callback::RangeLoad::Ptr& req) {
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-CHECK LAST RGR", cfg->cid, rid);

  // ranger.data
  auto rgr_data = Env::Rgr::rgr_data();
  Common::Files::RgrData::Ptr rs_last = get_last_rgr(err);

  if(rs_last->endpoints.size() && 
     !Comm::has_endpoint(rgr_data->endpoints, rs_last->endpoints)) {
    SWC_LOG_OUT(LOG_DEBUG,
      rs_last->print(SWC_LOG_OSTREAM << "RANGER LAST=");
      rgr_data->print(SWC_LOG_OSTREAM << " NEW=");
    );

    Env::Clients::get()->rgr->get(rs_last->endpoints)->put(
      std::make_shared<Comm::Protocol::Rgr::Req::RangeUnload>(
        shared_from_this(), req)
    );

  } else {
    internal_take_ownership(err, req);
  }
}

void Range::load(int &err, const Callback::RangeLoad::Ptr& req) {
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-DATA", cfg->cid, rid);

  if(Env::Rgr::is_shuttingdown() ||
     (Env::Rgr::is_not_accepting() &&
      DB::Types::MetaColumn::is_data(cfg->cid)))
    return loaded(Error::SERVER_SHUTTING_DOWN, req);

  bool is_initial_column_range = false;
  RangeData::load(err, blocks.cellstores);
  if(err) {
    (void)err;
    //err = Error::OK; // ranger-to determine range-removal (+ Notify Mngr)

  } else if(blocks.cellstores.empty()) {
    // init 1st cs(for log_cells)
    auto cs = CellStore::create_initial(err, shared_from_this());
    if(!err) {
      blocks.cellstores.add(cs);
      is_initial_column_range = true;
    }
  }
 
  if(!err)
    blocks.load(err);

  if(err)
    return loaded(err, req);

  blocks.cellstores.get_prev_key_end(0, prev_range_end);

  m_interval.free();
  if(cfg->range_type == DB::Types::Range::DATA)
    blocks.expand_and_align(m_interval);
  else
    blocks.expand(m_interval);

  if(is_initial_column_range) {
    RangeData::save(err, blocks.cellstores);
    return on_change(false, [req, range=shared_from_this()]
      (const client::Query::Update::Result::Ptr& res) {
        range->loaded(res ? res->error() : Error::OK, req);
      }
    );
  }

  if(cfg->range_type == DB::Types::Range::MASTER)
    return loaded(err, req);

  auto col_spec = DB::Specs::Column::make_ptr(
    cfg->meta_cid, {DB::Specs::Interval::make_ptr()});
  auto& intval = col_spec->intervals.front();
  intval->set_opt__key_equal();
  intval->flags.limit = 1;

  /* or select ranges of cid, with rid match in value 
        and on dup. cell of rid, delete earliest */
  auto& key_intval = intval->key_intervals.add();
  key_intval->start.set(m_interval.key_begin, Condition::EQ);
  key_intval->start.insert(0, std::to_string(cfg->cid), Condition::EQ);

  auto selector = std::make_shared<client::Query::Select>(
    [req, col_spec, range=shared_from_this()] 
    (const client::Query::Select::Result::Ptr& res) {
      range->check_meta(req, col_spec, res);
    },
    false,
    Env::Rgr::io()
  );
  selector->specs.columns.push_back(col_spec);

  if(Env::Rgr::is_shuttingdown() ||
     (Env::Rgr::is_not_accepting() &&
      DB::Types::MetaColumn::is_data(cfg->cid)))
    return loaded(Error::SERVER_SHUTTING_DOWN, req);

  selector->scan(err);
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-SELECTOR err=%d(%s)", 
                      cfg->cid, rid, err, Error::get_text(err));
  if(err)
    return loaded(Error::RGR_NOT_LOADED_RANGE, req);
}

void Range::check_meta(const Callback::RangeLoad::Ptr& req,
                       const DB::Specs::Column::Ptr& col_spec,
                       const client::Query::Select::Result::Ptr& result) {
  DB::Cells::Result cells; 
  int err = result->err;
  if(!err) {
    auto col = result->get_columnn(cfg->meta_cid);
    if(!(err = col->error()) && !col->empty())
      col->get_cells(cells);
  }
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-CHECK META err=%d(%s)",
                      cfg->cid, rid, err, Error::get_text(err));
  if(err)
    return loaded(Error::RGR_NOT_LOADED_RANGE, req);

  if(Env::Rgr::is_shuttingdown() ||
      (Env::Rgr::is_not_accepting() &&
       DB::Types::MetaColumn::is_data(cfg->cid)))
    return loaded(Error::SERVER_SHUTTING_DOWN, req);

  if(cells.empty()) {
    SWC_LOG_OUT(LOG_ERROR, 
      SWC_LOG_OSTREAM 
        << "Range MetaData missing cid=" << cfg->cid << " rid=" << rid;
      m_interval.print(SWC_LOG_OSTREAM << "\n\t auto-registering=");
      col_spec->print(SWC_LOG_OSTREAM << "\n\t"); 
    );
    return on_change(false, [req, range=shared_from_this()]
      (const client::Query::Update::Result::Ptr& res) {
        range->loaded(res ? res->error() : Error::OK, req);
      }
    );
  }
  
  DB::Cells::Interval interval(cfg->key_seq);
  interval.was_set = true;
  /* Range MetaData does not include timestamp
      for the comparison use current interval ts */
  interval.ts_earliest.copy(m_interval.ts_earliest);
  interval.ts_latest.copy(m_interval.ts_latest);

  rid_t _rid = 0;
  bool synced = false;
  try {
    auto& cell = *cells[0];
    interval.key_begin.copy(cell.key);
    interval.key_begin.remove(0);
    size_t remain = cell.vlen;
    const uint8_t* ptr = cell.value;
    interval.key_end.decode(&ptr, &remain, false);
    interval.key_end.remove(0);
    _rid = Serialization::decode_vi64(&ptr, &remain);

    interval.aligned_min.decode(&ptr, &remain);
    interval.aligned_min.remove(0);
    interval.aligned_max.decode(&ptr, &remain);
    interval.aligned_max.remove(0);

    synced = !remain && rid == _rid && m_interval.equal(interval);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

  if(Env::Rgr::is_shuttingdown() ||
      (Env::Rgr::is_not_accepting() &&
       DB::Types::MetaColumn::is_data(cfg->cid)))
    return loaded(Error::SERVER_SHUTTING_DOWN, req);

  if(!synced) {
    SWC_LOG_OUT(LOG_ERROR, 
      SWC_LOG_OSTREAM 
        << "Range MetaData NOT-SYNCED cid=" << cfg->cid;
      SWC_LOG_OSTREAM << "\n\t     loaded-rid=" << rid;
      m_interval.print(SWC_LOG_OSTREAM << ' ');
      SWC_LOG_OSTREAM << "\n\t registered-rid=" << _rid;
      interval.print(SWC_LOG_OSTREAM << ' ');
    );
    return on_change(false, [req, range=shared_from_this()]
      (const client::Query::Update::Result::Ptr& res) {
        range->loaded(res ? res->error() : Error::OK, req);
      }
    );
  }

  loaded(err, req);
}

void Range::loaded(int err, const Callback::RangeLoad::Ptr& req) {
  bool tried;
  {
    std::scoped_lock lock(m_mutex);
    tried = m_state == State::LOADING;
    if(tried)
      m_state = err ? State::NOTLOADED : State::LOADED;
  }

  SWC_LOG_OUT((is_loaded() ? LOG_INFO : LOG_WARN),
    if(!tried)
      SWC_LOG_OSTREAM << "CHECK ";
    SWC_LOG_OSTREAM << "LOAD RANGE ";
    if(_log_pr == LOG_INFO)
      SWC_LOG_OSTREAM << "SUCCEED";
    else
      Error::print(SWC_LOG_OSTREAM << "FAILED ", err);
    print(SWC_LOG_OSTREAM << ' ', _log_pr == LOG_INFO &&
                                  err != Error::SERVER_SHUTTING_DOWN &&
                                  err != Error::RGR_NOT_LOADED_RANGE);
  );

  blocks.processing_decrement();
  req->loaded(err);
}

bool Range::wait(uint8_t from_state, bool incr) {
  bool waited;
  std::unique_lock lock_wait(m_mutex);
  if((waited = (m_compacting >= from_state))) {
    m_cv.wait(
      lock_wait, 
      [this, from_state]() {
        return m_compacting < from_state;
      }
    );
  }
  if(incr)
    blocks.processing_increment();
  return waited;
}

void Range::run_add_queue() {
  ReqAdd* req;

  DB::Cells::Cell cell;
  const uint8_t* ptr;
  size_t remain; 
  bool intval_chg;
  uint64_t ttl = cfg->cell_ttl();

  do {
    {
      std::scoped_lock lock(m_mutex);
      if(m_compacting >= COMPACT_PREPARING) {
        m_q_run_add = true;
        return;
      }
      blocks.processing_increment();
    }
  
    req = m_q_add.front();
    ptr = req->input.base;
    m_inbytes += remain = req->input.size;
    intval_chg = false;

    auto params = new Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp(
      Error::OK);

    if(req->cb->expired(remain/100000))
      params->err = Error::REQUEST_TIMEOUT;
      
    if(m_state != State::LOADED && m_state != State::UNLOADING)
       params->err = m_state == State::DELETED 
        ? Error::COLUMN_MARKED_REMOVED : Error::RGR_NOT_LOADED_RANGE;

    if(!params->err) { try { while(remain) {
      
      cell.read(&ptr, &remain);

      {
        Core::MutexAtomic::scope lock(m_mutex_intval);

        if(!m_interval.key_end.empty() && 
            DB::KeySeq::compare(cfg->key_seq, m_interval.key_end, cell.key)
             == Condition::GT) {
          if(params->range_end.empty()) {
            params->range_end.copy(m_interval.key_end);
            params->err = Error::RANGE_BAD_INTERVAL;
          }
          continue;
        }
      }

      if(!prev_range_end.empty() && 
          DB::KeySeq::compare(cfg->key_seq, prev_range_end, cell.key)
           != Condition::GT) {
        if(params->range_prev_end.empty()) {
          params->range_prev_end.copy(prev_range_end);
          params->err = Error::RANGE_BAD_INTERVAL;
        }
        continue;
      }

      ++params->cells_added;
      if(cell.has_expired(ttl))
        continue;

      if(!(cell.control & DB::Cells::HAVE_TIMESTAMP)) {
        cell.set_timestamp(Time::now_ns());
        if(cell.control & DB::Cells::AUTO_TIMESTAMP)
          cell.control ^= DB::Cells::AUTO_TIMESTAMP;
        cell.control |= DB::Cells::REV_IS_TS;
      } else {
        cell.set_revision(Time::now_ns());
      }

      blocks.add_logged(cell);
        
      if(cfg->range_type == DB::Types::Range::DATA) {
        if(align(cell.key))
          intval_chg = true;
      } else {
        /* MASTER/META need aligned interval 
             over cells value +plus (cs+logs) at compact
        if(cell.flag == DB::Cells::INSERT) {
          size_t remain = cell.vlen;
          const uint8_t * ptr = cell.value;
          DB::Cell::Key key_end;
          key_end.decode(&ptr, &remain);
          Serialization::decode_vi64(&ptr, &remain);//rid
          DB::Cell::Key aligned_min;
          aligned_min.decode(&ptr, &remain);
          DB::Cell::Key aligned_max;
          aligned_max.decode(&ptr, &remain);
          intval_chg = align(aligned_min);
          if(align(aligned_max))
            intval_chg = true;
        }
        */
      }
    } } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      params->err = Error::RANGE_BAD_CELLS_INPUT;
    } }


    blocks.processing_decrement();

    if(intval_chg)
      return on_change(false, 
        [req, params, range=shared_from_this()]
        (const client::Query::Update::Result::Ptr& res) {
          if(!params->err)
            params->err = res->error();
          req->cb->response(*params);
          delete params;
          delete req;
          if(range->m_q_add.pop_and_more())
            range->run_add_queue();
        }
      );

    req->cb->response(*params);
    delete params;
    delete req;

  } while(m_q_add.pop_and_more());
    
}


}}


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.cc"
