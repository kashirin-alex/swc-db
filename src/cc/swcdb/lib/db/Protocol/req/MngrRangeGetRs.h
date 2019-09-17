
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_MngrRangeGetRs_h
#define swc_lib_db_protocol_req_MngrRangeGetRs_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "MngrMngrActive.h"
#include "../params/MngrRangeGetRs.h"


namespace SWC {
namespace Protocol {
namespace Req {

  
class MngrRangeGetRs: public ConnQueue::ReqBase {
  public:
  
  typedef std::function<
          void(ConnQueue::ReqBase::Ptr, int, Params::MngrRangeGetRsRsp)> Cb_t;

  static void request(int64_t cid, int64_t rid, 
                      const Cb_t cb, const uint32_t timeout = 10000){
    std::make_shared<MngrRangeGetRs>(
      Protocol::Params::MngrRangeGetRsReq(cid, rid), cb, timeout
    )->run();
  }
  static void request(int64_t cid, Cells::Intervals::Ptr intervals, 
                      const Cb_t cb, const uint32_t timeout = 10000){
    std::make_shared<MngrRangeGetRs>(
      Protocol::Params::MngrRangeGetRsReq(cid, intervals), cb, timeout
    )->run();
  }


  MngrRangeGetRs(const Protocol::Params::MngrRangeGetRsReq params, const Cb_t cb, 
      const uint32_t timeout) : ConnQueue::ReqBase(false), cb(cb), cid(params.cid) {

    CommHeader header(Protocol::Command::CLIENT_REQ_GET_RANGE_RS, timeout);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }

  virtual ~MngrRangeGetRs(){}

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()){
      // columns-get (can be any mngr) -- if col_name (cid=1)
      Env::Clients::get()->mngrs_groups->select(cid, endpoints); 
      if(endpoints.empty()){
        std::make_shared<MngrMngrActive>(cid, shared_from_this())->run();
        return false;
      }
    } 
    Env::Clients::get()->mngr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {

    if(ev->type == Event::Type::DISCONNECT){
      handle_no_conn();
      return;
    }

    Protocol::Params::MngrRangeGetRsRsp rsp_params;
    int err = ev->error != Error::OK? ev->error: Protocol::response_code(ev);

    if(err == Error::OK){
      try{
        const uint8_t *ptr = ev->payload+4;
        size_t remain = ev->payload_len-4;
        rsp_params.decode(&ptr, &remain);
      } catch (Exception &e) {
        HT_ERROR_OUT << e << HT_END;
        err = e.code();
      }
    }

    cb(req(), err, rsp_params);
  }

  private:
  
  void clear_endpoints() {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    endpoints.clear();
  }

  const Cb_t  cb;
  EndPoints   endpoints;
  int64_t     cid;
};


}}}

#endif // swc_lib_db_protocol_req_MngrRangeGetRs_h
