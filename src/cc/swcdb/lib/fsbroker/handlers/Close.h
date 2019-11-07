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

  Close(ConnHandlerPtr conn, Event::Ptr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    
    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      FS::Protocol::Params::CloseReq params;
      params.decode(&ptr, &remain);

      FS::SmartFd::Ptr smartfd = Env::Fds::get()->remove(params.get_fd());
      if(smartfd == nullptr)
        err = EBADR;
      else
        Env::FsInterface::fs()->close(err, smartfd);
    }
    catch (Exception &e) {
      err = e.code();
      HT_ERROR_OUT << e << HT_END;
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

#endif // swc_app_fsbroker_handlers_Close_h