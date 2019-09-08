
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_RsUnloadRange_h
#define swc_lib_db_protocol_req_RsUnloadRange_h

#include "swcdb/lib/db/Protocol/params/ColRangeId.h"

namespace SWC {
namespace Protocol {
namespace Req {


class RsUnloadRange : public ConnQueue::ReqBase {
  public:

  RsUnloadRange(DB::RangeBasePtr range, ResponseCallbackPtr cb,
                uint32_t timeout=60000) 
               : ConnQueue::ReqBase(false), range(range), cb(cb) {

    Params::ColRangeId params = Params::ColRangeId(range->cid, range->rid);
    CommHeader header(Protocol::Command::REQ_RS_UNLOAD_RANGE, timeout);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }

  virtual ~RsUnloadRange() { }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
      
    if(was_called)
      return;
    was_called = true;

    if(!valid())
      unloaded(Error::RS_DELETED_RANGE, cb); 
    else if(ev->type == Event::Type::DISCONNECT
            || ev->header.command == Command::REQ_RS_UNLOAD_RANGE){
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

}}}

#endif // swc_lib_db_protocol_req_RsUnloadRange_h
