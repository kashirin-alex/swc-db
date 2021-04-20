/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/IoContext.h"

namespace SWC {

namespace Comm {

IoContextPtr IoContext::make(std::string&& _name, int32_t size) {
  return std::make_shared<IoContext>(std::move(_name), size);
}

IoContext::IoContext(std::string&& _name, int32_t size)
                    : running(true), name(std::move(_name)),
                      pool(size), m_size(size) {
  SWC_LOGF(LOG_DEBUG, "Starting IO-ctx(%s) size=%d", name.c_str(), m_size);
  SWC_ASSERT(m_size > 0);
}

int32_t IoContext::get_size() const noexcept {
  return m_size; // asio::query(executor(), asio::execution::occupancy);
}

SWC_SHOULD_INLINE
IoContext::Executor IoContext::executor() noexcept {
  return pool.get_executor();
}

void IoContext::set_signals() {
  signals.reset(new asio::signal_set(executor() , SIGINT, SIGTERM));
}

void IoContext::set_periodic_timer(const Config::Property::V_GINT32::Ptr ms,
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

  SWC_LOGF(LOG_DEBUG, "Wait for IO-ctx(%s) finished %sgracefully",
           name.c_str(), untracked ? "" : "not-");

  //if(!untracked)
  //  pool.join();
  pool.stop();
}


} // namespace Comm



namespace Env {

void IoCtx::init(int32_t size) {
  m_env = std::make_shared<IoCtx>(size);
}

bool IoCtx::ok() noexcept {
  return bool(m_env);
}

SWC_SHOULD_INLINE
Comm::IoContextPtr IoCtx::io() {
  SWC_ASSERT(ok());
  return m_env->m_io;
}

SWC_SHOULD_INLINE
bool IoCtx::stopping() noexcept {
  return !m_env->m_io->running;
}

void IoCtx::reset() noexcept {
  m_env = nullptr;
}

IoCtx::IoCtx(int32_t size)
            : m_io(std::make_shared<Comm::IoContext>("Env", size)) {
}


} // namespace Env



} // namespace SWC
