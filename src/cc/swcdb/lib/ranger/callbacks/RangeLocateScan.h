/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangeLocateScan_h
#define swc_lib_ranger_callbacks_RangeLocateScan_h

#include "swcdb/lib/core/comm/ResponseCallback.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeLocateScan : public ResponseCallback {
  public:

  RangeLocateScan(ConnHandlerPtr conn, EventPtr ev, const int64_t cid_scanned)
                  : ResponseCallback(conn, ev), cid_scanned(cid_scanned) {
  }

  virtual ~RangeLocateScan() { }

  void response(int &err) override {

    if(err == Error::OK && Env::RgrData::is_shuttingdown()) 
      err = Error::SERVER_SHUTTING_DOWN;

    Protocol::Rgr::Params::RangeLocateRsp params(err);
    if(err == Error::OK && req->cells->size() > 0) {

      DB::Cells::Cell cell;
      req->cells->get(0, cell);
      std::string id_name(cell.key.get_string(0));
      params.cid = (int64_t)strtoll(id_name.c_str(), NULL, 0);
      
      id_name = std::string((char *)cell.value, cell.vlen);
      params.rid = (int64_t)strtoll(id_name.c_str(), NULL, 0);

      if(req->cells->size() > 1) {
        req->cells->get(1, cell);
        params.next_key.set(cell.key, Condition::GE);
      }
    }

    std::cout << "RangeLocateScan, rsp " << req->to_string() << "\n";

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


  DB::Cells::ReqScan::Ptr     req;

  private:
  const int64_t               cid_scanned;

};
typedef std::shared_ptr<RangeLocateScan> RangeLocateScanPtr;


}
}}}
#endif // swc_lib_ranger_callbacks_RangeLocateScan_h
