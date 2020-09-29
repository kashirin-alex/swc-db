
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_comm_PeriodicTimer_h
#define swc_core_comm_PeriodicTimer_h

#include <asio.hpp>
#include "swcdb/core/config/Settings.h"

namespace SWC {
  
class PeriodicTimer final : private asio::high_resolution_timer {
  public:
  typedef const std::function<void()> Call_t; 

  using asio::high_resolution_timer::cancel;

  PeriodicTimer(const Config::Property::V_GINT32::Ptr cfg_ms, 
                const Call_t& call, asio::io_context* io);
  
  ~PeriodicTimer();

  private:

  void schedule();

  const Config::Property::V_GINT32::Ptr   m_ms;
  const Call_t                            m_call;
};


class PeriodicTimers final 
    : private std::vector<std::unique_ptr<PeriodicTimer>> {
  public:

  ~PeriodicTimers();
  
  void stop();

  void set(const Config::Property::V_GINT32::Ptr ms,
           const PeriodicTimer::Call_t& call, 
           asio::io_context* io);

  private:
  Mutex m_mutex;
};

} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/PeriodicTimer.cc"
#endif 

#endif // swc_core_comm_PeriodicTimer_h