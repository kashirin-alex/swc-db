/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_Protocol_handlers_Echo_h
#define swc_manager_Protocol_handlers_Echo_h



namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {

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
  

}}}}

#endif // swc_manager_Protocol_handlers_Echo_h