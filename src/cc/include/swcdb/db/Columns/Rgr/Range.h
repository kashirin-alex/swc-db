/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_Range_h
#define swcdb_lib_db_Columns_Rgr_Range_h

#include "swcdb/db/Types/Range.h"
#include "swcdb/db/Protocol/Rgr/req/RangeUnload.h"

#include "swcdb/db/Files/RangeBlocks.h"

namespace SWC { namespace server { namespace Rgr {

class Range : public DB::RangeBase {

  public:
  
  typedef std::shared_ptr<Range>                    Ptr;

  struct ReqAdd final {
    public:
    ReqAdd(const StaticBuffer::Ptr& input, const ResponseCallback::Ptr& cb) 
          : input(input), cb(cb) {}
    ~ReqAdd() {}
    const StaticBuffer::Ptr     input;
    const ResponseCallback::Ptr cb;
  };

  enum State{
    NOTLOADED,
    LOADING,
    LOADED,
    UNLOADING,
    DELETED,
  };
  
  enum Compact{
    NONE,
    CHECKING,
    COMPACTING,
  };

  const Types::Range   type;
  Files::Range::Blocks blocks;

  Range(const DB::ColumnCfg* cfg, const int64_t rid)
        : RangeBase(cfg, rid), 
          m_state(State::NOTLOADED), 
          type(cfg->cid == 1 ? Types::Range::MASTER 
               :(cfg->cid == 2 ? Types::Range::META : Types::Range::DATA)),
          m_compacting(Compact::NONE), m_require_compact(false) {
  }

  void init() {
    blocks.init(shared_from_this());
  }

  Ptr shared() {
    return std::dynamic_pointer_cast<Range>(shared_from_this());
  }

  static Ptr shared(const DB::RangeBase::Ptr& other){
    return std::dynamic_pointer_cast<Range>(other);
  }

  virtual ~Range() {
    SWC_LOG_OUT(LOG_INFO) 
      << " ~Range cid=" << cfg->cid << " rid=" << rid 
      << SWC_LOG_OUT_END;
  }
  
  void set_state(State new_state) {
    std::scoped_lock lock(m_mutex);
    m_state = new_state;
  }

  bool is_loaded() {
    std::shared_lock lock(m_mutex);
    return m_state == State::LOADED;
  }

  bool deleted() { 
    std::shared_lock lock(m_mutex);
    return m_state == State::DELETED;
  }

  void add(ReqAdd* req) {
    std::scoped_lock lock(m_mutex);
    m_q_adding.push(req);
    
    if(m_q_adding.size() == 1) { 
      asio::post(*Env::IoCtx::io()->ptr(), 
        [ptr=shared()](){ ptr->run_add_queue(); }
      );
    }
  }

  void scan(DB::Cells::ReqScan::Ptr req) {
    if(wait() && !is_loaded()) {
      int err = Error::RS_NOT_LOADED_RANGE;
      req->response(err);
      return;
    }
    blocks.scan(req);
  }

  void scan_internal(DB::Cells::ReqScan::Ptr req) {
    blocks.scan(req);
  }

  void create_folders(int& err) {
    Env::FsInterface::interface()->mkdirs(err, get_path(log_dir));
    Env::FsInterface::interface()->mkdirs(err, get_path(cellstores_dir));
  }

