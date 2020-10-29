/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_Echo_h
#define swcdb_manager_Protocol_handlers_Echo_h



namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {

void do_echo(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  conn->send_response(
    ev->data_ext.size 
      ? Buffers::make(ev->data_ext)
      : Buffers::make(), 
    ev
  );
}
  

}}}}}

#endif // swcdb_manager_Protocol_handlers_Echo_h
