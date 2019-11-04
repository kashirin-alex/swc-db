/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Flush_h
#define swc_app_fsbroker_handlers_Flush_h

#include "swcdb/lib/fs/Broker/Protocol/params/Flush.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Flush : public AppHandler {
  public:

  Flush(ConnHandlerPtr conn, Event::Ptr ev)
        : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    
    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      FS::Protocol::Params::FlushReq params;
      params.decode(&ptr, &remain);

      FS::SmartFdPtr smartfd = Env::Fds::get()->select(params.get_fd());
      if(smartfd == nullptr)
        err = EBADR;
      else
        Env::FsInterface::fs()->flush(err, smartfd);
    }
    catch (Exception &e) {
      err = e.code();
      HT_ERROR_OUT << e << HT_END;
    }

    try {
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      auto cbp = CommBuf::make(header, 4);
      cbp->append_i32(err);
      cbp->finalize_data();
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Flush_h