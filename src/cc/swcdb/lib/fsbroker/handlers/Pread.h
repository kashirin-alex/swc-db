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

  Pread(ConnHandlerPtr conn, EventPtr ev)
       : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    size_t amount = 0;
    size_t offset = 0;
    StaticBuffer rbuf;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      FS::Protocol::Params::PreadReq params;
      params.decode(&ptr, &remain);

      FS::SmartFdPtr smartfd = EnvFds::get()->select(params.get_fd());
      if(smartfd == nullptr)
        err = EBADR;
      else {
        StaticBuffer buf = StaticBuffer(params.get_amount());
        rbuf = buf;
        amount = EnvFsInterface::fs()->pread(
          err, smartfd, params.get_offset(), rbuf.base, params.get_amount());
        offset = smartfd->pos()-amount;
      }
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
  
    try {
      FS::Protocol::Params::ReadRsp rsp_params(offset, amount);
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(header, 
                            4+rsp_params.encoded_length(), rbuf);
      cbp->append_i32(err);
      rsp_params.encode(cbp->get_data_ptr_address());

      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }

  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Pread_h