/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Length_h
#define swc_app_fsbroker_handlers_Length_h

#include "swcdb/lib/fs/Broker/Protocol/params/Length.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Length : public AppHandler {
  public:

  Length(ConnHandlerPtr conn, Event::Ptr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    size_t length = 0;

    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      FS::Protocol::Params::LengthReq params;
      params.decode(&ptr, &remain);

      length = Env::FsInterface::fs()->length(err, params.fname);
      
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
      err = e.code();
    }
  
    try {
      auto cbp = CommBuf::make(FS::Protocol::Params::LengthRsp(length), 4);
      cbp->header.initialize_from_request_header(m_ev->header);
      cbp->append_i32(err);
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Length_h