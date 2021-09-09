/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_PeriodicTimer_h
#define swcdb_core_comm_PeriodicTimer_h


#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/asio_wrap.h"


namespace SWC { namespace Comm {

class PeriodicTimer final {
  public:
  typedef const std::function<void()> Call_t;

  PeriodicTimer(const Config::Property::V_GINT32::Ptr cfg_ms,
                Call_t&& call, const IoContextPtr& ioctx);

  ~PeriodicTimer();

  void cancel();

  private:

  void schedule();

  const Config::Property::V_GINT32::Ptr   m_ms;
  const Call_t                            m_call;
  Core::MutexAtomic                       m_mutex;
  asio::high_resolution_timer             m_timer;
};



class PeriodicTimers final
    : private Core::Vector<std::unique_ptr<PeriodicTimer>> {
  public:

  ~PeriodicTimers();

  void stop();

  void set(const Config::Property::V_GINT32::Ptr ms,
           PeriodicTimer::Call_t&& call,
           const IoContextPtr& ioctx);

  private:
  Core::MutexSptd m_mutex;
};


}} // namespace SWC::Comm



#endif // swcdb_core_comm_PeriodicTimer_h
