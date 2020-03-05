
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_PeriodicTimer_h
#define swc_core_comm_PeriodicTimer_h

#include <asio.hpp>
#include "swcdb/core/config/Settings.h"

namespace SWC {
  
class PeriodicTimer final {
  public:
  typedef const std::function<void()> Call_t; 

  PeriodicTimer(const Property::V_GINT32::Ptr cfg_ms, 
                Call_t call, asio::io_context* io);
  
  ~PeriodicTimer();

  private:

  void schedule();

  const Property::V_GINT32::Ptr   m_ms;
  Call_t                          m_call;
  asio::high_resolution_timer     m_timer; 
};


class PeriodicTimers : public std::vector<PeriodicTimer*> {
  public:
  
  void stop();

  void set(const Property::V_GINT32::Ptr ms, PeriodicTimer::Call_t call, 
           asio::io_context* io);

  private:
  std::mutex m_mutex;
};

} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/PeriodicTimer.cc"
#endif 

#endif // swc_core_comm_PeriodicTimer_h