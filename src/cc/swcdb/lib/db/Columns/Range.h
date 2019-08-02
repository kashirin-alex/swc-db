/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Range_h
#define swcdb_lib_db_Columns_Range_h

#include <random>

#include "RangeBase.h"
#include "swcdb/lib/db/Protocol/req/UnloadRange.h"



namespace SWC { namespace DB {

class Range : public RangeBase {
  public:
  
  inline static const std::string range_data_file = "range.data";

  Range(int64_t cid, int64_t rid, SchemaPtr schema): 
        RangeBase(cid, rid),
        m_schema(schema) { }

  virtual ~Range(){}
  
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
    // CellStores
    // CommitLogs
    
    std::mt19937_64 eng{std::random_device{}()};  // or seed however you want
    std::uniform_int_distribution<> dist{1000, 5000};
    std::this_thread::sleep_for(std::chrono::milliseconds{dist(eng)});

    set_loaded(RangeState::LOADED);


    if(is_loaded()) {
      HT_DEBUGF("LOADED RANGE cid=%d rid=%d", cid, rid);
      return true;
    }
    
    HT_WARNF("LOAD RANGE FAILED cid=%d rid=%d", cid, rid);
    return false;
  }

  void unload(){

    // CommitLogs  
    // CellStores
    // range.data


    int err = Error::OK;
    EnvFsInterface::fs()->remove(err, get_path(rs_data_file));
    set_loaded(RangeState::NOTLOADED);

    HT_DEBUGF("UNLOADED RANGE cid=%d rid=%d", cid, rid);
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
  SchemaPtr         m_schema;
};

typedef std::shared_ptr<Range> RangePtr;


}}
#endif