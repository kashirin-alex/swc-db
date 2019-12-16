
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_rgr_req_RangeLoad_h
#define swc_lib_db_protocol_rgr_req_RangeLoad_h

#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeLoad : public Common::Req::ConnQueue::ReqBase {
  public:

  RangeLoad(server::Mngr::Ranger::Ptr rgr, server::Mngr::Range::Ptr range) 
            : Common::Req::ConnQueue::ReqBase(false), 
              rgr(rgr), range(range) {
    auto schema = Env::Schemas::get()->get(range->cfg->cid);
    schema_revision = schema->revision;
    cbp = CommBuf::make(
      Params::RangeLoad(range->cfg->cid, range->rid, schema)
    );
    cbp->header.set(RANGE_LOAD, 60000);
  }
  
  virtual ~RangeLoad() { }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
      
    if(was_called)
      return;
    was_called = true;

    if(!valid() || ev->type == Event::Type::DISCONNECT) {
      handle_no_conn();
      return;
    }

    if(ev->header.command == RANGE_LOAD){
      int err = ev->error != Error::OK? ev->error: ev->response_code();
      if(err != Error::OK){
        loaded(err, false, DB::Cells::Interval()); 
        return; 
      }
      
      const uint8_t *ptr = ev->data.base+4;
      size_t remain = ev->data.size-4;
      Params::RangeLoaded params;
      params.decode(&ptr, &remain);
      loaded(err, false, params.interval); 
    }
  }

  bool valid() override {
    return !range->deleted();
  }
  
  void handle_no_conn() override {
    loaded(Error::COMM_NOT_CONNECTED, true, DB::Cells::Interval());
  }

  void loaded(int err, bool failure, const DB::Cells::Interval& intval);


  private:

  server::Mngr::Ranger::Ptr rgr;
  server::Mngr::Range::Ptr  range;
  int64_t                   schema_revision;
   
};

}}}}

#endif // swc_lib_db_protocol_rgr_req_RangeLoad_h
