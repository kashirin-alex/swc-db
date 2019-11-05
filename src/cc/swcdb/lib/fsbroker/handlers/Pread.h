/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Pread_h
#define swc_app_fsbroker_handlers_Pread_h

#include "swcdb/lib/fs/Broker/Protocol/params/Pread.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Pread : public AppHandler {
  public:

  Pread(ConnHandlerPtr conn, Event::Ptr ev)
       : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    size_t offset = 0;
    StaticBuffer rbuf;

    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      FS::Protocol::Params::PreadReq params;
      params.decode(&ptr, &remain);

      FS::SmartFdPtr smartfd = Env::Fds::get()->select(params.get_fd());
      if(smartfd == nullptr)
        err = EBADR;
      else {
        offset = params.get_offset();
        rbuf.reallocate(params.get_amount());
        rbuf.size = Env::FsInterface::fs()->pread(
          err, smartfd, params.get_offset(), rbuf.base, params.get_amount());
      }
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
  
    try {
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      auto cbp = CommBuf::make(
        header, 
        FS::Protocol::Params::ReadRsp(offset), 
        rbuf, 
        4
      );
      cbp->append_i32(err);
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }

  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Pread_h