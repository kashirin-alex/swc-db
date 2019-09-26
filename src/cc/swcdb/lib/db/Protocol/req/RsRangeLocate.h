
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_RsRangeLocate_h
#define swc_lib_db_protocol_req_RsRangeLocate_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "../params/RsRangeLocate.h"


namespace SWC {
namespace Protocol {
namespace Req {

  
class RsRangeLocate: public ConnQueue::ReqBase {
  public:
  
  typedef std::function<
          void(ConnQueue::ReqBase::Ptr, Params::RsRangeLocateRsp)> Cb_t;
  typedef std::function<void()> Cb_no_conn_t;

  static void request(int64_t cid, int64_t rid, 
                      const DB::Specs::Interval::Ptr interval, 
                      const EndPoints& endpoints, Cb_no_conn_t cb_no_conn, 
                      const Cb_t cb, const uint32_t timeout = 10000){
    request(
      Protocol::Params::RsRangeLocateReq(cid, rid, interval), 
      endpoints, cb_no_conn, cb, timeout
    );
  }

  static inline void request(const Protocol::Params::RsRangeLocateReq params,
                             const EndPoints& endpoints, Cb_no_conn_t cb_no_conn, 
                             const Cb_t cb, const uint32_t timeout = 10000){
    std::make_shared<RsRangeLocate>(params, endpoints, cb_no_conn, cb, timeout)
      ->run();
  }


  RsRangeLocate(const Protocol::Params::RsRangeLocateReq params, 
                const EndPoints& endpoints, Cb_no_conn_t cb_no_conn, 
                const Cb_t cb, const uint32_t timeout) 
              : ConnQueue::ReqBase(false), 
                endpoints(endpoints), cb_no_conn(cb_no_conn), cb(cb) {
    CommHeader header(Protocol::Command::RANGE_LOCATE, timeout);
    cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    params.encode(cbp->get_data_ptr_address());
  }

  virtual ~RsRangeLocate(){}

  void handle_no_conn() override {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    cb_no_conn();
  }

  bool run(uint32_t timeout=0) override {
    Env::Clients::get()->rs->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {

    if(ev->type == Event::Type::DISCONNECT){
      handle_no_conn();
      return;
    }

    Protocol::Params::RsRangeLocateRsp rsp_params;
    if(ev->type == Event::Type::ERROR){
      rsp_params.err = ev->error;
      cb(req(), rsp_params);
      return;
    }

    try{
      const uint8_t *ptr = ev->payload;
      size_t remain = ev->payload_len;
      rsp_params.decode(&ptr, &remain);
    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      rsp_params.err = e.code();
    }
    cb(req(), rsp_params);
  }

  private:

  EndPoints     endpoints;
  Cb_no_conn_t  cb_no_conn;
  const Cb_t    cb;
};


}}}

#endif // swc_lib_db_protocol_req_RsRangeLocate_h
