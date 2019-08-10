/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Append_h
#define swc_app_fsbroker_handlers_Append_h

#include "swcdb/lib/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Append : public AppHandler {
  public:

  Append(ConnHandlerPtr conn, EventPtr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    size_t amount = 0;
    size_t offset = 0;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      FS::Protocol::Params::AppendReq params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);

      if (remain != params.get_size()) {
        err = Error::SERIALIZATION_INPUT_OVERRUN;

      } else { 
        FS::SmartFdPtr smartfd = EnvFds::get()->select(params.get_fd());
        if(smartfd == nullptr)
          err = EBADR;
        else {
          offset = smartfd->pos();
          StaticBuffer buffer((uint8_t*)ptr, params.get_size(), false);
          amount = EnvFsInterface::fs()->append(
            err, smartfd, buffer, (FS::Flags)params.get_flags());
        }
      }
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
  
    try {
      FS::Protocol::Params::AppendRsp rsp_params(offset, amount);
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(header, 
                            4+rsp_params.encoded_length());
      cbp->append_i32(err);
      rsp_params.encode(cbp->get_data_ptr_address());

      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }

    // add to fds-map
  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Append_h