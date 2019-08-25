/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_fsbroker_handlers_Open_h
#define swc_app_fsbroker_handlers_Open_h

#include "swcdb/lib/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace server { namespace FsBroker {

namespace Handler {


class Open : public AppHandler {
  public:

  Open(ConnHandlerPtr conn, EventPtr ev)
         : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    int32_t fd = -1;

    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      FS::Protocol::Params::OpenReq params;
      params.decode(&ptr, &remain);

      FS::SmartFdPtr smartfd 
        = FS::SmartFd::make_ptr(params.get_name(), params.get_flags());
 
      EnvFsInterface::fs()->open(err, smartfd, params.get_buffer_size());
      
      if(smartfd->valid() && err==Error::OK)
        fd = EnvFds::get()->add(smartfd);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
  
    try {
      FS::Protocol::Params::OpenRsp rsp_params(fd);
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

#endif // swc_app_fsbroker_handlers_Open_h