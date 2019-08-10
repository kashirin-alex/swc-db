/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Length_h
#define swc_app_fsbroker_handlers_Length_h

#include "swcdb/lib/fs/Broker/Protocol/params/Length.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Length : public AppHandler {
  public:

  Length(ConnHandlerPtr conn, EventPtr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    size_t length = 0;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      FS::Protocol::Params::LengthReq params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);

      length = EnvFsInterface::fs()->length(err, params.get_fname());
      
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
  
    try {
      FS::Protocol::Params::LengthRsp rsp_params(length);
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
  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Length_h