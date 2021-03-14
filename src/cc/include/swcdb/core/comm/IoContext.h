
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_IoContext_h
#define swcdb_core_comm_IoContext_h


#include "swcdb/core/Logger.h"
#include "swcdb/core/comm/asio_wrap.h"


namespace SWC { namespace Comm {
// Forward Declaration
class IoContext;
typedef std::shared_ptr<IoContext>  IoContextPtr;
}}

#include "swcdb/core/comm/PeriodicTimer.h"



namespace SWC {


/**
 * @brief The SWC-DB Communications C++ namespace 'SWC::Comm'
 *
 * \ingroup Core
 */
namespace Comm {


class IoContext final : public std::enable_shared_from_this<IoContext> {
  public:

  typedef asio::thread_pool::executor_type    Executor;
  typedef asio::executor_work_guard<Executor> ExecutorWorkGuard;

  static IoContextPtr make(const std::string& name, int32_t size);


  Core::AtomicBool                     running;
  const std::string                    name;
  asio::thread_pool                    pool;
  std::unique_ptr<asio::signal_set>    signals;


  IoContext(const std::string& name, int32_t size);

  ~IoContext();

  int32_t get_size() const noexcept;

  Executor executor() noexcept;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
  template <typename T_Handler>
  SWC_CAN_INLINE
  void post(T_Handler&& handler) {
    asio::post(pool, handler);
  }
#pragma GCC diagnostic pop

  void set_signals();

  void set_periodic_timer(const Config::Property::V_GINT32::Ptr ms,
                          const PeriodicTimer::Call_t& call);

  void stop();

  private:
  int32_t             m_size;
  PeriodicTimers      m_periodic_timers;


};

} //namespace Comm



namespace Env {

class IoCtx final {
  public:

  static void init(int32_t size);

  static bool ok() noexcept;

  static Comm::IoContextPtr io();

  template <typename T_Handler>
  SWC_CAN_INLINE
  static void post(T_Handler&& handler)  {
    m_env->m_io->post(handler);
  }

  static bool stopping() noexcept;

  static void reset() noexcept;

  IoCtx(int32_t size);

  ~IoCtx();

  private:
  Comm::IoContextPtr                    m_io;
  inline static std::shared_ptr<IoCtx>  m_env = nullptr;
};

} // namespace Env


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/IoContext.cc"
#include "swcdb/core/comm/PeriodicTimer.cc"
#endif

#endif // swcdb_core_comm_IoContext_h
