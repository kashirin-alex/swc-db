/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_Range_h
#define swcdb_lib_db_Columns_Rgr_Range_h

#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Protocol/Rgr/req/RangeUnload.h"

#include "swcdb/lib/db/Cells/Mutable.h"
#include "swcdb/lib/db/Types/Range.h"
#include "swcdb/lib/db/Files/RangeData.h"
#include "swcdb/lib/db/Protocol/Common/req/Query.h"

#include "IntervalBlocks.h"


namespace Query = SWC::Protocol::Common::Req::Query;

namespace SWC { namespace server { namespace Rgr {

class Range : public DB::RangeBase {

  public:
  
  typedef std::shared_ptr<Range>                    Ptr;

  struct ReqAdd {
    public:
    ReqAdd(const StaticBufferPtr& input, const ResponseCallbackPtr& cb) 
          : input(input), cb(cb) {}
    virtual ~ReqAdd() {}
    const StaticBufferPtr     input;
    const ResponseCallbackPtr cb;
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
  IntervalBlocks       blocks;

  Range(const int64_t cid, const int64_t rid)
        : RangeBase(cid, rid), 
          m_state(State::NOTLOADED), 
          type(cid == 1 ? Types::Range::MASTER 
               :(cid == 2 ? Types::Range::META : Types::Range::DATA)),
          m_compacting(Compact::NONE) {
  }

  void init() {
    blocks.init(shared_from_this());
  }

  inline Ptr shared() {
    return std::dynamic_pointer_cast<Range>(shared_from_this());
  }

  inline static Ptr shared(const DB::RangeBase::Ptr& other){
    return std::dynamic_pointer_cast<Range>(other);
  }

  virtual ~Range(){
    std::cout << " ~Range cid=" << cid << " rid=" << rid << "\n"; 
  }
  
  void set_state(State new_state) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = new_state;
  }

  bool is_loaded() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::LOADED;
  }

  bool deleted() { 
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::DELETED;
  }

  void add(ReqAdd* req) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_q_adding.push(req);
    
    if(m_q_adding.size() == 1) { 
      asio::post(*Env::IoCtx::io()->ptr(), 
        [ptr=shared()](){ ptr->run_add_queue(); }
      );
    }
  }

  void scan(DB::Cells::ReqScan::Ptr req) {
    wait();
    blocks.scan(req);
  }

  void scan_internal(DB::Cells::ReqScan::Ptr req) {
    blocks.scan(req);
  }

  void load(ResponseCallbackPtr cb) {
    bool is_loaded;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      is_loaded = m_state != State::NOTLOADED;
      if(m_state == State::NOTLOADED)
        m_state = State::LOADING;
    }
    int err = Env::RgrData::is_shuttingdown()
              ?Error::SERVER_SHUTTING_DOWN:Error::OK;
    if(is_loaded || err != Error::OK)
      return loaded(err, cb);

    HT_DEBUGF("LOADING RANGE %s", to_string().c_str());

    if(!Env::FsInterface::interface()->exists(err, get_path(""))){
      if(err != Error::OK)
        return loaded(err, cb);
      Env::FsInterface::interface()->mkdirs(err, get_path(log_dir));
      Env::FsInterface::interface()->mkdirs(err, get_path(cellstores_dir));
      if(err != Error::OK)
        return loaded(err, cb);
      
      take_ownership(err, cb);
    } else {
      last_rgr_chk(err, cb);
    }

  }

  void take_ownership(int &err, ResponseCallbackPtr cb) {
    if(err == Error::RS_DELETED_RANGE)
      return loaded(err, cb);

    Env::RgrData::get()->set_rgr(err, get_path(ranger_data_file));
    if(err != Error::OK)
      return loaded(err, cb);

    load(err, cb);
  }

