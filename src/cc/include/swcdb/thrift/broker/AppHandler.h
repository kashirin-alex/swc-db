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

  void sql_list_columns(Schemas& _return, const std::string& sql) {

    int err = Error::OK;
    std::vector<DB::Schema::Ptr> dbschemas;  
    std::string message;
    client::SQL::parse_list_columns(err, sql, dbschemas, message);
    if(err) 
      exception(err, message);

    if(dbschemas.empty()) { // get all schema
      std::promise<int> res;
      Protocol::Mngr::Req::ColumnList::request(
        [&dbschemas, await=&res]
        (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, int error, 
         Protocol::Mngr::Params::ColumnListRsp rsp) {
          if(!error)
            dbschemas = rsp.schemas;
          await->set_value(error);
        },
        300000
      );
      if(err = res.get_future().get()) {
        message.append(Error::get_text(err));
        message.append("\n");
        exception(err, message);
      }
    }
    
    _return.resize(dbschemas.size());
    uint32_t c = 0;
    for(auto& dbschema : dbschemas) {
      auto& schema = _return[c++];
      schema.__set_cid(dbschema->cid);
      schema.__set_col_name(dbschema->col_name);
      schema.__set_col_type((ColumnType::type)dbschema->col_type);

      schema.__set_cell_versions(dbschema->cell_versions);
      schema.__set_cell_ttl(dbschema->cell_ttl);

      schema.__set_blk_replication(dbschema->blk_replication);
      schema.__set_blk_encoding((EncodingType::type)dbschema->blk_encoding);
      schema.__set_blk_size(dbschema->blk_size);
      schema.__set_blk_cells(dbschema->blk_cells);

      schema.__set_cs_size(dbschema->cs_size);
      schema.__set_cs_max(dbschema->cs_max);
      schema.__set_compact_percent(dbschema->compact_percent);

      schema.__set_revision(dbschema->revision);
    }
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