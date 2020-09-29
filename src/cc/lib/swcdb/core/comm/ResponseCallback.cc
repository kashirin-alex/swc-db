/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/ResponseCallback.h"


namespace SWC { namespace Comm {


void ResponseCallback::run() { }

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


}}
