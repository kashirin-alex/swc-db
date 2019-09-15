/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_ResponseCallback_h
#define swc_core_comm_ResponseCallback_h

#include "Event.h"


namespace SWC {

class ResponseCallback: public std::enable_shared_from_this<ResponseCallback> {

  public:
  ResponseCallback(ConnHandlerPtr conn, EventPtr ev): m_conn(conn), m_ev(ev){}
    
  virtual ~ResponseCallback() { }


  virtual void run() {}

  virtual void response(int& err) {
    m_conn->send_error(err , "", m_ev);
  }

  virtual void response_ok()  {
    //HT_DEBUGF("response_ok, %s", m_ev->to_str().c_str());
    m_conn->response_ok(m_ev);
  }

  virtual void send_error(int code, std::string msg) {
    //HT_DEBUGF("send_error, %s", m_ev->to_str().c_str());
    m_conn->send_error(code , msg, m_ev);
  }

  
  protected:
  ConnHandlerPtr m_conn;
  EventPtr m_ev;
};

typedef std::shared_ptr<ResponseCallback> ResponseCallbackPtr;


}
#endif // swc_core_comm_ResponseCallback_h
