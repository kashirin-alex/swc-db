/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/ranger/db/Range.h"

#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/ranger/callbacks/RangeQueryUpdate.h"



namespace SWC { namespace Ranger {

Range::Range(const ColumnCfg* cfg, const rid_t rid)
            : cfg(cfg), rid(rid), 
              blocks(cfg->key_seq), 
              m_path(DB::RangeBase::get_path(cfg->cid, rid)),
              m_interval(cfg->key_seq),
              m_state(State::NOTLOADED), 
              m_compacting(COMPACT_NONE), m_require_compact(false),
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
  if(m_q_adding.push_and_is_1st(req))
    Env::IoCtx::post([ptr=shared_from_this()](){ ptr->run_add_queue(); } );
}

void Range::scan(const ReqScan::Ptr& req) {
  if(compacting_is(COMPACT_APPLYING)) {
    if(!m_q_scans.push_and_is_1st(req))
      return;
    wait(COMPACT_APPLYING);

    blocks.processing_increment();
    int err = Error::OK;
    state(err);
    ReqScan::Ptr qreq;
    do {
      if(!(qreq = std::move(m_q_scans.front()))->expired()) {
        if(err) {
          qreq->response(err);
        } else {
          blocks.processing_increment();
          Env::IoCtx::post(
            [qreq, ptr=shared_from_this()]() {
              ptr->blocks.scan(std::move(qreq));
              ptr->blocks.processing_decrement();
            }
          );
        }
      }
    } while(m_q_scans.pop_and_more());
    blocks.processing_decrement();

  } else {
    blocks.processing_increment();
    blocks.scan(std::move(req));
    blocks.processing_decrement();
  }
}

void Range::scan_internal(const ReqScan::Ptr& req) {
  blocks.scan(std::move(req));
}

void Range::create_folders(int& err) {
  Env::FsInterface::interface()->mkdirs(err, get_path(LOG_DIR));
  Env::FsInterface::interface()->mkdirs(err, get_path(LOG_TMP_DIR));
  Env::FsInterface::interface()->mkdirs(err, get_path(CELLSTORES_DIR));
}

void Range::load(const Comm::ResponseCallback::Ptr& cb) {
  blocks.processing_increment();

  bool is_loaded;
  {
    std::scoped_lock lock(m_mutex);
    is_loaded = m_state != State::NOTLOADED;
    if(m_state == State::NOTLOADED)
      m_state = State::LOADING;
  }
  int err = Env::Rgr::is_shuttingdown() ||
            (Env::Rgr::is_not_accepting() &&
             DB::Types::MetaColumn::is_data(cfg->cid))
          ? Error::SERVER_SHUTTING_DOWN : Error::OK;
  if(is_loaded || err)
    return loaded(err, cb);

  SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM << "LOADING RANGE "); );

  if(!Env::FsInterface::interface()->exists(err, get_path(CELLSTORES_DIR))) {
    if(!err)
      create_folders(err);
    if(err)
      return loaded(err, cb);
      
    take_ownership(err, cb);
  } else {
    last_rgr_chk(err, cb);
  }

}

void Range::take_ownership(int &err, const Comm::ResponseCallback::Ptr& cb) {
  if(Env::Rgr::is_shuttingdown() || 
     (Env::Rgr::is_not_accepting() && 
      DB::Types::MetaColumn::is_data(cfg->cid))) {
    return loaded(err = Error::SERVER_SHUTTING_DOWN, cb);
  }

  if(err == Error::RGR_DELETED_RANGE)
    return loaded(err, cb);

  Env::Rgr::rgr_data()->set_rgr(
    err, DB::RangeBase::get_path_ranger(m_path), cfg->file_replication());
  if(err)
    return loaded(err, cb);

  load(err, cb);
}

