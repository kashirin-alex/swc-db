/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Readdir_h
#define swc_app_fsbroker_handlers_Readdir_h

#include "swcdb/lib/fs/Broker/Protocol/params/Readdir.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Readdir : public AppHandler {
  public:

  Readdir(ConnHandlerPtr conn, EventPtr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    FS::DirentList results;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      FS::Protocol::Params::ReaddirReq params;
      params.decode(&ptr, &remain);

      Env::FsInterface::fs()->readdir(err, params.get_dirname(), results);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
    
    try {
      FS::Protocol::Params::ReaddirRsp rsp_params(results);
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

#endif // swc_app_fsbroker_handlers_Readdir_h