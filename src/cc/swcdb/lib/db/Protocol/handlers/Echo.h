/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_db_Protocol_handlers_Echo_h
#define swc_db_Protocol_handlers_Echo_h



namespace SWC { namespace server {  namespace common { 

namespace Handler {


class Echo : public AppHandler {
  public:

  Echo(ConnHandlerPtr conn, EventPtr ev)
       : AppHandler(conn, ev){}

  void run() override {
    try {

      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp;

      if(m_ev->payload_len > 0) {
        StaticBuffer buf = StaticBuffer((void*)m_ev->payload, m_ev->payload_len);

        cbp = std::make_shared<CommBuf>(header, 0, buf);
      } else 
        cbp = std::make_shared<CommBuf>(header);
      
      m_conn->send_response(cbp);

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_db_Protocol_handlers_Echo_h