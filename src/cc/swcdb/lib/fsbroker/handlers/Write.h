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

      FS::SmartFdPtr smartfd 
        = FS::SmartFd::make_ptr(params.get_name(), params.get_flags());
 
      StaticBuffer buffer((uint8_t*)ptr, params.get_size(), false);
      Env::FsInterface::fs()->write(
        err, smartfd,
        params.get_replication(), params.get_block_size(),
        buffer
      );

    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
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

#endif // swc_app_fsbroker_handlers_Write_h