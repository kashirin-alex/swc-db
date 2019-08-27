/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Mkdirs_h
#define swc_app_fsbroker_handlers_Mkdirs_h

#include "swcdb/lib/fs/Broker/Protocol/params/Mkdirs.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Mkdirs : public AppHandler {
  public:

  Mkdirs(ConnHandlerPtr conn, EventPtr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      FS::Protocol::Params::MkdirsReq params;
      params.decode(&ptr, &remain);

      Env::FsInterface::fs()->mkdirs(err, params.get_dirname());
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
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

#endif // swc_app_fsbroker_handlers_Mkdirs_h