
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_rgr_req_RangeUnload_h
#define swc_lib_db_protocol_rgr_req_RangeUnload_h

#include "../../Common/params/ColRangeId.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeUnload : public Common::Req::ConnQueue::ReqBase {
  public:

  RangeUnload(DB::RangeBasePtr range, ResponseCallbackPtr cb,
                uint32_t timeout=60000) 
               : Common::Req::ConnQueue::ReqBase(false), 
                range(range), cb(cb) {
    Common::Params::ColRangeId params(range->cid, range->rid);
    CommHeader header(RANGE_UNLOAD, timeout);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }

  virtual ~RangeUnload() { }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
      
    if(was_called)
      return;
    was_called = true;

    if(!valid())
      unloaded(Error::RS_DELETED_RANGE, cb); 
    else if(ev->type == Event::Type::DISCONNECT
            || ev->header.command == RANGE_UNLOAD){
      unloaded(Error::OK, cb); 
    }
  }

  bool valid() override;
  
  void handle_no_conn() override {
    unloaded(Error::OK, cb); 
  }

  void unloaded(int err, ResponseCallbackPtr cb);


  private:

  ResponseCallbackPtr   cb;
  DB::RangeBasePtr      range;
   
};

}}}}

#endif // swc_lib_db_protocol_rgr_req_RangeUnload_h
