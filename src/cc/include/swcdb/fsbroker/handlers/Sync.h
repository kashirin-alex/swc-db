/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Sync_h
#define swc_app_fsbroker_handlers_Sync_h

#include "swcdb/fs/Broker/Protocol/params/Sync.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Sync : public AppHandler {
  public:

  Sync(ConnHandlerPtr conn, Event::Ptr ev)
       : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    
    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      FS::Protocol::Params::SyncReq params;
      params.decode(&ptr, &remain);

      auto smartfd = Env::Fds::get()->select(params.fd);
      
      if(smartfd == nullptr)
        err = EBADR;
      else
        Env::FsInterface::fs()->sync(err, smartfd);
    }
    catch (Exception &e) {
      err = e.code();
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }

    try {
      auto cbp = CommBuf::make(4);
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

#endif // swc_app_fsbroker_handlers_Sync_h