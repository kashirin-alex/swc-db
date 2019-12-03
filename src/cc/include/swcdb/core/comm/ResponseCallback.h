/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_ResponseCallback_h
#define swc_core_comm_ResponseCallback_h

#include "swcdb/core/comm/ConnHandler.h"


namespace SWC {

class ResponseCallback: public std::enable_shared_from_this<ResponseCallback> {

  public:

  typedef std::shared_ptr<ResponseCallback> Ptr;

  ResponseCallback(ConnHandlerPtr conn, Event::Ptr ev);
    
  virtual ~ResponseCallback();

  virtual void run();

  virtual void response(int& err);

  virtual void response_ok();

  virtual void send_error(int code, std::string msg);
  
  protected:
  ConnHandlerPtr m_conn;
  Event::Ptr     m_ev;
};


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "../../../../lib/swcdb/core/comm/ResponseCallback.cc"
#endif 

#endif // swc_core_comm_ResponseCallback_h
