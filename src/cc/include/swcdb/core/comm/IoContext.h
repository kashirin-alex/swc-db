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

  static IoContextPtr make(std::string&& _name, int32_t size) {
    return IoContextPtr(new IoContext(std::move(_name), size));
  }

  SWC_CAN_INLINE
  static uint32_t
  get_number_of_threads(bool relative, int32_t size) noexcept {
    if(relative) {
      uint32_t sz = size * std::thread::hardware_concurrency();
      if(sz > 0) return sz;
    }
    return size;
  }

  static IoContextPtr make(std::string&& _name, bool relative, int32_t size) {
    return IoContextPtr(
      new IoContext(
        std::move(_name),
        get_number_of_threads(relative, size)
      )
    );
  }

  Core::AtomicBool                     running;
  const std::string                    name;
  asio::thread_pool                    pool;
  std::unique_ptr<asio::signal_set>    signals;


  IoContext(std::string&& _name, int32_t size);

  ~IoContext() noexcept { }

  SWC_CAN_INLINE
  int32_t get_size() const noexcept {
    return m_size; // asio::query(executor(), asio::execution::occupancy);
  }

  SWC_CAN_INLINE
  Executor executor() noexcept {
    return pool.get_executor();
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
  template <typename T_Handler>
  SWC_CAN_INLINE
  void post(T_Handler&& handler) {
    asio::post(pool, std::move(handler));
  }
#pragma GCC diagnostic pop

  void set_signals();

  void set_periodic_timer(const Config::Property::Value_int32_g::Ptr ms,
                          PeriodicTimer::Call_t&& call);

  void stop();

  private:
  int32_t             m_size;
  PeriodicTimers      m_periodic_timers;


};

} //namespace Comm



namespace Env {

class IoCtx final {
  public:

  SWC_CAN_INLINE
  static void init(int32_t size) {
    m_env.reset(new IoCtx(size));
  }

  SWC_CAN_INLINE
  static void init(bool relative, int32_t size) {
    m_env.reset(new IoCtx(relative, size));
  }

  SWC_CAN_INLINE
  static bool ok() noexcept {
    return bool(m_env);
  }

  static Comm::IoContextPtr io() {
    SWC_ASSERT(ok());
    return m_env->m_io;
  }

  template <typename T_Handler>
  SWC_CAN_INLINE
  static void post(T_Handler&& handler)  {
    m_env->m_io->post(std::move(handler));
  }

  SWC_CAN_INLINE
  static bool stopping() noexcept {
    return !m_env->m_io->running;
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  SWC_CAN_INLINE
  IoCtx(int32_t size)
      : m_io(Comm::IoContext::make("Env", size)) {
  }

  SWC_CAN_INLINE
  IoCtx(bool relative, int32_t size)
        : m_io(Comm::IoContext::make("Env", relative, size)) {
  }

  ~IoCtx() noexcept { }

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
