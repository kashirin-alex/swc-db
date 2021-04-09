/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/IoContext.h"

namespace SWC { namespace Comm {

PeriodicTimer::PeriodicTimer(const Config::Property::V_GINT32::Ptr cfg_ms,
                             PeriodicTimer::Call_t&& call,
                             const IoContextPtr& ioctx)
                            : asio::high_resolution_timer(ioctx->executor()),
                              m_ms(cfg_ms), m_call(std::move(call)) {
  schedule();
}

PeriodicTimer::~PeriodicTimer() { }

void PeriodicTimer::schedule() {
  expires_after(std::chrono::milliseconds(m_ms->get()));
  async_wait(
    [this](const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted) {
        m_call();
        schedule();
      }
    }
  );
}


PeriodicTimers::~PeriodicTimers() { }

void PeriodicTimers::stop() {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto& ptr : *this)
    ptr->cancel();
}

void PeriodicTimers::set(const Config::Property::V_GINT32::Ptr ms,
                         PeriodicTimer::Call_t&& call,
                         const IoContextPtr& ioctx) {
  Core::MutexSptd::scope lock(m_mutex);
  emplace_back(new PeriodicTimer(ms, std::move(call), ioctx));
}


}}
