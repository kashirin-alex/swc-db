/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RS_Range_h
#define swcdb_lib_db_Columns_RS_Range_h

#include <random>

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

  inline static const std::string range_data_file = "range.data";

  Range(int64_t cid, int64_t rid, SchemaPtr schema): 
        RangeBase(cid, rid), m_state(State::NOTLOADED), 
        m_schema(schema) { }

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
    
    // range.data (range_data_file: cells-interval > CS#)
    m_rdata = Files::RangeData::get_data(get_path(range_data_file));

    std::cout << m_rdata->to_string() << "\n";

    // m_rdata->cellstores
    // CommitLogs
    
    /* 
    std::mt19937_64 eng{std::random_device{}()};  // or seed however you want
    std::uniform_int_distribution<> dist{1000, 5000};
    std::this_thread::sleep_for(std::chrono::milliseconds{dist(eng)});
    */
    set_state(State::LOADED);


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
    
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(1));
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(2));
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(3));
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(4));
    m_rdata->cellstores.push_back(std::make_shared<Files::CellStore>(5));
    uint8_t* d = new uint8_t[Serialization::encoded_length_vi32(28)+28+Serialization::encoded_length_vi32(6)+6];
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
    
    size_t remain = d-base;
    for(auto cs : m_rdata->cellstores){
      size_t remain2 = remain;
      const uint8_t * base2 = base;
      *ptr_muta = (uint8_t)cs->cs_id;
      std::cout << "decoding remain=" << remain2 << " bufp=" << std::string((char*)base2, remain2) << "\n";
      cs->interval->decode(&base2, &remain2);
    }
    delete [] d;
    std::cout << "decoded \n";

    std::cout << m_rdata->to_string() << "\n";
    std::cout << "to_string \n";

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
    s.append("]");
    return s;
  }

  private:

  bool set_dirs(){
    int err = Error::OK;
    if(!EnvFsInterface::fs()->exists(err, get_path(""))){
      // init - range's work directories
      // EnvFsInterface::fs()->mkdirs(err, get_path("rs"));
      EnvFsInterface::fs()->mkdirs(err, get_path("log"));
      EnvFsInterface::fs()->mkdirs(err, get_path("cs"));
    } 
    return err == Error::OK;
  }
  
  private:
  SchemaPtr   m_schema;
  State       m_state;

  Files::RangeDataPtr m_rdata;
};

typedef std::shared_ptr<Range> RangePtr;


}}}
#endif