/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_db_Protocol_handlers_Echo_h
#define swc_db_Protocol_handlers_Echo_h



namespace SWC { namespace Protocol {  namespace Common { namespace Handler {

void do_echo(ConnHandlerPtr conn, Event::Ptr ev) {
    try {
      auto cbp = ev->data_ext.size ? 
                    CommBuf::make(ev->data_ext) 
                  : CommBuf::make();
      cbp->header.initialize_from_request_header(ev->header);
      conn->send_response(cbp);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
}

class Echo : public AppHandler {
  public:

  Echo(ConnHandlerPtr conn, Event::Ptr ev)
       : AppHandler(conn, ev){
  }

  void run() override {
    do_echo(m_conn, m_ev);
  }

};
  

}}}}

#endif // swc_db_Protocol_handlers_Echo_h