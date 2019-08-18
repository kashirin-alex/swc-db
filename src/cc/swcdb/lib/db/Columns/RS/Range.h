/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RS_Range_h
#define swcdb_lib_db_Columns_RS_Range_h

#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Protocol/req/UnloadRange.h"

#include "swcdb/lib/db/Files/RangeData.h"


namespace SWC { namespace server { namespace RS {

class Range : public DB::RangeBase {
  public:
  
  enum State{
    NOTLOADED,
    LOADED,
    DELETED
  };

  Range(int64_t cid, int64_t rid)
        : RangeBase(cid, rid), m_state(State::NOTLOADED)
        { }

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
    HT_DEBUGF("LOADING RANGE cid=%d rid=%d", cid, rid);
    
    if(!set_dirs())
      return false;

    Files::RsDataPtr rs_data = EnvRsData::get();
    Files::RsDataPtr rs_last = get_last_rs();

    if(rs_last->endpoints.size() > 0) {
      // if online, (means rs-mngr had comm issues with the RS-LAST )
      //   req. unload (sync) 
      std::cout << " RS-LAST=" << rs_last->to_string() << "\n"
                << " RS-NEW =" << rs_data->to_string() << "\n";
      if(!has_endpoint(rs_data->endpoints, rs_last->endpoints)){
        client::ClientConPtr old_conn = EnvClients::get()->rs_service->get_connection(
          rs_last->endpoints, std::chrono::milliseconds(10000), 1);
        if(old_conn != nullptr)
          Protocol::Req::UnloadRange(old_conn, RangeBase::shared());
      }
    }

    if(!EnvRsData::get()->set_rs(get_path(rs_data_file)))
      return false;
    
    m_rdata = Files::RangeData::get_data(cid, get_path(""));

    // m_rdata->cellstores
    // CommitLogs
    
    set_state(State::LOADED);



    std::cout << to_string() << "\n";;

    if(is_loaded()) {
      HT_DEBUGF("LOADED RANGE cid=%d rid=%d", cid, rid);
      return true;
    }
    
    HT_WARNF("LOAD RANGE FAILED cid=%d rid=%d", cid, rid);
    return false;
  }

  void unload(bool completely){

    // CommitLogs  
    // CellStores
    // range.data
    
    m_rdata->cellstores.clear();
    // TEST-DATA
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(1));
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(2));
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(3));
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(4));
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(5));
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
    for(auto cs : m_rdata->cellstores){
      size_t remain2 = remain;
      const uint8_t * base2 = base;
      *ptr_muta = *(uint8_t*)std::to_string(cs->cs_id).c_str();
      std::cout << "decoding remain=" << remain2 << " bufp=" << std::string((char*)base2, remain2) << "\n";
      cs->intervals->decode(&base2, &remain2);
    }
    delete [] d;

    std::cout << to_string() << "\n";;

    m_rdata->save();

    int err = Error::OK;
    if(completely)
      EnvFsInterface::fs()->remove(err, get_path(rs_data_file));
    set_state(State::NOTLOADED);

    HT_DEBUGF("UNLOADED RANGE cid=%d rid=%d", cid, rid);
  }

  std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("[");
    s.append(DB::RangeBase::to_string());
    s.append(", state=");
    s.append(std::to_string(m_state));
    if(m_rdata != nullptr) {
      s.append(", ");
      s.append(m_rdata->to_string());
    }
    s.append("]");
    return s;
  }

  private:

  bool set_dirs(){
    int err = Error::OK;
    if(!EnvFsInterface::fs()->exists(err, get_path(""))){
      EnvFsInterface::fs()->mkdirs(err, get_path("log"));
      EnvFsInterface::fs()->mkdirs(err, get_path("cs"));
    } 
    return err == Error::OK;
  }
  
  private:

  State               m_state;
  Files::RangeDataPtr m_rdata = nullptr;

};

typedef std::shared_ptr<Range> RangePtr;


}}}
#endif