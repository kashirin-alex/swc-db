/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/ResponseCallback.h"


namespace SWC { namespace Comm {


void ResponseCallback::run() { }

bool ResponseCallback::expired(int64_t within) const {
  return  (m_ev && m_ev->expired(within)) || 
          (m_conn && !m_conn->is_open());
}

void ResponseCallback::response(int& err) {
  err ? send_error(err, "") : response_ok();
}

void ResponseCallback::response_ok()  {
  m_conn->response_ok(m_ev);
}

void ResponseCallback::send_error(int err, const std::string& msg) {
  m_conn->send_error(err , msg, m_ev);
}


}}
