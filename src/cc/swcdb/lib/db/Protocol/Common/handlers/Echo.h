/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_db_Protocol_handlers_Echo_h
#define swc_db_Protocol_handlers_Echo_h



namespace SWC { namespace Protocol {  namespace Common { namespace Handler {


class Echo : public AppHandler {
  public:

  Echo(ConnHandlerPtr conn, Event::Ptr ev)
       : AppHandler(conn, ev){}

  void run() override {
    try {

      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBuf::Ptr cbp;

      if(m_ev->data.size > 0) {
        StaticBuffer buf = StaticBuffer((void*)m_ev->data.base, m_ev->data.size);
        cbp = CommBuf::make(header, buf);
      } else 
        cbp = CommBuf::make(header);
      
      m_conn->send_response(cbp);

    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_db_Protocol_handlers_Echo_h