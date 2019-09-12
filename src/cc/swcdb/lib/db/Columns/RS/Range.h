/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RS_Range_h
#define swcdb_lib_db_Columns_RS_Range_h

#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Types/Range.h"

namespace SWC { namespace server { namespace RS {

class Range;
typedef std::shared_ptr<Range> RangePtr;

}}}

#include "swcdb/lib/db/Protocol/req/RsRangeUnload.h"

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
        : RangeBase(cid, rid), 
          m_state(State::NOTLOADED),
          intervals(std::make_shared<Cells::Intervals>()) { 
            
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

    if(is_loaded()) {
      cb->response_ok();
      return;
    }
    set_state(State::LOADED);

    HT_DEBUGF("LOADING RANGE %s", to_string().c_str());
    int err = Error::OK;

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
    loaded(err, cb); // RSP-LOAD-ACK
    if(err != Error::OK)
      return;

    read_range_data(err);
  }

  void on_change(){ // range-interval || cellstores
    
    switch(m_type){
      case Types::Range::DATA:
        // + INSERT meta-range(col-2), cid,rid,intervals(keys)
        break;
      case Types::Range::META:
        // + INSERT master-range(col-1), cid(2),rid,intervals(keys)
        break;
      default: // Types::Range::MASTER:
        break;
    }

    int err;
    Files::RangeData::save(err, get_path(range_data_file), cellstores);

  }

  void unload(int &err, bool completely){

    // CommitLogs  
    // CellStores
    // range.data
    /* 
    cellstores.clear();
    // TEST-DATA
    cellstores.push_back(std::make_shared<Files::CellStore>(1));
    cellstores.push_back(std::make_shared<Files::CellStore>(2));
    cellstores.push_back(std::make_shared<Files::CellStore>(3));
    cellstores.push_back(std::make_shared<Files::CellStore>(4));
    cellstores.push_back(std::make_shared<Files::CellStore>(5));
    uint8_t* d = new uint8_t[
      Serialization::encoded_length_vi32(28)
      +28
      +Serialization::encoded_length_vi32(6)
      +6
      +Serialization::encoded_length_vi64(123)
      +Serialization::encoded_length_vi64(987)
      ];
    const uint8_t * base = d;
    Serialization::encode_vi32(&d, 28);
    *d++ = 'a';
    *d++ = '1';
    *d++ = '2'; 
    *d++ = '3'; 
    *d++ = '4'; 
    uint8_t* ptr_muta = d;
    *d++ = '5'; // chk changed
    *d++ = 0;
    *d++ = 'b';
    *d++ = '1';
    *d++ = '2'; 
    *d++ = '3'; 
    *d++ = '4'; 
    *d++ = '5'; 
    *d++ = 0;
    *d++ = 'c';
    *d++ = '1';
    *d++ = '2'; 
    *d++ = '3'; 
    *d++ = '4'; 
    *d++ = '5'; 
    *d++ = 0;
    *d++ = 'd';
    *d++ = '1';
    *d++ = '2'; 
    *d++ = '3'; 
    *d++ = '4'; 
    *d++ = '5'; 
    *d++ = 0; 
    Serialization::encode_vi32(&d, 6);
    *d++ = 'a';
    *d++ = '6';
    *d++ = 0;
    *d++ = 'b';
    *d++ = '6';
    *d++ = 0; 
 
    Serialization::encode_vi64(&d, 123);
    Serialization::encode_vi64(&d, 987);
    size_t remain = d-base;
    for(auto& cs : cellstores){
      size_t remain2 = remain;
      const uint8_t * base2 = base;
      *ptr_muta = *(uint8_t*)std::to_string(cs->cs_id).c_str();
      cs->intervals->decode(&base2, &remain2);
    }
    delete [] d;

    std::cout << to_string() << "\n";;

    Files::RangeData::save(get_path(range_data_file), cellstores);
    */
    if(completely)
      Env::FsInterface::interface()->remove(err, get_path(rs_data_file));

    set_state(State::NOTLOADED);
    
    HT_INFOF("UNLOADED RANGE cid=%d rid=%d", cid, rid);
  }

  void remove(int &err){
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_state = State::DELETED;
  
      for(auto& cs : cellstores){
        cs->remove(err);
      }

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
    s.append(intervals->to_string());

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
    if(err == Error::OK)
      cb->response_ok(); // cb->run();
    else
      // ? remove from map
     cb->send_error(err == Error::OK? Error::RS_NOT_LOADED_RANGE: err , "");
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

  void read_range_data(int &err) {

    // range.data
    Files::RangeData::load(err, get_path(range_data_file), cellstores);
    if(err != Error::OK || cellstores.size() == 0) {
      err = Error::OK;
      Files::RangeData::load_by_path(err, get_path("cs"), cellstores);
      if(err == Error::OK)
        Files::RangeData::save(err, get_path(range_data_file), cellstores);
      
      if(err != Error::OK)
        // rs-to determine range-removal (+ Notify Mngr )
        return;
    }

    int64_t ts;
    int64_t ts_earliest = ScanSpecs::TIMESTAMP_NULL;
    int64_t ts_latest = ScanSpecs::TIMESTAMP_NULL;
    for(auto& cs : cellstores){
      // intervals->expande(cs->intervals);

      if(intervals->is_any_keys_begin() 
        || !intervals->is_in_begin(cs->intervals->get_keys_begin()))
        intervals->set_keys_begin(cs->intervals);

      if(intervals->is_any_keys_end() 
        || !intervals->is_in_end(cs->intervals->get_keys_end()))
        intervals->set_keys_end(cs->intervals);

      ts = cs->intervals->get_ts_earliest();
      if(ts_earliest == ScanSpecs::TIMESTAMP_NULL || ts_earliest > ts)
        ts_earliest = ts;
      ts = cs->intervals->get_ts_latest();
      if(ts_latest == ScanSpecs::TIMESTAMP_NULL || ts_latest < ts)
        ts_latest = ts;
      }
    
    intervals->set_ts_earliest(ts_earliest);
    intervals->set_ts_latest(ts_latest);
    
    // cellstores
    // CommitLogs
    

    set_state(State::LOADED);
    if(is_loaded()) {
      HT_INFOF("LOADED RANGE %s", to_string().c_str());
      return;
    }
    
    HT_WARNF("LOAD RANGE FAILED %s", to_string().c_str());
    return;
  }


  State               m_state;
  
  Types::Range        m_type;
  Cells::IntervalsPtr intervals;
  Files::CellStores   cellstores;

};



}} // server namespace


void Protocol::Req::RsRangeUnload::unloaded(int err, ResponseCallbackPtr cb) {
  std::dynamic_pointer_cast<server::RS::Range>(range)->take_ownership(err, cb);
}
bool Protocol::Req::RsRangeUnload::valid() {
  return !std::dynamic_pointer_cast<server::RS::Range>(range)->deleted();
}

}
#endif