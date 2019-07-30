/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_ResponseCallback_h
#define swc_core_comm_ResponseCallback_h
#include <memory>

#include "Event.h"
#include "AppHandler.h"


namespace SWC {

// forward declarations
class ConnHandler;
typedef ConnHandler* ConnHandlerPtr;


class ResponseCallback : public AppHandler {
  public:

  ResponseCallback(ConnHandlerPtr conn, EventPtr ev)
              : AppHandler(conn, ev) { }

  virtual ~ResponseCallback() { return; }

  virtual void run() override {

  }

  virtual void response_ok() {
    //HT_DEBUGF("response_ok, %s", m_ev->to_str().c_str());
    m_conn->response_ok(m_ev);
  }

  virtual void send_error(int code, std::string msg) {
    //HT_DEBUGF("send_error, %s", m_ev->to_str().c_str());
    m_conn->send_error(code , msg, m_ev);
  }

  

  

};

typedef std::shared_ptr<ResponseCallback> ResponseCallbackPtr;


}
#endif // swc_core_comm_ResponseCallback_h
