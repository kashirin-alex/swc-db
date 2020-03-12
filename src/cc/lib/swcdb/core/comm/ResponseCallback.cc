/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/comm/ResponseCallback.h"


namespace SWC {

ResponseCallback::ResponseCallback(ConnHandlerPtr conn, Event::Ptr ev)
                                   : m_conn(conn), m_ev(ev) { }
    
ResponseCallback::~ResponseCallback() { }

void ResponseCallback::run() {}

void ResponseCallback::response(int& err) {
  m_conn->send_error(err , "", m_ev);
}

void ResponseCallback::response_ok()  {
  //SWC_LOGF(LOG_DEBUG, "response_ok, %s", m_ev->to_str().c_str());
   m_conn->response_ok(m_ev);
}

void ResponseCallback::send_error(int code, std::string msg) {
  //SWC_LOGF(LOG_DEBUG, "send_error, %s", m_ev->to_str().c_str());
  m_conn->send_error(code , msg, m_ev);
}


}