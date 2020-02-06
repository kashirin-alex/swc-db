/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_thriftbroker_AppHandler_h
#define swc_app_thriftbroker_AppHandler_h

#include "swcdb/client/sql/SQL.h"

namespace SWC { 
namespace thrift = apache::thrift;
namespace Thrift {


class AppHandler : virtual public BrokerIf {
  public:

  virtual ~AppHandler() { }

  void exception(int err, const std::string& msg) {
    Exception e;
    e.__set_code(err);
    e.__set_message(msg.empty() ? Error::get_text(err) : msg);
    SWC_LOG_OUT(LOG_DEBUG);
    e.printTo(std::cout);
    SWC_LOG_OUT_END;
    throw e;
  }

  void sql_select_list(Cells& _return, const std::string& sql) {
    
    auto req = std::make_shared<Protocol::Common::Req::Query::Select>();
    int err = Error::OK;
    std::string message;
    uint8_t display_flags = 0;
    client::SQL::parse_select(err, sql, req->specs, display_flags, message);
    if(err) 
      exception(err, message);
    
    req->scan();
    req->wait();
    
    if(err) 
      exception(err, message);

    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err, message);
  }

  static void process_results(
          int& err, Protocol::Common::Req::Query::Select::Result::Ptr result,
          bool with_value, Cells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Vector vec; 
    for(auto cid : result->get_cids()) {
      vec.free();
      result->get_cells(cid, vec);

      schema = Env::Clients::get()->schemas->get(err, cid);
      _return.resize(vec.cells.size());

      uint32_t c = 0;
      for(auto& dbcell : vec.cells) {
        auto& cell = _return[c++];
        
        cell.c = schema->col_name;
        dbcell->key.convert_to(cell.k);
        cell.ts = dbcell->timestamp;
        if(cell.__isset.v = with_value) {
          if(dbcell->vlen)
            cell.v = std::string((const char*)dbcell->value, dbcell->vlen);
        }
      }
    }                            
  }

  void sql_select_map(ColumnsMapCells& _return, const std::string& sql) {
    auto req = std::make_shared<Protocol::Common::Req::Query::Select>();
    int err = Error::OK;
    std::string message;
    uint8_t display_flags = 0;
    client::SQL::parse_select(err, sql, req->specs, display_flags, message);
    if(err) 
      exception(err, message);
    
    req->scan();
    req->wait();
    
    if(err) 
      exception(err, message);

    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err, message);
  }

  static void process_results(
          int& err, Protocol::Common::Req::Query::Select::Result::Ptr result,
          bool with_value, ColumnsMapCells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Vector vec; 
    for(auto cid : result->get_cids()) {
      vec.free();
      result->get_cells(cid, vec); 

      schema = Env::Clients::get()->schemas->get(err, cid);
      if(err)
        return;
      auto& list = _return[schema->col_name];     
      list.resize(vec.cells.size());

      uint32_t c = 0;
      for(auto& dbcell : vec.cells) {
        auto& cell = list[c++];
        
        dbcell->key.convert_to(cell.k);
        cell.ts = dbcell->timestamp;
        if(cell.__isset.v = with_value) {
          if(dbcell->vlen)
            cell.v = std::string((const char*)dbcell->value, dbcell->vlen);
        }
      }
    }
  }

  void sql_select_keys(KeysCells& _return, const std::string& sql) {
    auto req = std::make_shared<Protocol::Common::Req::Query::Select>();
    int err = Error::OK;
    std::string message;
    uint8_t display_flags = 0;
    client::SQL::parse_select(err, sql, req->specs, display_flags, message);
    if(err) 
      exception(err, message);
    
    req->scan();
    req->wait();
    
    if(err) 
      exception(err, message);

    process_results(
      err, 
      req->result, 
      !req->specs.flags.is_only_keys() && !req->specs.flags.is_only_deletes(),
      _return
    );
    if(err) 
      exception(err, message);
  }

  static void process_results(
          int& err, Protocol::Common::Req::Query::Select::Result::Ptr result,
          bool with_value, KeysCells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Vector vec; 
    for(auto cid : result->get_cids()) {
      vec.free();
      result->get_cells(cid, vec); 

      schema = Env::Clients::get()->schemas->get(err, cid);

      if(err)
        return;
      for(auto& dbcell : vec.cells) {
        auto it = std::find_if(_return.begin(), _return.end(), 
                              [dbcell](const KeyCells& key_cells)
                              {return dbcell->key.equal(key_cells.k);});
        if(it == _return.end()) {
          _return.emplace_back();
          it = _return.end()-1;
          dbcell->key.convert_to(it->k);
        }
        
        auto& cell = it->cells.emplace_back();
        cell.c = schema->col_name;
        cell.ts = dbcell->timestamp;
        if(cell.__isset.v = with_value) {
          if(dbcell->vlen)
            cell.v = std::string((const char*)dbcell->value, dbcell->vlen);
        }
      }
    }
  }


};




}}

#endif // swc_app_thriftbroker_AppHandler_h