void Range::on_change(int &err, bool removal, 
                      const DB::Cell::Key* old_key_begin,
                      const client::Query::Update::Cb_t& cb) {
  if(cfg->range_type == DB::Types::Range::MASTER) {
    // update manager-root
    // Mngr::RangeUpdated
    if(cb)
      cb(nullptr);
    return;
  }

  std::scoped_lock lock(m_mutex);
    
  auto updater = std::make_shared<client::Query::Update>(cb);
  // Env::Rgr::updater();

  updater->columns->create(
    cfg->meta_cid, cfg->key_seq, 1, 0, DB::Types::Column::PLAIN);

  DB::Cells::Cell cell;
  auto cid_f(std::to_string(cfg->cid));

  if(removal) {
    cell.flag = DB::Cells::DELETE;
    m_mutex_intval.lock();
    cell.key.copy(m_interval.key_begin);
    m_mutex_intval.unlock();
    cell.key.insert(0, cid_f);
    updater->columns->add(cfg->meta_cid, cell);
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
    updater->columns->add(cfg->meta_cid, cell);

    if(chg) {
      SWC_ASSERT(!old_key_begin->empty()); 
      // remove begin-any should not happen

      cell.free();
      cell.flag = DB::Cells::DELETE;
      cell.key.copy(*old_key_begin);
      cell.key.insert(0, std::to_string(cfg->cid));
      updater->columns->add(cfg->meta_cid, cell);
    }
  }
  updater->commit(cfg->meta_cid);
  if(!cb) {
    updater->wait();
    err = updater->result->error();
  }
      
  // INSERT master-range(col-{1,4}), key[cid+m_interval(data(cid)+key)], value[rid]
  // INSERT meta-range(col-{5,8}), key[cid+m_interval(key)], value[rid]
}

void Range::unload(const Callback::RangeUnloaded_t& cb, bool completely) {
  int err = Error::OK;
  bool done;
  {
    std::scoped_lock lock(m_mutex);
    if(!(done = m_state == State::DELETED))
      m_state = State::UNLOADING;
  }
  if(done) {
    cb(err);
    return;
  }

  blocks.commitlog.stopping = true;

  wait();
  wait_queue();

  blocks.unload();

  if(completely) // whether to keep RANGER_FILE
    Env::FsInterface::interface()->remove(
      err, DB::RangeBase::get_path_ranger(m_path));
    
  SWC_LOGF(LOG_INFO, "UNLOADED RANGE cid=%lu rid=%lu error=%d(%s)", 
                      cfg->cid, rid, err, Error::get_text(err));
  set_state(State::NOTLOADED);
  cb(err);
}
  
void Range::remove(int &err, bool meta) {
  {
    std::scoped_lock lock(m_mutex);
    m_state = State::DELETED;
  }
  if(meta)
    on_change(err, true);

  blocks.commitlog.stopping = true;
  
  wait();
  wait_queue();
  blocks.remove(err);

  Env::FsInterface::interface()->rmdir(err, get_path(""));  

  SWC_LOG_OUT(LOG_INFO, print(SWC_LOG_OSTREAM << "REMOVED RANGE "); );
}

