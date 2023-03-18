/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/IoContext.h"

namespace SWC { namespace Comm {


IoContext::IoContext(std::string&& _name, int32_t size)
                    : running(true), name(std::move(_name)),
                      pool(size), signals(nullptr),
                      m_size(size), m_periodic_timers() {
  SWC_LOGF(LOG_DEBUG, "Starting IO-ctx(%s) size=%d", name.c_str(), m_size);
  SWC_ASSERT(m_size > 0);
}

void IoContext::set_signals() {
  signals.reset(new asio::signal_set(executor() , SIGINT, SIGTERM));
}

void IoContext::set_periodic_timer(
      const Config::Property::Value_int32_g::Ptr ms,
      PeriodicTimer::Call_t&& call) {
  m_periodic_timers.set(ms, std::move(call), shared_from_this());
}

void IoContext::stop() {
  SWC_LOGF(LOG_DEBUG, "Stopping IO-ctx(%s)", name.c_str());

  m_periodic_timers.stop();

  running.store(false);

  // hold on for IO to finish
  bool untracked;
  for(int i=0; i<10; ++i) {
    untracked = asio::query(executor(), asio::execution::outstanding_work)
      == asio::execution::outstanding_work.untracked;
    if(untracked)
      break;
    SWC_LOGF(LOG_DEBUG, "Waiting for IO-ctx(%s)", name.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  pool.stop();
  pool.wait();

  SWC_LOGF(LOG_DEBUG, "Wait for IO-ctx(%s) finished %sgracefully",
           name.c_str(), untracked ? "" : "not-");
}


}}
