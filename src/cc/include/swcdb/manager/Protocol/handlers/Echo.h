/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_Echo_h
#define swcdb_manager_Protocol_handlers_Echo_h



namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {

void do_echo(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  try {
    auto cbp = ev->data_ext.size 
                ? Buffers::make(ev->data_ext) 
                : Buffers::make();
    cbp->header.initialize_from_request_header(ev->header);
    conn->send_response(cbp);
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}
  

}}}}}

#endif // swcdb_manager_Protocol_handlers_Echo_h
