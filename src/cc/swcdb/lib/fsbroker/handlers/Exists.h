/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Exists_h
#define swc_app_fsbroker_handlers_Exists_h

#include "swcdb/lib/fs/Broker/Protocol/params/Exists.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Exists : public AppHandler {
  public:

  Exists(ConnHandlerPtr conn, EventPtr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      FS::Protocol::Params::ExistsReq params;
      const uint8_t *base = ptr;
      params.decode(&ptr, &remain);

      int err = Error::OK;
      bool exists = EnvFsInterface::fs()->exists(err, params.get_fname());

      FS::Protocol::Params::ExistsRsp rsp_params(exists);
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
      m_conn->send_error(e.code(), e.what(), m_ev);
    }
  
  }

};
  

}}}}

#endif // swc_app_fsbroker_handlers_Exists_h