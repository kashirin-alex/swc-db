/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RS_Range_h
#define swcdb_lib_db_Columns_RS_Range_h

#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Protocol/req/UnloadRange.h"

#include "swcdb/lib/db/Files/RangeData.h"
#include "swcdb/lib/db/Types/Range.h"


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

  bool load(){
    HT_DEBUGF("LOADING RANGE %s", to_string().c_str());
    
    if(!set_dirs())
      return false;

    // last_rs.data
    Files::RsDataPtr rs_data = Env::RsData::get();
    Files::RsDataPtr rs_last = get_last_rs();

    if(rs_last->endpoints.size() > 0) {
      HT_DEBUGF("RS-LAST=%s RS-NEW=%s", 
                rs_last->to_string().c_str(), rs_data->to_string().c_str());
      if(!has_endpoint(rs_data->endpoints, rs_last->endpoints)){
        client::ClientConPtr old_conn 
          = Env::Clients::get()->rs_service->get_connection(
              rs_last->endpoints, std::chrono::milliseconds(10000), 1);
        // if online, means rs-mngr had comm issues with the RS-LAST, 
        //   req.unload (sync)
        if(old_conn != nullptr)
          Protocol::Req::UnloadRange(old_conn, RangeBase::shared());
      }
    }
    if(!rs_data->set_rs(get_path(rs_data_file)))
      return false;
    

    // range.data
    if(!Files::RangeData::load(get_path(range_data_file), cellstores)) {
      Files::RangeData::load_by_path(get_path("cs"), cellstores);
      Files::RangeData::save(get_path(range_data_file), cellstores);
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
      HT_DEBUGF("LOADED RANGE %s", to_string().c_str());
      return true;
    }
    
    HT_WARNF("LOAD RANGE FAILED %s", to_string().c_str());
    return false;
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

    Files::RangeData::save(get_path(range_data_file), cellstores);

  }

  void unload(bool completely){

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
    int err = Error::OK;
    if(completely)
      Env::FsInterface::fs()->remove(err, get_path(rs_data_file));

    set_state(State::NOTLOADED);

    HT_DEBUGF("UNLOADED RANGE cid=%d rid=%d", cid, rid);
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

  bool set_dirs(){
    int err = Error::OK;
    if(!Env::FsInterface::fs()->exists(err, get_path(""))){
      Env::FsInterface::fs()->mkdirs(err, get_path("log"));
      Env::FsInterface::fs()->mkdirs(err, get_path("cs"));
    } 
    return err == Error::OK;
  }
  
  private:

  State               m_state;
  
  Types::Range        m_type;
  Cells::IntervalsPtr intervals;
  Files::CellStores   cellstores;

};

typedef std::shared_ptr<Range> RangePtr;


}}}
#endif