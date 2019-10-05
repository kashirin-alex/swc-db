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

//#include "RangeIntervals.h"
#include "CommitLog.h"
//#include "CellStore.h"


namespace SWC { namespace server { namespace Rgr {

class Range : public DB::RangeBase {

  inline static const std::string range_data_file = "range.data";

  public:
  
  struct ReqAdd {
    const StaticBufferPtr     input;
    const ResponseCallbackPtr cb;
  };

  enum State{
    NOTLOADED,
    LOADED,
    DELETED
  };

  Range(int64_t cid, int64_t rid)
        : RangeBase(cid, rid, std::make_shared<DB::Cells::Interval>()), 
          m_state(State::NOTLOADED)  { 
            
    if(cid == 1)
      m_type = Types::Range::MASTER;
    else if(cid == 2)
      m_type = Types::Range::META;
    else
      m_type = Types::Range::DATA;
  }

  virtual ~Range(){}
  
  void set_state(State new_state){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = new_state;
  }

  bool is_loaded(){
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
        [ptr=std::dynamic_pointer_cast<Range>(shared_from_this())](){
          ptr->_run_add_queue();
        }
      );
    }
  }

  void scan(DB::Specs::Interval::Ptr interval, ResponseCallbackPtr cb) {
    int err = Error::OK;
    cb->response(err);
  }

  void load(ResponseCallbackPtr cb){
    bool is_loaded;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      is_loaded = m_state != State::NOTLOADED;
      m_state = State::LOADED;
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

  void take_ownership(int &err, ResponseCallbackPtr cb){
    if(err == Error::RS_DELETED_RANGE)
      return loaded(err, cb);

    Env::RgrData::get()->set_rgr(err, get_path(rs_data_file));
    if(err != Error::OK)
      return loaded(err, cb);

    load_range_data(err, cb);
  }

  void on_change(int &err){ // range-interval || cellstores
    std::lock_guard<std::mutex> lock(m_mutex);
    
    switch(m_type){
      case Types::Range::DATA:
        // + INSERT meta-range(col-2), cid,rid,m_interval(key)
        break;
      case Types::Range::META:
        // + INSERT master-range(col-1), cid(2),rid,m_interval(key)
        break;
      default: // Types::Range::MASTER:
        break;
    }

    Files::RangeData::save(err, get_path(range_data_file), m_cellstores);
  }

  void unload(Callback::RangeUnloaded_t cb, bool completely){
    int err = Error::OK;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state == State::DELETED){
        cb(err);
        return;
      }
    }
    // CommitLogs  
    // CellStores
    // range.data



    // rs_last.data
    if(completely)
      Env::FsInterface::interface()->remove(err, get_path(rs_data_file));


    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    set_state(State::NOTLOADED);
    
    HT_INFOF("UNLOADED RANGE cid=%d rid=%d err=%d(%s)", 
              cid, rid, err, Error::get_text(err));
    cb(err);
  }

  void remove(int &err){
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_state = State::DELETED;
  
      for(auto& cs : m_cellstores){
        cs->remove(err);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      Env::FsInterface::interface()->rmdir(err, get_path(""));
    }
    HT_INFOF("REMOVED RANGE %s", to_string().c_str());
  }

  std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string s("[");
    s.append(DB::RangeBase::to_string());
    s.append(", state=");
    s.append(std::to_string(m_state));
    
    s.append(", type=");
    s.append(std::to_string((int)m_type));

    s.append(", ");
    s.append(m_interval->to_string());

    s.append(", cellstores=[");
    for(auto& cs : m_cellstores) {
      s.append(cs->to_string());
      s.append(", ");
    }
    s.append("], ");
    
    s.append("]");
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

  void last_rgr_chk(int &err, ResponseCallbackPtr cb){
    // ranger.data
    Files::RgrDataPtr rs_data = Env::RgrData::get();
    Files::RgrDataPtr rs_last = get_last_rgr(err);

    if(rs_last->endpoints.size() > 0 
      && !has_endpoint(rs_data->endpoints, rs_last->endpoints)){
      HT_DEBUGF("RANGER-LAST=%s RANGER-NEW=%s", 
                rs_last->to_string().c_str(), rs_data->to_string().c_str());
                
      Env::Clients::get()->rgr->get(rs_last->endpoints)->put(
        std::make_shared<Protocol::Rgr::Req::RangeUnload>(shared_from_this(), cb));
      return;
    }

    take_ownership(err, cb);
  }

  void load_range_data(int &err, ResponseCallbackPtr cb) {
    
    Files::RangeData::load(err, get_path(range_data_file), m_cellstores);
    if(err != Error::OK || m_cellstores.size() == 0) {
      err = Error::OK;
      Files::RangeData::load_by_path(err, get_path(cellstores_dir), m_cellstores);
      if(err == Error::OK)
        Files::RangeData::save(err, get_path(range_data_file), m_cellstores);
      if(err != Error::OK) 
        err = Error::OK; // ranger-to determine range-removal (+ Notify Mngr)
    }
    
    for(auto& cs: m_cellstores) {
      cs->smartfd = FS::SmartFd::make_ptr(get_path_cs(cs->id), 0); 
      if(!cs->interval)
        cs->load_blocks_index(err, true);
    }
    // sort on interval, if interval loaded

    
    /*
    m_intervals = std::make_shared<RangeIntervals>(
      shared_from_this(), cellstores);
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_intervals->expand(m_interval);
    }
    */
    loaded(err, cb); // RSP-LOAD-ACK

    on_change(err);

    if(err == Error::OK) {
      set_state(State::LOADED);
      if(is_loaded()) {
        HT_INFOF("LOADED RANGE %s", to_string().c_str());
        return;
      }
    }
    HT_WARNF("LOAD RANGE FAILED err=%d(%s) %s", 
             err, Error::get_text(err), to_string().c_str());
    return; 
  }


  void _run_add_queue() {
    ReqAdd* req;

    int err;
    DB::Cells::Cell cell;
    uint8_t* ptr;
    size_t remain;

    for(;;) {
      err = Error::OK;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        req = m_q_adding.front();
      }

      ptr = req->input->base;
      remain = req->input->size; 
      while(remain) {
        cell.read(&ptr, &remain);
        // COMMITLOG->add(
        // CS-BLOCK->LOG_CELLS-add(
        //m_intervals->add(cell);
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

  }

  State                       m_state;
  Types::Range                m_type;

  Files::CellStore::RPtrs     m_cellstores;
  std::queue<ReqAdd*>         m_q_adding;
};


typedef std::shared_ptr<Range> RangePtr;

}} // server namespace


void Protocol::Rgr::Req::RangeUnload::unloaded(int err, ResponseCallbackPtr cb) {
  std::dynamic_pointer_cast<server::Rgr::Range>(range)->take_ownership(err, cb);
}
bool Protocol::Rgr::Req::RangeUnload::valid() {
  return !std::dynamic_pointer_cast<server::Rgr::Range>(range)->deleted();
}

}
#endif