void Range::wait_queue() {
  while(!m_q_adding.empty() || !m_q_scans.empty())
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

bool Range::compacting() {
  std::shared_lock lock(m_mutex);
  return m_compacting != COMPACT_NONE;
}

bool Range::compacting_is(uint8_t state) {
  std::shared_lock lock(m_mutex);
  return m_compacting == state;
}

void Range::compacting(uint8_t state) {
  std::scoped_lock lock(m_mutex);
  m_compacting = state;
  m_cv.notify_all();
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

void Range::apply_new(int &err,
                      CellStore::Writers& w_cellstores, 
                      CommitLog::Fragments::Vec& fragments_old,
                      bool w_update) {
  {
    std::scoped_lock lock(m_mutex);
    blocks.apply_new(err, w_cellstores, fragments_old);
    if(err)
      return;
  }
  if(w_update) {
    expand_and_align(err, true);
    err = Error::OK;
  }
}

void Range::expand_and_align(int &err, bool w_chg_chk) {
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

  if(intval_chg)
    on_change(err, false, w_chg_chk ? &old_key_begin : nullptr);
}
  
void Range::create(int &err, const CellStore::Writers& w_cellstores) {
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

void Range::create(int &err, CellStore::Readers::Vec& mv_css) {
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

void Range::loaded(int &err, const Comm::ResponseCallback::Ptr& cb) {
  {
    std::shared_lock lock(m_mutex);
    if(m_state == State::DELETED)
      err = Error::RGR_DELETED_RANGE;
  }
  blocks.processing_decrement();
  cb->response(err);
}

void Range::last_rgr_chk(int &err, const Comm::ResponseCallback::Ptr& cb) {
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
        shared_from_this(), cb));
    return;
  }

  take_ownership(err, cb);
}

void Range::load(int &err, const Comm::ResponseCallback::Ptr& cb) {
  {
    std::scoped_lock lock(m_mutex);
    if(m_state != State::LOADING) // state has changed since load request
      err = Error::RGR_NOT_LOADED_RANGE;
  }
  if(err) 
    return loaded(err, cb);

  if(Env::Rgr::is_shuttingdown() ||
     (Env::Rgr::is_not_accepting() &&
      DB::Types::MetaColumn::is_data(cfg->cid)))
    return loaded_ack(Error::SERVER_SHUTTING_DOWN, cb);

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
    return loaded_ack(err, cb);

  blocks.cellstores.get_prev_key_end(0, prev_range_end);

  m_interval.free();
  if(cfg->range_type == DB::Types::Range::DATA)
    blocks.expand_and_align(m_interval);
  else
    blocks.expand(m_interval);

  if(is_initial_column_range) {
    RangeData::save(err, blocks.cellstores);
    return on_change(err, false, nullptr,
      [cb, range=shared_from_this()]
      (const client::Query::Update::Result::Ptr& res) {
        range->loaded_ack(res ? res->error() : Error::OK, cb);
      }
    );
  }

  if(cfg->range_type == DB::Types::Range::MASTER)
    return loaded_ack(err, cb);

  auto req = std::make_shared<client::Query::Select>();
  auto intval = DB::Specs::Interval::make_ptr();
  intval->key_eq = true;
  intval->flags.limit = 1;

  /* or select ranges of cid, with rid match in value 
        and on dup. cell of rid, delete earliest */
  intval->key_start.set(m_interval.key_begin, Condition::EQ);
  intval->key_start.insert(0, std::to_string(cfg->cid), Condition::EQ);

  req->specs.columns.push_back(
    DB::Specs::Column::make_ptr(cfg->meta_cid, {intval}));

  req->scan(err);
  if(err)
    return loaded_ack(err = Error::RGR_NOT_LOADED_RANGE, cb);
  req->wait();

  DB::Cells::Result cells; 
  if(!req->result->empty())
    req->result->get_cells(cfg->meta_cid, cells);
          
  if(cells.empty()) {
    SWC_LOG_OUT(LOG_ERROR, 
      SWC_LOG_OSTREAM 
        << "Range MetaData missing cid=" << cfg->cid << " rid=" << rid;
      m_interval.print(SWC_LOG_OSTREAM << "\n\t auto-registering=");
      req->specs.print(SWC_LOG_OSTREAM << "\n\t"); 
    );
    return on_change(err, false, nullptr,
      [cb, range=shared_from_this()]
      (const client::Query::Update::Result::Ptr& res) {
        range->loaded_ack(res ? res->error() : Error::OK, cb);
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

  if(!synced) {
    SWC_LOG_OUT(LOG_ERROR, 
      SWC_LOG_OSTREAM 
        << "Range MetaData NOT-SYNCED cid=" << cfg->cid;
      SWC_LOG_OSTREAM << "\n\t     loaded-rid=" << rid;
      m_interval.print(SWC_LOG_OSTREAM << ' ');
      SWC_LOG_OSTREAM << "\n\t registered-rid=" << _rid;
      interval.print(SWC_LOG_OSTREAM << ' ');
    );
    return on_change(err, false, nullptr,
      [cb, range=shared_from_this()]
      (const client::Query::Update::Result::Ptr& res) {
        range->loaded_ack(res ? res->error() : Error::OK, cb);
      }
    );
  }

  loaded_ack(err, cb);
}

void Range::loaded_ack(int err, const Comm::ResponseCallback::Ptr& cb) {
  if(!err) {
    std::scoped_lock lock(m_mutex);
    if(m_state == State::LOADING)
      m_state = State::LOADED;
  }
  
  SWC_LOG_OUT((is_loaded() ? LOG_INFO : LOG_WARN),
    SWC_LOG_OSTREAM << "LOAD RANGE ";
    if(_log_pr == LOG_INFO)
      SWC_LOG_OSTREAM << "SUCCEED";
    else
      Error::print(SWC_LOG_OSTREAM << "FAILED ", err);
    print(SWC_LOG_OSTREAM << ' ', _log_pr == LOG_INFO);
  );
  loaded(err, cb); // RSP-LOAD-ACK
}

bool Range::wait(uint8_t from_state) {
  bool waited;
  std::unique_lock lock_wait(m_mutex);
  if((waited = (m_compacting >= from_state))) {
    m_cv.wait(
      lock_wait, 
      [from_state, &compacting=m_compacting]() {
        return compacting < from_state;
      }
    );
  }
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
    wait(COMPACT_PREPARING);
    blocks.processing_increment();
  
    req = m_q_adding.front();
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
      return on_change(
        params->err, false, nullptr,
        [req, params, range=shared_from_this()] 
        (const client::Query::Update::Result::Ptr& res) {
          if(!params->err)
            params->err = res->error();
          req->cb->response(*params);
          delete params;
          delete req;
          if(range->m_q_adding.pop_and_more())
            range->run_add_queue();
        }
      );

    req->cb->response(*params);
    delete params;
    delete req;

  } while(m_q_adding.pop_and_more());
    
}


}}


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.cc"
