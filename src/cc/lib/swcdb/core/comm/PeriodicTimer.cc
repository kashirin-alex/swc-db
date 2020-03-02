
/*
 * Copyright (C) 2020 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/core/comm/PeriodicTimer.h"

namespace SWC {
  
PeriodicTimer::PeriodicTimer(const gInt32tPtr cfg_ms, Call_t call, 
                             asio::io_context* io)
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


void PeriodicTimers::stop() {
  std::scoped_lock lock(m_mutex);
  for(auto it = begin(); it<end();) {
    delete *it;
    erase(it);
  }
}

void PeriodicTimers::set(const gInt32tPtr ms, PeriodicTimer::Call_t call,
                         asio::io_context* io) {
  std::scoped_lock lock(m_mutex);
  push_back(new PeriodicTimer(ms, call, io));
}


}