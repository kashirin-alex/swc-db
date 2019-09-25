/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RS_Range_h
#define swcdb_lib_db_Columns_RS_Range_h

#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Protocol/req/RsRangeUnload.h"

#include "swcdb/lib/db/Types/Range.h"
#include "swcdb/lib/db/Files/RangeData.h"


namespace SWC { namespace server { namespace RS {

class Range : public DB::RangeBase {

  inline static const std::string range_data_file = "range.data";

  public:
  
  enum State{
    NOTLOADED,
    LOADED,
    DELETED
  };

  Range(int64_t cid, int64_t rid)
        : RangeBase(cid, rid, std::make_shared<DB::Cells::Intervals>()), 
          m_state(State::NOTLOADED) { 
            
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

  void load(ResponseCallbackPtr cb){
    bool is_loaded;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      is_loaded = m_state != State::NOTLOADED;
      m_state = State::LOADED;
    }
    int err = Env::RsData::is_shuttingdown()
              ?Error::SERVER_SHUTTING_DOWN:Error::OK;
    if(is_loaded || err != Error::OK)
      return loaded(err, cb);

    HT_DEBUGF("LOADING RANGE %s", to_string().c_str());

    if(!Env::FsInterface::interface()->exists(err, get_path(""))){
      if(err != Error::OK)
        return loaded(err, cb);
      Env::FsInterface::interface()->mkdirs(err, get_path("log"));
      Env::FsInterface::interface()->mkdirs(err, get_path("cs"));
      if(err != Error::OK)
        return loaded(err, cb);
      
      take_ownership(err, cb);
    } else {
      last_rs_chk(err, cb);
    }

  }

  void take_ownership(int &err, ResponseCallbackPtr cb){
    if(err == Error::RS_DELETED_RANGE)
      return loaded(err, cb);

    Env::RsData::get()->set_rs(err, get_path(rs_data_file));
    if(err != Error::OK)
      return loaded(err, cb);

    read_range_data(err, cb);
  }

  void on_change(){ // range-interval || cellstores
    
    switch(m_type){
      case Types::Range::DATA:
        // + INSERT meta-range(col-2), cid,rid,m_intervals(key)
        break;
      case Types::Range::META:
        // + INSERT master-range(col-1), cid(2),rid,m_intervals(key)
        break;
      default: // Types::Range::MASTER:
        break;
    }

    int err;
    Files::RangeData::save(err, get_path(range_data_file), cellstores);

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
    Files::RangeData::save(err, get_path(range_data_file), cellstores);
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
  
      for(auto& cs : cellstores){
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
    s.append(m_intervals->to_string());

    s.append(", cellstores=[");
    for(auto& cs : cellstores) {
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

  void last_rs_chk(int &err, ResponseCallbackPtr cb){
    // last_rs.data
    Files::RsDataPtr rs_data = Env::RsData::get();
    Files::RsDataPtr rs_last = get_last_rs(err);

    if(rs_last->endpoints.size() > 0 
      && !has_endpoint(rs_data->endpoints, rs_last->endpoints)){
      HT_DEBUGF("RS-LAST=%s RS-NEW=%s", 
                rs_last->to_string().c_str(), rs_data->to_string().c_str());
                
      Env::Clients::get()->rs->get(rs_last->endpoints)->put(
        std::make_shared<Protocol::Req::RsRangeUnload>(shared_from_this(), cb));
      return;
    }

    take_ownership(err, cb);
  }

  void read_range_data(int &err, ResponseCallbackPtr cb) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      // range.data
      Files::RangeData::load(err, get_path(range_data_file), cellstores);
      if(err != Error::OK || cellstores.size() == 0) {
        err = Error::OK;
        Files::RangeData::load_by_path(err, get_path("cs"), cellstores);
        for(auto& cs : cellstores) {
          cs->load_trailer();
        }
        if(err == Error::OK)
          Files::RangeData::save(err, get_path(range_data_file), cellstores);
      
        if(err != Error::OK) 
          err = Error::OK;
          // rs-to determine range-removal (+ Notify Mngr )
      }

      // cellstores


    //*
    // TEST-DATA
    if(cellstores.size() == 0) {
    cellstores.push_back(std::make_shared<Files::CellStore>(1));
    cellstores.push_back(std::make_shared<Files::CellStore>(2));
    cellstores.push_back(std::make_shared<Files::CellStore>(3));
    cellstores.push_back(std::make_shared<Files::CellStore>(4));
    cellstores.push_back(std::make_shared<Files::CellStore>(5));
    for(auto& cs : cellstores){
      auto s = std::to_string(cs->cs_id);
      DB::Specs::Key key;
      key.add("11", Condition::GE);
      key.add(std::string("a12345")+s, Condition::GE);
      key.add(std::string("b12345")+s, Condition::GE);
      key.add(std::string("c12345")+s, Condition::GE);
      key.add(std::string("d12345")+s, Condition::GE);
      cs->intervals->key_begin(key);
      key.free();

      key.add("11", Condition::LE);
      key.add(std::string("a98765")+s, Condition::LE);
      key.add(std::string("b98765")+s, Condition::LE);
      key.add(std::string("c98765")+s, Condition::LE);
      key.add(std::string("d98765")+s, Condition::LE);
      cs->intervals->key_end(key);

      DB::Specs::Timestamp ts;
      ts.comp = Condition::GE;
      ts.value = 1234;
      cs->intervals->ts_earliest(ts);
      ts.comp = Condition::LE;
      ts.value = 9876;
      cs->intervals->ts_latest(ts);
    }
    //delete [] d;
    }
    //*/

      bool init = false;
      for(auto& cs : cellstores) {
        m_intervals->expand(cs->intervals, init);
        init=true;
      }
      // CommitLogs
    

      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    
    loaded(err, cb); // RSP-LOAD-ACK

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


  State                 m_state;
  Types::Range          m_type;
  Files::CellStores     cellstores;

};


typedef std::shared_ptr<Range> RangePtr;

}} // server namespace


void Protocol::Req::RsRangeUnload::unloaded(int err, ResponseCallbackPtr cb) {
  std::dynamic_pointer_cast<server::RS::Range>(range)->take_ownership(err, cb);
}
bool Protocol::Req::RsRangeUnload::valid() {
  return !std::dynamic_pointer_cast<server::RS::Range>(range)->deleted();
}

}
#endif