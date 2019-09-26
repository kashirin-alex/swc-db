
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_RsRangeLoad_h
#define swc_lib_db_protocol_req_RsRangeLoad_h

#include "swcdb/lib/db/Protocol/params/RsRangeLoad.h"

namespace SWC { namespace Protocol { namespace Req {


class RsRangeLoad : public ConnQueue::ReqBase {
  public:

  RsRangeLoad(server::Mngr::RsStatusPtr rs, server::Mngr::RangePtr range) 
              : ConnQueue::ReqBase(false), rs(rs), range(range), 
                schema(Env::Schemas::get()->get(range->cid)) {
    int err = Error::OK;
    if(!Env::MngrColumns::get()->get_column(err, range->cid, false)
                           ->need_schema_sync(rs->rs_id, schema->revision))
      schema = nullptr;                     

    Params::RsRangeLoad params(range->cid, range->rid, schema);
    CommHeader header(Rgr::RANGE_LOAD, 60000);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }
  
  virtual ~RsRangeLoad() { }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
      
    if(was_called)
      return;
    was_called = true;

    if(!valid() || ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == Rgr::RANGE_LOAD){
      int err = ev->error != Error::OK? ev->error: response_code(ev);
      if(err != Error::OK){
        loaded(err, false, nullptr); 
        return; 
      }
      
      const uint8_t *ptr = ev->payload+4;
      size_t remain = ev->payload_len-4;
      Params::RsRangeLoaded params;
      params.decode(&ptr, &remain);
      loaded(err, false, params.intervals); 
    }
  }

  bool valid() override {
    return !range->deleted();
  }
  
  void handle_no_conn() override {
    loaded(Error::COMM_NOT_CONNECTED, true, nullptr);
  }

  void loaded(int err, bool failure, DB::Cells::Intervals::Ptr intvals);


  private:

  server::Mngr::RsStatusPtr rs;
  server::Mngr::RangePtr    range;

  DB::SchemaPtr schema; 
   
};

}}}

#endif // swc_lib_db_protocol_req_RsRangeLoad_h