  void load(ResponseCallback::Ptr cb) {
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

    if(!Env::FsInterface::interface()->exists(err, get_path(cellstores_dir))) {
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

  void take_ownership(int &err, ResponseCallback::Ptr cb) {
    if(err == Error::RS_DELETED_RANGE)
      return loaded(err, cb);

    RangerEnv::rgr_data()->set_rgr(
      err, get_path(ranger_data_file), cfg->block_replication());
    if(err != Error::OK)
      return loaded(err, cb);

    load(err, cb);
  }

  void on_change(int &err, bool removal, 
                 const DB::Cell::Key* old_key_begin=nullptr) {
    std::scoped_lock lock(m_mutex);
    
    if(type == Types::Range::MASTER) {
      // update manager-root
      return;
    }

    auto updater = RangerEnv::updater();
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
        
      cell.own = true;
      cell.vlen = key_end.encoded_length() 
                + Serialization::encoded_length_vi64(rid);
      cell.value = new uint8_t[cell.vlen];
      uint8_t * ptr = cell.value;
      key_end.encode(&ptr);
      Serialization::encode_vi64(&ptr, rid);
      cell.set_time_order_desc(true);
      updater->columns->add(cid_typ, cell);

      if(old_key_begin && !old_key_begin->equal(m_interval.key_begin)) {
        cell.free();
        cell.flag = DB::Cells::DELETE;
        cell.key.copy(*old_key_begin);
        cell.key.insert(0, std::to_string(cfg->cid));
        updater->columns->add(cid_typ, cell);
      }
    }
    updater->commit(cid_typ);
      
    // INSERT master-range(col-1), key[cid+m_interval(data(cid)+key)], value[rid]
    // INSERT meta-range(col-2), key[cid+m_interval(key)], value[rid]
  }

  void unload(Callback::RangeUnloaded_t cb, bool completely) {
    int err = Error::OK;
    {
      std::scoped_lock lock(m_mutex);
      if(m_state == State::DELETED){
        cb(err);
        return;
      }
      m_state = State::UNLOADING;
    }

    wait();
    wait_queue();

    set_state(State::NOTLOADED);

    blocks.unload();

    if(completely) // whether to keep ranger_data_file
      Env::FsInterface::interface()->remove(err, get_path(ranger_data_file));
    
    SWC_LOGF(LOG_INFO, "UNLOADED RANGE cid=%d rid=%d err=%d(%s)", 
              cfg->cid, rid, err, Error::get_text(err));
    cb(err);
  }
  
  void remove(int &err) {
    {
      std::scoped_lock lock(m_mutex);
      m_state = State::DELETED;
    }
    on_change(err, true);

    wait();
    wait_queue();
    blocks.remove(err);

    Env::FsInterface::interface()->rmdir(err, get_path(""));  

    SWC_LOGF(LOG_INFO, "REMOVED RANGE %s", to_string().c_str());
  }

  void wait_queue() {
    for(;;) {
      {
        std::shared_lock lock(m_mutex);
        if(m_q_adding.empty())
          break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  const bool compacting() {
    std::shared_lock lock(m_mutex);
    return m_compacting != Compact::NONE;
  }

  void compacting(Compact state) {
    std::scoped_lock lock(m_mutex);
    m_compacting = state;
    if(state == Compact::NONE)
      m_cv.notify_all();
  }
  
  const bool compact_possible() {
    std::scoped_lock lock(m_mutex);
    if(m_state != State::LOADED || m_compacting != Compact::NONE
       || (!m_require_compact && blocks.processing()))
      return false;
    m_compacting = Range::Compact::CHECKING;
    return true;
  }

  void compact_require(bool require) {
    std::scoped_lock lock(m_mutex);
    m_require_compact = require;
  }

  const bool compact_required() {
    std::shared_lock lock(m_mutex);
    return m_require_compact;
  }

  void apply_new(int &err,
                Files::CellStore::Writers& w_cellstores, 
                std::vector<Files::CommitLog::Fragment::Ptr>& fragments_old) {
    bool intval_changed;
    DB::Cell::Key old_key_begin;
    {
      std::scoped_lock lock(m_mutex);
      blocks.apply_new(err, w_cellstores, fragments_old);
      if(err)
        return;

      old_key_begin.copy(m_interval.key_begin);
      DB::Cell::Key old_key_end(m_interval.key_end);
      m_interval.free();
      blocks.cellstores.expand(m_interval);
      intval_changed = !m_interval.key_begin.equal(old_key_begin) ||
                       !m_interval.key_end.equal(old_key_end);
    }
    if(intval_changed)
      on_change(err, false, &old_key_begin);
    err = Error::OK;
  }
  
  void create(int &err, const Files::CellStore::Writers& w_cellstores) {
    
    create_folders(err);
    if(err)
      return;
    RangerEnv::rgr_data()->set_rgr(
      err, get_path(ranger_data_file), cfg->block_replication());
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
        Files::CellStore::Read::make(
          err, cs->id, shared_from_this(), cs->interval)
      );
      if(err)
        return;
    }

    m_interval.free();
    blocks.cellstores.expand(m_interval);
    Files::RangeData::save(err, blocks.cellstores);
    on_change(err, false);
    
    fs->remove(err, get_path(ranger_data_file));
  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);
    
    std::string s("(");
    s.append(DB::RangeBase::to_string());
    s.append(" state=");
    s.append(std::to_string(m_state));
    s.append(" type=");
    s.append(Types::to_string(type));
    s.append(" ");
    s.append(blocks.to_string());
    s.append(")");
    return s;
  }

  private:

  void loaded(int &err, ResponseCallback::Ptr cb) {
    {
      std::shared_lock lock(m_mutex);
      if(m_state == State::DELETED)
        err = Error::RS_DELETED_RANGE;
    }
    cb->response(err);
  }

  void last_rgr_chk(int &err, ResponseCallback::Ptr cb) {
    // ranger.data
    auto rgr_data = RangerEnv::rgr_data();
    Files::RgrData::Ptr rs_last = get_last_rgr(err);

    if(rs_last->endpoints.size() 
      && !has_endpoint(rgr_data->endpoints, rs_last->endpoints)){
      SWC_LOGF(LOG_DEBUG, "RANGER-LAST=%s RANGER-NEW=%s", 
                rs_last->to_string().c_str(), rgr_data->to_string().c_str());
                
      Env::Clients::get()->rgr->get(rs_last->endpoints)->put(
        std::make_shared<Protocol::Rgr::Req::RangeUnload>(
          shared_from_this(), cb));
      return;
    }

    take_ownership(err, cb);
  }

  void load(int &err, ResponseCallback::Ptr cb) {
    bool is_initial_column_range = false;
    Files::RangeData::load(err, blocks.cellstores);
    if(err) 
      (void)err;
      //err = Error::OK; // ranger-to determine range-removal (+ Notify Mngr)

    else if(blocks.cellstores.empty()) {
      // init 1st cs(for log_cells)
      auto cs = Files::CellStore::create_init_read(
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
        blocks.cellstores.expand(m_interval);
        if(is_initial_column_range) {
          Files::RangeData::save(err, blocks.cellstores);
          on_change(err, false);
        }
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

  const bool wait() {
    bool waited;
    std::unique_lock lock_wait(m_mutex);
    if(waited = (m_compacting == Compact::COMPACTING))
      m_cv.wait(
        lock_wait, 
        [&compacting=m_compacting](){return compacting == Compact::NONE;}
      );  
    return waited;
  }

  void run_add_queue() {
    ReqAdd* req;

    int err;
    DB::Cells::Cell cell;
    const uint8_t* ptr;
    size_t remain;

    for(;;) {
      err = Error::OK;
      {
        std::shared_lock lock(m_mutex);
        req = m_q_adding.front();
      }
      uint32_t count = 0;
      ptr = req->input->base;
      remain = req->input->size; 

      while(!err && remain) {
        cell.read(&ptr, &remain);
        if(!m_interval.is_in_end(cell.key)) {
          // skip( with error)
          continue;
        }
        
        if(!(cell.control & DB::Cells::HAVE_TIMESTAMP)) {
          cell.set_timestamp(Time::now_ns());
          if(cell.control & DB::Cells::AUTO_TIMESTAMP)
            cell.control ^= DB::Cells::AUTO_TIMESTAMP;
        }
        if(!(cell.control & DB::Cells::HAVE_REVISION))
          cell.control |= DB::Cells::REV_IS_TS;
        
        if(m_state != State::LOADED && m_state != State::UNLOADING) {
          err = m_state == State::DELETED ? 
                Error::COLUMN_MARKED_REMOVED 
                : Error::RS_NOT_LOADED_RANGE;
          break;
        } 

        wait();
        blocks.add_logged(cell);
        //SWC_LOG_OUT(LOG_INFO) 
        //  << " range(added) "<< cell.to_string() 
        //  << SWC_LOG_OUT_END;
        count++;
      }
      req->cb->response(err);

      delete req;
      {
        std::scoped_lock lock(m_mutex);
        m_q_adding.pop();
        if(m_q_adding.empty())
          break;
      }
    }
    
    if(blocks.commitlog.size() > cfg->cellstore_size()/cfg->block_size()) {
      compact_require(true);
      RangerEnv::compaction_schedule(10000);
    }
    
    //std::cout << " run_add_queue log-count=" << m_commit_log->cells_count() 
    //          << " blocks-count=" << blocks.cells_count()
    //          << " " << blocks.to_string() << "\n";

  }


  std::atomic<State>            m_state;
   
  Compact                       m_compacting;
  bool                          m_require_compact;
  std::queue<ReqAdd*>           m_q_adding;

  std::condition_variable_any   m_cv;
};



}} // server namespace


void Protocol::Rgr::Req::RangeUnload::unloaded(
                            int err, ResponseCallback::Ptr cb) {
  server::Rgr::Range::shared(range)->take_ownership(err, cb);
}
bool Protocol::Rgr::Req::RangeUnload::valid() {
  return !server::Rgr::Range::shared(range)->deleted();
}

}
#endif