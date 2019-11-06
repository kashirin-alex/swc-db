/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_db_Protocol_handlers_NotImplemented_h
#define swc_db_Protocol_handlers_NotImplemented_h


namespace SWC { namespace Protocol {  namespace Common { namespace Handler {


class NotImplemented : public AppHandler {
  public:

  NotImplemented(ConnHandlerPtr conn, Event::Ptr ev)
                : AppHandler(conn, ev){
  }

  void run() override {
    try {
      auto cbp = CommBuf::make(4);
      cbp->header.initialize_from_request_header(m_ev->header);
      cbp->append_i32(Error::NOT_IMPLEMENTED);
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  }

};
  

}}}}

#endif // swc_db_Protocol_handlers_NotImplemented_h