/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_manager_Protocol_handlers_Echo_h
#define swc_manager_Protocol_handlers_Echo_h



namespace SWC { namespace Protocol { namespace Mngr { namespace Handler {

void do_echo(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev) {
  try {
    auto cbp = ev->data_ext.size ? 
                  Comm::Buffers::make(ev->data_ext) 
                : Comm::Buffers::make();
    cbp->header.initialize_from_request_header(ev->header);
    conn->send_response(cbp);
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}
  

}}}}

#endif // swc_manager_Protocol_handlers_Echo_h