/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Close_h
#define swc_app_fsbroker_handlers_Close_h

#include "swcdb/lib/fs/Broker/Protocol/params/Close.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Close : public AppHandler {
  public:

  Close(ConnHandlerPtr conn, EventPtr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    
    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      FS::Protocol::Params::CloseReq params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);

      FS::SmartFdPtr smartfd = EnvFds::get()->remove(params.get_fd());
      if(smartfd == nullptr)
        err = EBADR;
      else
        EnvFsInterface::fs()->close(err, smartfd);
    }
    catch (Exception &e) {
      err = e.code();
      HT_ERROR_OUT << e << HT_END;
    }

    try {
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(header, 4);
      cbp->append_i32(err);
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Close_h