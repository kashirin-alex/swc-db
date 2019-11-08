/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Write_h
#define swc_app_fsbroker_handlers_Write_h

#include "swcdb/lib/fs/Broker/Protocol/params/Write.h"

namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Write : public AppHandler {
  public:

  Write(ConnHandlerPtr conn, Event::Ptr ev)
        : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;

    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      FS::Protocol::Params::WriteReq params;
      params.decode(&ptr, &remain);

      auto smartfd = FS::SmartFd::make_ptr(params.fname, params.flags);
      
      Env::FsInterface::fs()->write(
        err, smartfd, params.replication, params.blksz, m_ev->data_ext
      );

    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
  
    try {
      auto cbp = CommBuf::make(4);
      cbp->header.initialize_from_request_header(m_ev->header);
      cbp->append_i32(err);
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Write_h