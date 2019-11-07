/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Seek_h
#define swc_app_fsbroker_handlers_Seek_h

#include "swcdb/lib/fs/Broker/Protocol/params/Seek.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Seek : public AppHandler {
  public:

  Seek(ConnHandlerPtr conn, Event::Ptr ev)
       : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    size_t offset = 0;

    try {

      const uint8_t *ptr = m_ev->data.base;
      size_t remain = m_ev->data.size;

      FS::Protocol::Params::SeekReq params;
      params.decode(&ptr, &remain);

      FS::SmartFd::Ptr smartfd = Env::Fds::get()->select(params.get_fd());
      if(smartfd == nullptr)
        err = EBADR;
      else {
        Env::FsInterface::fs()->seek(err, smartfd, params.get_offset());
        offset = smartfd->pos();
      }
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
  
    try {
      auto cbp = CommBuf::make(FS::Protocol::Params::SeekRsp(offset), 4);
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

#endif // swc_app_fsbroker_handlers_Seek_h