/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_sys_Resources_h
#define swcdb_common_sys_Resources_h


#include "swcdb/core/config/Property.h"
#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/CompletionCounter.h"


namespace SWC {


//! The SWC-DB System C++ namespace 'SWC::System'
namespace System {

struct Notifier {

  virtual void rss_used_reg(size_t) noexcept = 0;
  virtual void rss_free(size_t)     noexcept = 0;
  virtual void rss_used(size_t)     noexcept = 0;

  virtual void cpu_user(size_t)     noexcept = 0;
  virtual void cpu_sys(size_t)      noexcept = 0;
  virtual void cpu_threads(size_t)  noexcept = 0;

  virtual uint64_t get_cpu_ms_interval() const noexcept = 0;

  virtual ~Notifier() noexcept { }

};

}}



#include "swcdb/common/sys/Resource_Mem.h"
#include "swcdb/common/sys/Resource_CPU.h"



namespace SWC { namespace System {


class Resources final {
  public:
  CPU cpu;
  Mem mem;

  Resources(const Comm::IoContextPtr& ioctx,
            Config::Property::Value_int32_g::Ptr ram_percent_allowed,
            Config::Property::Value_int32_g::Ptr ram_percent_reserved,
            Config::Property::Value_int32_g::Ptr ram_release_rate,
            Notifier* notifier=nullptr,
            Mem::ReleaseCall_t&& release_call=nullptr)
            : cpu(notifier),
              mem(
                ram_percent_allowed,
                ram_percent_reserved,
                ram_release_rate,
                notifier,
                std::move(release_call)
              ),
              running(0),
              m_mutex_timer(),
              m_timer(ioctx->executor()) {
    checker();
  }

  Resources(const Resources& other)           = delete;
  Resources(Resources&& other)                = delete;
  Resources operator=(const Resources& other) = delete;
  Resources operator=(Resources&& other)      = delete;

  ~Resources() noexcept { }

  SWC_CAN_INLINE
  bool is_low_mem_state() const noexcept {
    return mem.is_low_mem_state();
  }

  SWC_CAN_INLINE
  size_t need_ram() const noexcept {
    return mem.need_ram();
  }

  SWC_CAN_INLINE
  size_t avail_ram() const noexcept {
    return mem.avail_ram();
  }

  SWC_CAN_INLINE
  bool need_ram(uint32_t sz) const noexcept {
    return mem.need_ram(sz);
  }

  SWC_CAN_INLINE
  void more_mem_future(size_t sz) noexcept {
    mem.more_mem_future(sz);
  }

  SWC_CAN_INLINE
  void less_mem_future(size_t sz) noexcept {
    mem.less_mem_future(sz);
  }

  SWC_CAN_INLINE
  void more_mem_releasable(size_t sz) noexcept {
    mem.more_mem_releasable(sz);
  }

  SWC_CAN_INLINE
  void less_mem_releasable(size_t sz) noexcept {
    mem.less_mem_releasable(sz);
  }

  SWC_CAN_INLINE
  void adj_mem_releasable(ssize_t sz) noexcept {
    mem.adj_mem_releasable(sz);
  }

  SWC_CAN_INLINE
  uint32_t available_mem_mb() const noexcept {
    return mem.available_mem_mb();
  }

  SWC_CAN_INLINE
  uint32_t concurrency() const noexcept {
    return cpu.concurrency;
  }

  SWC_CAN_INLINE
  uint32_t available_cpu_mhz() const noexcept {
    return cpu.mhz;
  }

  SWC_CAN_INLINE
  uint32_t cpu_usage() const noexcept {
    return cpu.usage;
  }

  void stop() {
    for(;;) {
      {
        Core::MutexAtomic::scope lock(m_mutex_timer);
        m_timer.cancel();
      }
      if(!running.count())
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  }

  void print(std::ostream& out) const {
    out << "Resources(";
    cpu.print(out << "CPU-");
    mem.print(out << " Mem-MB-", 1048576);
    out << ')';
  }

  private:

  void checker() noexcept {
    running.increment();
    uint64_t ts = Time::now_ms();
    uint64_t t1 = mem.check(ts);
    uint64_t t2 = cpu.check(ts);
    try {
      schedule(t1 < t2 ? t1 : t2);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
  }

  SWC_CAN_INLINE
  void schedule(uint64_t intval) {
    Core::MutexAtomic::scope lock(m_mutex_timer);
    m_timer.expires_after(std::chrono::milliseconds(intval));
    struct TimerTask {
      Resources* ptr;
      SWC_CAN_INLINE
      TimerTask(Resources* a_ptr) noexcept : ptr(a_ptr) { }
      void operator()(const asio::error_code& ec) {
        if(ec != asio::error::operation_aborted)
          ptr->checker();
        ptr->running.is_last();
      }
    };
    m_timer.async_wait(TimerTask(this));
  }

  Core::CompletionCounter<uint8_t>  running;
  Core::MutexAtomic                 m_mutex_timer;
  asio::steady_timer                m_timer;

};


}}


#endif // swcdb_common_sys_Resources_h
