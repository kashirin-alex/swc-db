/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/ranger/db/Range.h"

#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/ranger/callbacks/RangeQueryUpdate.h"



namespace SWC { namespace Ranger {

Range::Range(const ColumnCfg* cfg, const rid_t rid)
            : cfg(cfg), rid(rid), 
              type(Types::MetaColumn::get_range_type(cfg->cid)),
              meta_cid(Types::MetaColumn::get_sys_cid(cfg->key_seq, type)),
              blocks(cfg->key_seq), 
              m_path(DB::RangeBase::get_path(cfg->cid, rid)),
              m_interval(cfg->key_seq),
              m_state(State::NOTLOADED), 
              m_compacting(COMPACT_NONE), m_require_compact(false),
              m_inbytes(0) { 
  RangerEnv::res().more_mem_usage(size_of());
}

void Range::init() {
  blocks.init(shared_from_this());
}

Range::~Range() {
  RangerEnv::res().less_mem_usage(size_of());
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
Files::RgrData::Ptr Range::get_last_rgr(int &err) {
  return Files::RgrData::get_rgr(err, DB::RangeBase::get_path_ranger(m_path));
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
  LockAtomic::Unique::scope lock(m_mutex_intval);
  return m_interval.key_begin.empty();
}

bool Range::is_any_end() {
  LockAtomic::Unique::scope lock(m_mutex_intval);
  return m_interval.key_end.empty();
}

uint24_t Range::known_interval_count() {
  LockAtomic::Unique::scope lock(m_mutex_intval);
  return m_interval.key_end.empty()
          ? m_interval.key_begin.count 
          : m_interval.key_end.count;
}

bool Range::align(const DB::Cells::Interval& interval) {
  LockAtomic::Unique::scope lock(m_mutex_intval);
  return m_interval.align(interval);
}
  
bool Range::align(const DB::Cell::Key& key) {
  LockAtomic::Unique::scope lock(m_mutex_intval);
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

void Range::add(Range::ReqAdd* req) {
  if(m_q_adding.push_and_is_1st(req)) { 
    asio::post(*Env::IoCtx::io()->ptr(), 
      [ptr=shared_from_this()](){ ptr->run_add_queue(); }
    );
  }
}

void Range::scan(const ReqScan::Ptr& req) {
  if(compacting_is(COMPACT_APPLYING)) {
    if(!m_q_scans.push_and_is_1st(req))
      return;
    wait(COMPACT_APPLYING);

    blocks.processing_increment();
    int err = is_loaded() ? Error::OK : Error::RS_NOT_LOADED_RANGE;
    ReqScan::Ptr qreq;
    do {
      if(!(qreq = std::move(m_q_scans.front()))->expired()) {
        if(err) {
          qreq->response(err);
        } else {
          blocks.processing_increment();
          asio::post(*Env::IoCtx::io()->ptr(), 
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

void Range::load(const ResponseCallback::Ptr& cb) {
  blocks.processing_increment();

  bool is_loaded;
  {
    std::scoped_lock lock(m_mutex);
    is_loaded = m_state != State::NOTLOADED;
    if(m_state == State::NOTLOADED)
      m_state = State::LOADING;
  }
  int err = RangerEnv::is_shuttingdown() ?
            Error::SERVER_SHUTTING_DOWN : Error::OK;
  if(is_loaded || err)
    return loaded(err, cb);

  SWC_LOGF(LOG_DEBUG, "LOADING RANGE %s", to_string().c_str());

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

void Range::take_ownership(int &err, const ResponseCallback::Ptr& cb) {
  if(err == Error::RS_DELETED_RANGE)
    return loaded(err, cb);

  RangerEnv::rgr_data()->set_rgr(
    err, DB::RangeBase::get_path_ranger(m_path), cfg->file_replication());
  if(err)
    return loaded(err, cb);

  load(err, cb);
}

void Range::on_change(int &err, bool removal, 
                      const DB::Cell::Key* old_key_begin,
                      const client::Query::Update::Cb_t& cb) {
  if(type == Types::Range::MASTER) {
    // update manager-root
    // Mngr::RangeUpdated
    return cb(nullptr);
  }

  std::scoped_lock lock(m_mutex);
    
  auto updater = std::make_shared<client::Query::Update>(cb);
  // RangerEnv::updater();

  updater->columns->create(
    meta_cid, cfg->key_seq, 1, 0, Types::Column::PLAIN);

  DB::Cells::Cell cell;
  auto cid_f(std::to_string(cfg->cid));

  if(removal) {
    cell.flag = DB::Cells::DELETE;
    m_mutex_intval.lock();
    cell.key.copy(m_interval.key_begin);
    m_mutex_intval.unlock();
    cell.key.insert(0, cid_f);
    updater->columns->add(meta_cid, cell);
  } else {

    cell.flag = DB::Cells::INSERT;
    DB::Cell::KeyVec aligned_min;
    DB::Cell::KeyVec aligned_max;
    bool chg;

    m_mutex_intval.lock();
    cell.key.copy(m_interval.key_begin);
    DB::Cell::Key key_end(m_interval.key_end);
    if(type == Types::Range::DATA) { 
      // only DATA until MASTER/META aligned on cells value min/max
      aligned_min.copy(m_interval.aligned_min);
      aligned_max.copy(m_interval.aligned_max);
    }
    chg = old_key_begin && !old_key_begin->equal(m_interval.key_begin);
    m_mutex_intval.unlock();

    cell.key.insert(0, cid_f);
    key_end.insert(0, cid_f);
    if(type == Types::Range::DATA) { 
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
    updater->columns->add(meta_cid, cell);

    if(chg) {
      SWC_ASSERT(!old_key_begin->empty()); 
      // remove begin-any should not happen

      cell.free();
      cell.flag = DB::Cells::DELETE;
      cell.key.copy(*old_key_begin);
      cell.key.insert(0, std::to_string(cfg->cid));
      updater->columns->add(meta_cid, cell);
    }
  }
  updater->commit(meta_cid);
  if(!cb) {
    updater->wait();
    err = updater->result->error();
  }
      
  // INSERT master-range(col-{1,4}), key[cid+m_interval(data(cid)+key)], value[rid]
  // INSERT meta-range(col-{5,8}), key[cid+m_interval(key)], value[rid]
}

void Range::unload(const Callback::RangeUnloaded_t& cb, bool completely) {
  int err = Error::OK;
  {
    std::scoped_lock lock(m_mutex);
    if(m_state == State::DELETED) {
      cb(err);
      return;
    }
    m_state = State::UNLOADING;
  }

  blocks.commitlog.stopping = true;

  wait();
  wait_queue();

  set_state(State::NOTLOADED);

  blocks.unload();

  if(completely) // whether to keep RANGER_FILE
    Env::FsInterface::interface()->remove(
      err, DB::RangeBase::get_path_ranger(m_path));
    
  SWC_LOGF(LOG_INFO, "UNLOADED RANGE cid=%lu rid=%lu err=%d(%s)", 
                      cfg->cid, rid, err, Error::get_text(err));
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

  SWC_LOGF(LOG_INFO, "REMOVED RANGE %s", to_string().c_str());
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
  if(type == Types::Range::DATA)
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
    
  create_folders(err);
  if(err)
    return;
  RangerEnv::rgr_data()->set_rgr(
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
}

std::string Range::to_string() {
  std::string s("(");
  s.append(cfg->to_string());
  s.append(" rid=");
  s.append(std::to_string(rid));
  s.append(" type=");
  s.append(Types::to_string(type));
  s.append(" state=");
  {
    std::shared_lock lock(m_mutex);
    s.append(std::to_string(m_state));
  }
  s.append(" ");
  s.append(blocks.to_string());
  s.append(" prev=");
  s.append(prev_range_end.to_string());
  s.append(" ");
  m_mutex_intval.lock();
  s.append(m_interval.to_string());
  m_mutex_intval.unlock();
  s.append(")");
  return s;
}

void Range::loaded(int &err, const ResponseCallback::Ptr& cb) {
  {
    std::shared_lock lock(m_mutex);
    if(m_state == State::DELETED)
      err = Error::RS_DELETED_RANGE;
  }
  blocks.processing_decrement();
  cb->response(err);
}

void Range::last_rgr_chk(int &err, const ResponseCallback::Ptr& cb) {
  // ranger.data
  auto rgr_data = RangerEnv::rgr_data();
  Files::RgrData::Ptr rs_last = get_last_rgr(err);

  if(rs_last->endpoints.size() && 
     !has_endpoint(rgr_data->endpoints, rs_last->endpoints)){
    SWC_LOGF(LOG_DEBUG, "RANGER-LAST=%s RANGER-NEW=%s", 
              rs_last->to_string().c_str(), rgr_data->to_string().c_str());
                
    Env::Clients::get()->rgr->get(rs_last->endpoints)->put(
      std::make_shared<Protocol::Rgr::Req::RangeUnload>(
        shared_from_this(), cb));
    return;
  }

  take_ownership(err, cb);
}

void Range::load(int &err, const ResponseCallback::Ptr& cb) {
  {
    std::scoped_lock lock(m_mutex);
    if(m_state != State::LOADING) { 
      // state has changed since load request
      err = Error::RS_NOT_LOADED_RANGE;
      return;
    }
  }

  bool is_initial_column_range = false;
  RangeData::load(err, blocks.cellstores);
  if(err) 
    (void)err;
    //err = Error::OK; // ranger-to determine range-removal (+ Notify Mngr)

  else if(blocks.cellstores.empty()) {
    // init 1st cs(for log_cells)
    auto cs = CellStore::create_initial(err, shared_from_this());
    if(!err) {
      blocks.cellstores.add(cs);
      is_initial_column_range = true;
    }
  }
 
  if(!err) {
    blocks.load(err);

    if(!err) {
      blocks.cellstores.get_prev_key_end(0, prev_range_end);

      m_interval.free();
      if(type == Types::Range::DATA)
        blocks.expand_and_align(m_interval);
      else
        blocks.expand(m_interval);

      if(is_initial_column_range) { // or re-reg on load (cfg/req/..)
        RangeData::save(err, blocks.cellstores);
        return on_change(err, false, nullptr,
          [cb, range=shared_from_this()] 
          (const client::Query::Update::Result::Ptr& res) {
            range->loaded_ack(res ? res->error() : Error::OK, cb);
          }
        );
      }
      //else if(cfg->cid > 2) { // meta-recovery, meta need pre-delete >=[cfg->cid]
      //  on_change(err, false);
      //}
    }
  }
  loaded_ack(err, cb);
}

void Range::loaded_ack(int err, const ResponseCallback::Ptr& cb) {
  if(!err)
    set_state(State::LOADED);
      
  if(is_loaded()) {
    SWC_LOGF(LOG_INFO, "LOADED RANGE %s", to_string().c_str());
  } else {
    SWC_LOGF(LOG_WARN, "LOAD RANGE FAILED err=%d(%s) %s", 
              err, Error::get_text(err), to_string().c_str());
  }
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

    auto params = new Protocol::Rgr::Params::RangeQueryUpdateRsp;
    if(req->cb->expired(remain/100000))
      params->err = Error::REQUEST_TIMEOUT;
      
    if(m_state != State::LOADED && m_state != State::UNLOADING)
       params->err = m_state == State::DELETED 
                  ? Error::COLUMN_MARKED_REMOVED : Error::RS_NOT_LOADED_RANGE;

    if(!params->err) while(remain) {
      cell.read(&ptr, &remain);

      if(cell.has_expired(ttl))
        continue;

      {
        LockAtomic::Unique::scope lock(m_mutex_intval);

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

      if(!(cell.control & DB::Cells::HAVE_TIMESTAMP)) {
        cell.set_timestamp(Time::now_ns());
        if(cell.control & DB::Cells::AUTO_TIMESTAMP)
          cell.control ^= DB::Cells::AUTO_TIMESTAMP;
        cell.control |= DB::Cells::REV_IS_TS;
      } else {
        cell.set_revision(Time::now_ns());
      }

      blocks.add_logged(cell);
        
      if(type == Types::Range::DATA) {
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
    }
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
