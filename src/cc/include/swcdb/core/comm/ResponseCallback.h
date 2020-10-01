/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_ResponseCallback_h
#define swcdb_core_comm_ResponseCallback_h

#include "swcdb/core/comm/ConnHandler.h"


namespace SWC { namespace Comm {

class ResponseCallback: public std::enable_shared_from_this<ResponseCallback> {

  public:

  typedef std::shared_ptr<ResponseCallback> Ptr;

  ResponseCallback(const ConnHandlerPtr& conn, const Event::Ptr& ev)
                  : m_conn(conn), m_ev(ev) {
  }
    
  virtual ~ResponseCallback() { };

  virtual void run();

  virtual void response(int& err);

  virtual void response_ok();

  virtual void send_error(int code, std::string msg);
  
  protected:
  ConnHandlerPtr m_conn;
  Event::Ptr     m_ev;

};


}} // namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ResponseCallback.cc"
#endif 

#endif // swcdb_core_comm_ResponseCallback_h
