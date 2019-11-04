/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Remove_h
#define swc_app_fsbroker_handlers_Remove_h

#include "swcdb/lib/fs/Broker/Protocol/params/Remove.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Remove : public AppHandler {
  public:

  Remove(ConnHandlerPtr conn, Event::Ptr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    
    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      FS::Protocol::Params::RemoveReq params;
      params.decode(&ptr, &remain);

      Env::FsInterface::fs()->remove(err, params.get_fname());
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

#endif // swc_app_fsbroker_handlers_Remove_h