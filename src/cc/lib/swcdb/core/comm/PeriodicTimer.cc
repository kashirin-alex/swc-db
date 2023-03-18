/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/IoContext.h"

namespace SWC { namespace Comm {

PeriodicTimer::PeriodicTimer(
      const Config::Property::Value_int32_g::Ptr cfg_ms,
      PeriodicTimer::Call_t&& call,
      const IoContextPtr& ioctx)
      : m_ms(cfg_ms), m_call(std::move(call)),
        m_mutex(), m_timer(ioctx->executor()) {
  schedule();
}

PeriodicTimer::~PeriodicTimer() noexcept {
  m_timer.cancel();
}

void PeriodicTimer::schedule() {
  Core::MutexAtomic::scope lock(m_mutex);
  m_timer.expires_after(std::chrono::milliseconds(m_ms->get()));
  struct TimerTask {
    PeriodicTimer* tm;
    SWC_CAN_INLINE
    TimerTask(PeriodicTimer* a_tm) noexcept : tm(a_tm) { }
    void operator()(const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted) {
        tm->m_call();
        tm->schedule();
      }
    }
  };
  m_timer.async_wait(TimerTask(this));
}

void PeriodicTimer::cancel() {
  Core::MutexAtomic::scope lock(m_mutex);
  m_timer.cancel();
}



PeriodicTimers::~PeriodicTimers() noexcept { }

void PeriodicTimers::stop() {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto& tm : *this)
    tm->cancel();
  clear();
}

void PeriodicTimers::set(const Config::Property::Value_int32_g::Ptr ms,
                         PeriodicTimer::Call_t&& call,
                         const IoContextPtr& ioctx) {
  Core::MutexSptd::scope lock(m_mutex);
  emplace_back(new PeriodicTimer(ms, std::move(call), ioctx));
}


}}
