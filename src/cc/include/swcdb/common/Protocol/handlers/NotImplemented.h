/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_Protocol_handlers_NotImplemented_h
#define swcdb_common_Protocol_handlers_NotImplemented_h



namespace SWC { namespace Comm { namespace Protocol {
namespace Common { namespace Handler {


void not_implemented(const ConnHandlerPtr& conn, 
                     const Event::Ptr& ev) {
  auto cbp = Buffers::make(4);
  cbp->append_i32(Error::NOT_IMPLEMENTED);
  conn->send_response(cbp, ev);
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
  


}}}}}


#endif // swcdb_common_Protocol_handlers_NotImplemented_h
