
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/PeriodicTimer.h"

namespace SWC {
  
PeriodicTimer::PeriodicTimer(const Property::V_GINT32::Ptr cfg_ms, 
                             Call_t call, asio::io_context* io)
                             : m_ms(cfg_ms), m_call(call), m_timer(*io) {
  schedule();
}
  
PeriodicTimer::~PeriodicTimer() {
  m_timer.cancel(); 
}

void PeriodicTimer::schedule() {
  m_timer.expires_from_now(std::chrono::milliseconds(m_ms->get()));
  m_timer.async_wait(
    [this](const asio::error_code ec) {
      if(ec != asio::error::operation_aborted) {
        m_call();
        schedule();
      }
    }
  );
}


PeriodicTimers::~PeriodicTimers() { }

void PeriodicTimers::stop() {
  Mutex::scope lock(m_mutex);
  for(auto it = begin(); it<end();) {
    delete *it;
    erase(it);
  }
}

void PeriodicTimers::set(const Property::V_GINT32::Ptr ms, 
                         PeriodicTimer::Call_t call,
                         asio::io_context* io) {
  Mutex::scope lock(m_mutex);
  push_back(new PeriodicTimer(ms, call, io));
}


}