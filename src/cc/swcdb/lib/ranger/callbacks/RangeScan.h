/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangesScan_h
#define swc_lib_ranger_callbacks_RangesScan_h

#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/db/Cells/ReqScan.h"
//#include "swcdb/lib/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeScan : public ResponseCallback {
  public:

  RangeScan(ConnHandlerPtr conn, EventPtr ev) : ResponseCallback(conn, ev) {
  }
  
  virtual ~RangeScan() { }

  void response(int &err) override {

  
    try {
      /*
      Protocol::Rgr::Params::RangeLoaded params(range->get_interval());
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(
        header, 4+params.encoded_length());
      cbp->append_i32(err);
      params.encode(cbp->get_data_ptr_address());

      m_conn->send_response(cbp);
      return;
      */
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = Error::COMM_SEND_ERROR;
      
    }
    
  }

  DB::Cells::ReqScan::Ptr    req;
};


}
}}}
#endif // swc_lib_ranger_callbacks_RangesScan_h
