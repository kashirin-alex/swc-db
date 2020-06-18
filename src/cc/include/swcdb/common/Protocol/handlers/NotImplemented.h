/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_db_Protocol_handlers_NotImplemented_h
#define swc_db_Protocol_handlers_NotImplemented_h


namespace SWC { namespace Protocol {  namespace Common { namespace Handler {

void not_implemented(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
    try {
      auto cbp = CommBuf::make(4);
      cbp->header.initialize_from_request_header(ev->header);
      cbp->append_i32(Error::NOT_IMPLEMENTED);
      conn->send_response(cbp);
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
}


class NotImplemented : public AppHandler {
  public:

  NotImplemented(const ConnHandlerPtr& conn, const Event::Ptr& ev)
                : AppHandler(conn, ev){
  }

  void run() override {
    not_implemented(m_conn, m_ev);
  }

};
  

}}}}

#endif // swc_db_Protocol_handlers_NotImplemented_h