  void on_change(int &err, bool removal=false) { // change of range-interval
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if(type != Types::Range::MASTER) {
      uint8_t cid_typ = type == Types::Range::DATA ? 2 : 1;
      
      if(m_req_set_intval == nullptr) {
        DB::SchemaPtr schema = Env::Schemas::get()->get(cid_typ);
        if(schema == nullptr) {
          schema = Env::Clients::get()->schemas->get(err, cid_typ);
          if(err)
            return;
        }
        m_req_set_intval = std::make_shared<Query::Update>();
        m_req_set_intval->columns_cells->create(schema);
      }

      DB::Cells::Cell cell;
      cell.key.copy(m_interval.key_begin);
      cell.key.insert(0, std::to_string(cid));

      if(removal) {
        cell.flag = DB::Cells::DELETE;
      } else {

        cell.flag = DB::Cells::INSERT;
        DB::Cell::Key key_end(m_interval.key_end);
        key_end.insert(0, std::to_string(cid));
        
        cell.own = true;
        cell.vlen = Serialization::encoded_length_vi64(rid) 
                  + key_end.encoded_length();
        cell.value = new uint8_t[cell.vlen];
        uint8_t * ptr = cell.value;
        Serialization::encode_vi64(&ptr, rid);
        key_end.encode(&ptr);
        m_req_set_intval->columns_cells->add(cid_typ, cell);

        if(m_interval_old.was_set) {
          cell.free();
          cell.flag = DB::Cells::DELETE;
          cell.key.copy(m_interval_old.key_begin);
          cell.key.insert(0, std::to_string(cid));
          m_req_set_intval->columns_cells->add(cid_typ, cell);
        }
      }
      m_req_set_intval->commit();
      m_req_set_intval->wait();
      err = m_req_set_intval->result->err;
      // INSERT master-range(col-1), key[cid+m_interval(data(cid)+key)], value[rid]
      // INSERT meta-range(col-2), key[cid+m_interval(key)], value[rid]
    } else {
      // update manager-root
    }
  }

  void unload(Callback::RangeUnloaded_t cb, bool completely) {
    int err = Error::OK;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
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
    
    HT_INFOF("UNLOADED RANGE cid=%d rid=%d err=%d(%s)", 
              cid, rid, err, Error::get_text(err));
    cb(err);
  }

  void wait_queue() {
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_q_adding.empty())
          break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  
  void remove(int &err) {
    wait();
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_state = State::DELETED;
    }
    wait_queue();
    blocks.remove(err);

    Env::FsInterface::interface()->rmdir(err, get_path(""));  
    
