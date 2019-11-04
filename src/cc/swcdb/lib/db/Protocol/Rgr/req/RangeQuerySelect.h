
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_rgr_req_RangeQuerySelect_h
#define swc_lib_db_protocol_rgr_req_RangeQuerySelect_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "../params/RangeQuerySelect.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

  
class RangeQuerySelect: public Common::Req::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(Common::Req::ConnQueue::ReqBase::Ptr, 
                             const Params::RangeQuerySelectRsp&)> Cb_t;
  typedef std::function<void()> Cb_no_conn_t;

  static inline void 
  request(const Params::RangeQuerySelectReq& params,
          const EndPoints& endpoints, Cb_no_conn_t cb_no_conn, const Cb_t cb,
          const uint32_t timeout = 10000){
    std::make_shared<RangeQuerySelect>(
      params, endpoints, cb_no_conn, cb, timeout)
      ->run();
  }
  
  EndPoints     endpoints;

  RangeQuerySelect(const Params::RangeQuerySelectReq& params,
                   const EndPoints& endpoints, 
                   Cb_no_conn_t cb_no_conn, const Cb_t cb, 
                   const uint32_t timeout) 
                  : Common::Req::ConnQueue::ReqBase(false), 
                    endpoints(endpoints), cb_no_conn(cb_no_conn), cb(cb) {
    CommHeader header(RANGE_QUERY_SELECT, timeout);
    cbp = CommBuf::make(header, params);
  }

  virtual ~RangeQuerySelect(){}

  void handle_no_conn() override {
    cb_no_conn();
  }

  bool run(uint32_t timeout=0) override {
    Env::Clients::get()->rgr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr &ev) override {
    
    //std::cout << "RangeQuerySelectRsp " << ev->to_str() << "\n";
    if(ev->type == Event::Type::DISCONNECT){
      handle_no_conn();
      return;
    }

    Params::RangeQuerySelectRsp rsp_params;
    if(ev->type == Event::Type::ERROR){
      rsp_params.err = ev->error;
      cb(req(), rsp_params);
      return;
    }

    try{
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      rsp_params.decode(&ptr, &remain);
      rsp_params.bufp = ptr;
      
      if(rsp_params.size 
        && !checksum_i32_chk(
          rsp_params.checksum , rsp_params.bufp, rsp_params.size))
        rsp_params.err = Error::CHECKSUM_MISMATCH;

      if(remain != rsp_params.size) {
        std::cerr << "RangeQuerySelect remain=" << remain << " rsp_params.size=" << rsp_params.size << "\n";
        exit(1);
      }
      DB::Cells::Cell cell;
      while(remain) {
        cell.read(&ptr, &remain);
        if(cell.flag == DB::Cells::NONE) {
          std::cerr << "RangeQuerySelect remain=" << remain << " FLAG::NONE " << cell.to_string() << "\n";
          exit(1);
        }
      }
    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      rsp_params.err = e.code();
    }
    cb(req(), rsp_params);
  }

  private:

  Cb_no_conn_t  cb_no_conn;
  const Cb_t    cb;
};


}}}}

#endif // swc_lib_db_protocol_req_RangeLocate_h
