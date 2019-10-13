/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangeQueryUpdate_h
#define swc_lib_ranger_callbacks_RangeQueryUpdate_h

#include "swcdb/lib/core/comm/ResponseCallback.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeQueryUpdate : public ResponseCallback {
  public:

  RangeQueryUpdate(ConnHandlerPtr conn, EventPtr ev)
                  : ResponseCallback(conn, ev) {
  }

  virtual ~RangeQueryUpdate() { }

  void response(int &err) override {

    Protocol::Rgr::Params::RangeQueryUpdateRsp params(err);
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

};
typedef std::shared_ptr<RangeQueryUpdate> RangeQueryUpdatePtr;


}
}}}
#endif // swc_lib_ranger_callbacks_RangeQueryUpdate_h
