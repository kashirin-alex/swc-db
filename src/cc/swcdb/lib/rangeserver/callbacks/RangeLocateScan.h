/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_rangeserver_callbacks_RangeLocateScan_h
#define swc_lib_rangeserver_callbacks_RangeLocateScan_h

#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/db/Protocol/params/RsRangeLoad.h"

namespace SWC {
namespace server {
namespace RS {
namespace Callback {


class RangeLocateScan : public ResponseCallback {
  public:

  RangeLocateScan(ConnHandlerPtr conn, EventPtr ev, const int64_t cid_scanned)
                  : ResponseCallback(conn, ev), cid_scanned(cid_scanned) {
  }

  virtual ~RangeLocateScan() { }

  void response(int &err) override {

    if(err == Error::OK && Env::RsData::is_shuttingdown()) 
      err = Error::SERVER_SHUTTING_DOWN;

    Protocol::Params::RsRangeLocateRsp params(err);
    if(err == Error::OK){
      if(cid_scanned == 1)
        params.cid = 2;
      else if(cid_scanned == 2)
        params.cid = 3; //  (result[0].key-fraction[0])
      
      params.rid = 3; //    (result[0].value-fraction[0])
      // params.next_key =  // (result[1].key)
    }


    try {
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(
        header, params.encoded_length());
      params.encode(cbp->get_data_ptr_address());

      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
    
  }

  private:
  const int64_t cid_scanned;

};
typedef std::shared_ptr<RangeLocateScan> RangeLocateScanPtr;


}
}}}
#endif // swc_lib_rangeserver_callbacks_RangeLocateScan_h