    on_change(err, true);
    HT_INFOF("REMOVED RANGE %s", to_string().c_str());
  }

  const bool compacting() {
    return m_compacting != Compact::NONE;
  }

  void compacting(Compact state) {
    m_compacting = state;
    if(m_compacting == Compact::NONE)
      m_cv.notify_all();
  }

  void apply_new(int &err,
                Files::CellStore::Writers& w_cellstores, 
                std::vector<Files::CommitLog::Fragment::Ptr>& fragments_old) {
    std::cout << "RANGE::apply_new 1\n";
    bool intval_changed;
    {
      auto fs = Env::FsInterface::interface();
      std::lock_guard<std::mutex> lock(m_mutex);
      
      fs->rename(err, get_path(cellstores_dir), get_path(cellstores_bak_dir));
      if(err)
        return;
      fs->rename(err, get_path(cellstores_tmp_dir), get_path(cellstores_dir));
      if(err) {
        fs->rmdir(err, get_path(cellstores_dir));
        fs->rename(err, get_path(cellstores_bak_dir), get_path(cellstores_dir));
        return;
      }
      fs->rmdir(err, get_path(cellstores_bak_dir));

      // cellstores->replace(..);
      blocks.cellstores->clear();
      for(auto cs : w_cellstores) {
        blocks.cellstores->add(
          Files::CellStore::Read::make(
            cs->id, shared_from_this(), cs->interval
          )
        );
        // cs->load_blocks_index(err, true);
        // cs can be not loaded and done with 1st scan-req
      }
      Files::RangeData::save(err, blocks.cellstores);
      
      blocks.commitlog->remove(err, fragments_old);

      m_interval_old.copy(m_interval);
      m_interval.free();
      blocks.cellstores->expand(m_interval);

      intval_changed = !m_interval.key_begin.equal(m_interval_old.key_begin)
                    || !m_interval.key_end.equal(m_interval_old.key_end); 
    }

    if(intval_changed)
      on_change(err);
    err = Error::OK;
    std::cout << "RANGE::apply_new 2\n";
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
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

  void loaded(int &err, ResponseCallbackPtr cb) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state == State::DELETED)
        err = Error::RS_DELETED_RANGE;
    }
    cb->response(err);
  }

  void last_rgr_chk(int &err, ResponseCallbackPtr cb) {
    // ranger.data
    Files::RgrDataPtr rs_data = Env::RgrData::get();
    Files::RgrDataPtr rs_last = get_last_rgr(err);

    if(rs_last->endpoints.size() > 0 
      && !has_endpoint(rs_data->endpoints, rs_last->endpoints)){
      HT_DEBUGF("RANGER-LAST=%s RANGER-NEW=%s", 
                rs_last->to_string().c_str(), rs_data->to_string().c_str());
                
      Env::Clients::get()->rgr->get(rs_last->endpoints)->put(
        std::make_shared<Protocol::Rgr::Req::RangeUnload>(
          shared_from_this(), cb));
      return;
    }

    take_ownership(err, cb);
  }

  void load(int &err, ResponseCallbackPtr cb) {
    bool is_initial_column_range = false;
    Files::RangeData::load(err, blocks.cellstores);
    if(err) 
      (void)err;
      //err = Error::OK; // ranger-to determine range-removal (+ Notify Mngr)

    else if(blocks.cellstores->empty()) {
      // init 1st cs(for log_cells)
      DB::SchemaPtr schema = Env::Schemas::get()->get(cid);
      Files::CellStore::Read::Ptr cs 
        = Files::CellStore::create_init_read(
          err, schema->blk_encoding, shared_from_this());
      if(!err) {
        blocks.cellstores->add(cs);
        is_initial_column_range = true;
      }
    }
 
    if(!err) {
      blocks.load(err);
      if(!err) {
        m_interval.free();
        blocks.cellstores->expand(m_interval);
        if(is_initial_column_range) {
          Files::RangeData::save(err, blocks.cellstores);
          on_change(err);
        }
      }
    }
  
    if(!err) 
      set_state(State::LOADED);
      
    loaded(err, cb);   // RSP-LOAD-ACK

    if(is_loaded()) {
      HT_INFOF("LOADED RANGE %s", to_string().c_str());
    } else 
      HT_WARNF("LOAD RANGE FAILED err=%d(%s) %s", 
               err, Error::get_text(err), to_string().c_str());
  }

  void wait() {  
    if(m_compacting == Compact::COMPACTING) {
      std::unique_lock<std::mutex> lock_wait(m_mutex);
      m_cv.wait(lock_wait, [&compacting=m_compacting](){return compacting == Compact::NONE;});  
    }
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
        std::lock_guard<std::mutex> lock(m_mutex);
        req = m_q_adding.front();
      }
      uint32_t count = 0;
      ptr = req->input->base;
      remain = req->input->size; 

      while(!err && remain) {
        cell.read(&ptr, &remain);
        if(!m_interval.is_in_end(cell.key)) {
          // validate over range.interval match skip( with error)
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
        count++;
      }
      req->cb->response(err);

      delete req;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_q_adding.pop();
        if(m_q_adding.empty())
          break;
      }
    }
    
    
    //std::cout << " run_add_queue log-count=" << m_commit_log->cells_count() 
    //          << " blocks-count=" << blocks.cells_count()
    //          << " " << blocks.to_string() << "\n";

  }


  std::atomic<State>                m_state;
   
  std::atomic<Compact>              m_compacting;
  std::queue<ReqAdd*>               m_q_adding;

  
  Query::Update::Ptr                m_req_set_intval;

  std::condition_variable           m_cv;
  DB::Cells::Interval               m_interval_old;
};



}} // server namespace


void Protocol::Rgr::Req::RangeUnload::unloaded(int err, ResponseCallbackPtr cb) {
  server::Rgr::Range::shared(range)->take_ownership(err, cb);
}
bool Protocol::Rgr::Req::RangeUnload::valid() {
  return !server::Rgr::Range::shared(range)->deleted();
}

}
#endif