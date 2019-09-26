/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_db_Protocol_handlers_NotImplemented_h
#define swc_db_Protocol_handlers_NotImplemented_h


namespace SWC { namespace Protocol {  namespace Common { namespace Handler {


class NotImplemented : public AppHandler {
  public:

  NotImplemented(ConnHandlerPtr conn, EventPtr ev)
       : AppHandler(conn, ev){}

  void run() override {
    try {

      CommHeader header;
      header.initialize_from_request_header(m_ev->header);      
      CommBufPtr cbp = std::make_shared<CommBuf>(header, 4);
      cbp->append_i32(Error::NOT_IMPLEMENTED);

      m_conn->send_response(cbp);
    } catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_db_Protocol_handlers_NotImplemented_h