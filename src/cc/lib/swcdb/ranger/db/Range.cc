/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/ranger/db/Range.h"

#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/ranger/callbacks/RangeQueryUpdate.h"



namespace SWC { namespace Ranger {

Range::Range(const ColumnCfg* cfg, const int64_t rid)
            : cfg(cfg), rid(rid), 
              m_path(DB::RangeBase::get_path(cfg->cid, rid)),
              m_state(State::NOTLOADED), 
              type(cfg->cid == 1 ? Types::Range::MASTER 
                  :(cfg->cid == 2 ? Types::Range::META : Types::Range::DATA)),
              m_compacting(COMPACT_NONE), m_require_compact(false) {
}

void Range::init() {
  blocks.init(shared_from_this());
}

Range::~Range() { }
  
const std::string Range::get_path(const std::string suff) const {
  std::string s(m_path);
  s.append(suff);
  return s;
}

const std::string Range::get_path_cs(const int64_t cs_id) const {
  std::string s(m_path);
  s.append(CELLSTORES_DIR);
  s.append("/");
  s.append(std::to_string(cs_id));
  s.append(".cs");
  return s;
}

const std::string Range::get_path_cs_on(const std::string folder, 
                                        const int64_t cs_id) const {
  std::string s(m_path);
  s.append(folder);
  s.append("/");
  s.append(std::to_string(cs_id));
  s.append(".cs");
  return s;
}

Files::RgrData::Ptr Range::get_last_rgr(int &err) {
  return Files::RgrData::get_rgr(err, DB::RangeBase::get_path_ranger(m_path));
}

void Range::get_interval(DB::Cells::Interval& interval) {
  std::shared_lock lock(m_mutex);
  interval.copy(m_interval);
}

void Range::get_interval(DB::Cell::Key& key_begin, DB::Cell::Key& key_end) {
  std::shared_lock lock(m_mutex);
  key_begin.copy(m_interval.key_begin);
  key_end.copy(m_interval.key_end);
}
  
const bool Range::is_any_begin() {
  std::shared_lock lock(m_mutex);
  return m_interval.key_begin.empty();
}

const bool Range::is_any_end() {
  std::shared_lock lock(m_mutex);
  return m_interval.key_end.empty();
}

void Range::get_prev_key_end(DB::Cell::Key& key) {
  std::shared_lock lock(m_mutex);
  key.copy(m_prev_key_end);
}
  
void Range::set_prev_key_end(const DB::Cell::Key& key) {
  std::scoped_lock lock(m_mutex);
  m_prev_key_end.copy(key);
}

const bool Range::align(const DB::Cells::Interval& interval) {
  std::scoped_lock lock(m_mutex);
  return m_interval.align(interval);
}
  
const bool Range::align(const DB::Cell::Key& key) {
  std::scoped_lock lock(m_mutex);
  return key.align(m_interval.aligned_min, m_interval.aligned_max);
}

void Range::schema_update(bool compact) {
  blocks.schema_update();
  if(compact)
    compact_require(compact);
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
  std::scoped_lock lock(m_mutex);
  m_q_adding.push(req);
    
  if(m_q_adding.size() == 1) { 
    asio::post(*Env::IoCtx::io()->ptr(), 
      [ptr=shared_from_this()](){ ptr->run_add_queue(); }
    );
  }
}

void Range::scan(ReqScan::Ptr req) {
  if(wait(COMPACT_APPLYING) && !is_loaded()) {
    int err = Error::RS_NOT_LOADED_RANGE;
    req->response(err);
    return;
  }
  blocks.scan(req);
}

void Range::scan_internal(ReqScan::Ptr req) {
  blocks.scan(req);
}

void Range::create_folders(int& err) {
  Env::FsInterface::interface()->mkdirs(err, get_path(LOG_DIR));
  Env::FsInterface::interface()->mkdirs(err, get_path(CELLSTORES_DIR));
}

void Range::load(ResponseCallback::Ptr cb) {
  bool is_loaded;
  {
    std::scoped_lock lock(m_mutex);
    is_loaded = m_state != State::NOTLOADED;
    if(m_state == State::NOTLOADED)
      m_state = State::LOADING;
  }
  int err = RangerEnv::is_shuttingdown() ?
            Error::SERVER_SHUTTING_DOWN : Error::OK;
  if(is_loaded || err != Error::OK)
    return loaded(err, cb);

  SWC_LOGF(LOG_DEBUG, "LOADING RANGE %s", to_string().c_str());

  if(!Env::FsInterface::interface()->exists(err, get_path(CELLSTORES_DIR))) {
    if(err != Error::OK)
      return loaded(err, cb);
    create_folders(err);
    if(err != Error::OK)
      return loaded(err, cb);
      
    take_ownership(err, cb);
  } else {
    last_rgr_chk(err, cb);
  }

}

void Range::take_ownership(int &err, ResponseCallback::Ptr cb) {
  if(err == Error::RS_DELETED_RANGE)
    return loaded(err, cb);

  RangerEnv::rgr_data()->set_rgr(
    err, DB::RangeBase::get_path_ranger(m_path), cfg->file_replication());
  if(err != Error::OK)
    return loaded(err, cb);

  load(err, cb);
}

void Range::on_change(int &err, bool removal, 
                      const DB::Cell::Key* old_key_begin) {
  std::scoped_lock lock(m_mutex);
    
  if(type == Types::Range::MASTER) {
    // update manager-root
    // Mngr::RangeUpdated
    return;
  }

  auto updater = std::make_shared<client::Query::Update>();
  // RangerEnv::updater();
  uint8_t cid_typ = type == Types::Range::DATA ? 2 : 1;

  updater->columns->create(cid_typ, 1, 0, Types::Column::PLAIN);

  DB::Cells::Cell cell;
  cell.key.copy(m_interval.key_begin);
  cell.key.insert(0, std::to_string(cfg->cid));

  if(removal) {
    cell.flag = DB::Cells::DELETE;
    updater->columns->add(cid_typ, cell);
  } else {

    cell.flag = DB::Cells::INSERT;
    DB::Cell::Key key_end(m_interval.key_end);
    key_end.insert(0, std::to_string(cfg->cid));

    DB::Cell::KeyVec aligned_min;
    DB::Cell::KeyVec aligned_max;
    if(type == Types::Range::DATA) { 
      // only DATA until MASTER/META aligned on cells value min/max
      aligned_min.copy(m_interval.aligned_min);
      aligned_min.insert(0, std::to_string(cfg->cid));
      aligned_max.copy(m_interval.aligned_max);
      aligned_max.insert(0, std::to_string(cfg->cid));
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
    updater->columns->add(cid_typ, cell);

    if(old_key_begin && !old_key_begin->equal(m_interval.key_begin)) {
      SWC_ASSERT(!old_key_begin->empty()); 
      // remove begin-any should not happen

      cell.free();
      cell.flag = DB::Cells::DELETE;
      cell.key.copy(*old_key_begin);
      cell.key.insert(0, std::to_string(cfg->cid));
      updater->columns->add(cid_typ, cell);
    }
  }
  updater->commit(cid_typ);
  updater->wait();
  err = updater->result->error();
      
  // INSERT master-range(col-1), key[cid+m_interval(data(cid)+key)], value[rid]
  // INSERT meta-range(col-2), key[cid+m_interval(key)], value[rid]
}

void Range::unload(Callback::RangeUnloaded_t cb, bool completely) {
  int err = Error::OK;
  {
    std::scoped_lock lock(m_mutex);
    if(m_state == State::DELETED) {
      cb(err);
      return;
    }
    m_state = State::UNLOADING;
  }

  wait();
  wait_queue();

  set_state(State::NOTLOADED);

  blocks.unload();

  if(completely) // whether to keep RANGER_FILE
    Env::FsInterface::interface()->remove(
      err, DB::RangeBase::get_path_ranger(m_path));
    
  SWC_LOGF(LOG_INFO, "UNLOADED RANGE cid=%d rid=%d err=%d(%s)", 
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

  wait();
  wait_queue();
  blocks.remove(err);

  Env::FsInterface::interface()->rmdir(err, get_path(""));  

  SWC_LOGF(LOG_INFO, "REMOVED RANGE %s", to_string().c_str());
}

void Range::wait_queue() {
  for(;;) {
    {
      std::shared_lock lock(m_mutex);
      if(m_q_adding.empty())
        break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

const bool Range::compacting() {
  std::shared_lock lock(m_mutex);
  return m_compacting != COMPACT_NONE;
}

void Range::compacting(uint8_t state) {
  {
    std::scoped_lock lock(m_mutex);
    m_compacting = state;
  }
  m_cv.notify_all();
}
  
const bool Range::compact_possible() {
  std::scoped_lock lock(m_mutex);
  if(m_state != State::LOADED || m_compacting != COMPACT_NONE
      || (!m_require_compact && blocks.processing()))
    return false;
  m_compacting = COMPACT_CHECKING;
  return true;
}

void Range::compact_require(bool require) {
  std::scoped_lock lock(m_mutex);
  m_require_compact = require;
}

const bool Range::compact_required() {
  std::shared_lock lock(m_mutex);
  return m_require_compact;
}

void Range::apply_new(int &err,
                      CellStore::Writers& w_cellstores, 
                      std::vector<CommitLog::Fragment::Ptr>& fragments_old) {
  bool intval_chg;
  DB::Cell::Key old_key_begin;
  {
    std::scoped_lock lock(m_mutex);
    blocks.apply_new(err, w_cellstores, fragments_old);
    if(err)
      return;

    old_key_begin.copy(m_interval.key_begin);
    auto old_key_end(m_interval.key_end);
    auto old_aligned_min(m_interval.aligned_min);
    auto old_aligned_max(m_interval.aligned_max);

    m_interval.free();
    if(type == Types::Range::DATA)
      blocks.expand_and_align(m_interval);
    else
      blocks.expand(m_interval);

    intval_chg = !m_interval.key_begin.equal(old_key_begin) ||
                 !m_interval.key_end.equal(old_key_end) ||
                 !m_interval.aligned_min.equal(old_aligned_min) ||
                 !m_interval.aligned_max.equal(old_aligned_max);
  }
  if(intval_chg)
    on_change(err, false, &old_key_begin);
  err = Error::OK;
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
      get_path_cs(cs->id)
    );
    if(err)
      return;
        
    blocks.cellstores.add(
      CellStore::Read::make(
        err, cs->id, shared_from_this(), cs->interval)
    );
    if(err)
      return;
  }

  m_interval.free();
  if(type == Types::Range::DATA)
    blocks.expand_and_align(m_interval);
  else
    blocks.expand(m_interval);

  RangeData::save(err, blocks.cellstores);
  on_change(err, false);
    
  fs->remove(err, DB::RangeBase::get_path_ranger(m_path));
}

const std::string Range::to_string() {
  std::shared_lock lock(m_mutex);
    
  std::string s("(");
  s.append(cfg->to_string());
  s.append(" rid=");
  s.append(std::to_string(rid));
  s.append(" prev=");
  s.append(m_prev_key_end.to_string());
  s.append(" ");
  s.append(m_interval.to_string());
  s.append(" state=");
  s.append(std::to_string(m_state));
  s.append(" type=");
  s.append(Types::to_string(type));
  s.append(" ");
  s.append(blocks.to_string());
  s.append(")");
  return s;
}

void Range::loaded(int &err, ResponseCallback::Ptr cb) {
  {
    std::shared_lock lock(m_mutex);
    if(m_state == State::DELETED)
      err = Error::RS_DELETED_RANGE;
  }
  cb->response(err);
}

void Range::last_rgr_chk(int &err, ResponseCallback::Ptr cb) {
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

void Range::load(int &err, ResponseCallback::Ptr cb) {
  bool is_initial_column_range = false;
  RangeData::load(err, blocks.cellstores);
  if(err) 
    (void)err;
    //err = Error::OK; // ranger-to determine range-removal (+ Notify Mngr)

  else if(blocks.cellstores.empty()) {
    // init 1st cs(for log_cells)
    auto cs = CellStore::create_init_read(
      err, cfg->blk_enc, shared_from_this());
    if(!err) {
      blocks.cellstores.add(cs);
      is_initial_column_range = true;
    }
  }
 
  if(!err) {
    blocks.load(err);
    if(!err) {
      m_interval.free();
      if(type == Types::Range::DATA)
        blocks.expand_and_align(m_interval);
      else
        blocks.expand(m_interval);

      if(is_initial_column_range) { // or re-reg on load (cfg/req/..)
        RangeData::save(err, blocks.cellstores);
        on_change(err, false);
      }
      //else if(cfg->cid > 2) { // meta-recovery, meta need pre-delete >=[cfg->cid]
      //  on_change(err, false);
      //}
    }
  }
  
  if(!err) 
    set_state(State::LOADED);
      
  loaded(err, cb);   // RSP-LOAD-ACK

  if(is_loaded()) {
    SWC_LOGF(LOG_INFO, "LOADED RANGE %s", to_string().c_str());
  } else 
    SWC_LOGF(LOG_WARN, "LOAD RANGE FAILED err=%d(%s) %s", 
              err, Error::get_text(err), to_string().c_str());
}

const bool Range::wait(uint8_t from_state) {
  bool waited;
  std::unique_lock lock_wait(m_mutex);
  if(waited = (m_compacting >= from_state)) {
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

  int err;
  DB::Cells::Cell cell;
  const uint8_t* ptr;
  size_t remain; 
  bool early_range_end;
  bool intval_chg;

  DB::Cell::Key key_end;
  {
    std::shared_lock lock(m_mutex);
    key_end.copy(m_interval.key_end);
  }
  uint64_t ttl = cfg->cell_ttl();

  for(;;) {
    err = Error::OK;
    early_range_end = false;
    if(wait(COMPACT_COMPACTING)) { 
      // to wait only on COMPACT_APPLYING will require transfer-log
      std::shared_lock lock(m_mutex);
      key_end.copy(m_interval.key_end);
    }
    blocks.processing_increment();
  
    intval_chg = false;
    {
      std::shared_lock lock(m_mutex);
      req = m_q_adding.front();
    }
    ptr = req->input.base;
    remain = req->input.size; 
    if(req->cb->expired(remain/100000))
      err = Error::REQUEST_TIMEOUT;
      
    if(m_state != State::LOADED && m_state != State::UNLOADING) {
      err = m_state == State::DELETED ? 
            Error::COLUMN_MARKED_REMOVED 
            : Error::RS_NOT_LOADED_RANGE;
    }

    while(!err && remain) {
      cell.read(&ptr, &remain);

      if(cell.has_expired(ttl))
        continue;

      if(!key_end.empty() && key_end.compare(cell.key) == Condition::GT) {
        early_range_end = true;
        continue;
      } // + checking prev_key_end.compare(cell.key) != Condition::GT) {
        // late_range_begin = true;
        // continue;
        // }
        
      if(!(cell.control & DB::Cells::HAVE_TIMESTAMP)) {
        cell.set_timestamp(Time::now_ns());
        if(cell.control & DB::Cells::AUTO_TIMESTAMP)
          cell.control ^= DB::Cells::AUTO_TIMESTAMP;
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

    if(intval_chg) {
      intval_chg = false;
      on_change(err, false);
    }

    if(early_range_end) {
      req->cb->response(Error::RANGE_END_EARLIER, key_end);
    } else {
      req->cb->response(err);
    }
      
    delete req;
    {
      std::scoped_lock lock(m_mutex);
      m_q_adding.pop();
      if(m_q_adding.empty())
        break;
    }
  }
    
  if(blocks.commitlog.size_bytes_encoded() > 
      (cfg->cellstore_size()/100) * cfg->compact_percent()) {
    compact_require(true);
    RangerEnv::compaction_schedule(10000);
  }

}


}}


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.cc"
