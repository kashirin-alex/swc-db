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
    e.__set_message(msg);
    throw e;
  }

  void select_sql(Cells& _return, const std::string& sql) {
    
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

    bool only_keys = req->specs.flags.is_only_keys();
    DB::Schema::Ptr schema = 0;
    DB::Cells::Vector vec; 
    for(auto cid : req->result->get_cids()) {
      schema = Env::Clients::get()->schemas->get(err, cid);
      vec.free();
      req->result->get_cells(cid, vec);
      for(auto& dbcell : vec.cells) {
        _return.push_back(Cell());
        Cell& cell = _return.back();
        
        dbcell->key.convert_to(cell.k);
        cell.ts = dbcell->timestamp;
        if(cell.__isset.v = !only_keys) {
          if(dbcell->vlen)
            cell.v = std::string((const char*)dbcell->value, dbcell->vlen);
        }
      }
    }

  }


};




}}

#endif // swc_app_thriftbroker_AppHandler_h