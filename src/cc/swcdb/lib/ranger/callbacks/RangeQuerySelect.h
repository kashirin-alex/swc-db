/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangeQuerySelect_h
#define swc_lib_ranger_callbacks_RangeQuerySelect_h

#include "swcdb/lib/core/comm/ResponseCallback.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeQuerySelect : public ResponseCallback {
  public:

  RangeQuerySelect(ConnHandlerPtr conn, EventPtr ev, Range::Ptr range)
                  : ResponseCallback(conn, ev), range(range) {
  }

  virtual ~RangeQuerySelect() { }

  void response(int &err) override {

    if(err == Error::OK && Env::RgrData::is_shuttingdown()) 
      err = Error::SERVER_SHUTTING_DOWN;

    DynamicBuffer buffer;
    Protocol::Rgr::Params::RangeQuerySelectRsp params(err);
    if(req->cells->size() > 0) {
      req->cells->write(buffer);
      params.reached_limit = req->limit_buffer_sz <= req->cells->size_bytes();
    }
    params.size = buffer.fill();

    std::cout << "RangeQuerySelect, rsp " << req->to_string() << "\n";
    req = nullptr;
    std::cout << params.to_string() << "\n";

    try {
      StaticBuffer sndbuf(buffer);
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(
        header, params.encoded_length(), sndbuf);
      params.encode(cbp->get_data_ptr_address());

      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
    
  }


  DB::Cells::ReqScan::Ptr req;
  Range::Ptr              range;

};
typedef std::shared_ptr<RangeQuerySelect> RangeQuerySelectPtr;


}
}}}
#endif // swc_lib_ranger_callbacks_RangeLocateScan_h
