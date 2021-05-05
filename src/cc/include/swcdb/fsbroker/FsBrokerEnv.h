/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fsbroker_FsBrokerEnv_h
#define swcdb_fsbroker_FsBrokerEnv_h


#include "swcdb/common/sys/Resources.h"
#include "swcdb/fs/Broker/Protocol/Commands.h"
#include "swcdb/fsbroker/queries/update/MetricsReporting.h"

#include <unordered_map>


namespace SWC { namespace FsBroker {

class Fds final : private std::unordered_map<int32_t, FS::SmartFd::Ptr> {

  public:

  typedef Fds* Ptr;

  Fds() : m_next_fd(0) {}

  //~Fds() { }

  int32_t add(const FS::SmartFd::Ptr& smartfd);

  FS::SmartFd::Ptr remove(int32_t fd);

  FS::SmartFd::Ptr select(int32_t fd) noexcept {
    Core::MutexSptd::scope lock(m_mutex);

    auto it = find(fd);
    return it == end() ? nullptr : it->second;
  }

  FS::SmartFd::Ptr pop_next() {
    Core::MutexSptd::scope lock(m_mutex);

    auto it = begin();
    if(it == end())
      return nullptr;
    FS::SmartFd::Ptr smartfd = std::move(it->second);
    erase(it);
    return smartfd;
  }

  private:
  Core::MutexSptd     m_mutex;
  int32_t             m_next_fd;
};

} // namespace FsBroker


namespace Env {
class FsBroker final {
  public:

  static void init() {
    SWC_ASSERT(!m_env);
    m_env = std::make_shared<FsBroker>();
  }

  static SWC::FsBroker::Fds& fds() noexcept {
    return m_env->m_fds;
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  SWC_CAN_INLINE
  static SWC::FsBroker::Metric::Reporting::Ptr& metrics_track() noexcept {
    return m_env->_reporting;
  }

  SWC_CAN_INLINE
  static Common::Resources& res() noexcept {
    return m_env->_resources;
  }

  static int64_t in_process() noexcept {
    return m_env->m_in_process;
  }

  static void in_process(int64_t count) noexcept {
    m_env->m_in_process.fetch_add(count);
  }

  static bool can_process() noexcept {
    return m_env->_can_process();
  }

  static void shuttingdown() {
    return m_env->_shuttingdown();
  }

  FsBroker() noexcept
    : cfg_ram_percent_allowed(100, nullptr),
      cfg_ram_percent_reserved(0, nullptr),
      cfg_ram_release_rate(100, nullptr),
      _reporting(
        SWC::Env::Config::settings()->get_bool("swc.FsBroker.metrics.enabled")
          ? std::make_shared<SWC::FsBroker::Metric::Reporting>(
              Env::IoCtx::io(),
              SWC::Env::Config::settings()
                ->get<SWC::Config::Property::V_GINT32>(
                  "swc.FsBroker.metrics.report.interval"))
          : nullptr
      ),
      _resources(
        Env::IoCtx::io(),
        &cfg_ram_percent_allowed,
        &cfg_ram_percent_reserved,
        &cfg_ram_release_rate,
        nullptr,
        _reporting ? &_reporting->hardware : nullptr
      ),
      m_shuttingdown(false), m_in_process(0) {
  }

  //~FsBroker() { }

  bool _can_process() noexcept {
    if(m_shuttingdown)
      return false;
    m_in_process.fetch_add(1);
    return true;
  }

  void _shuttingdown() {
    m_shuttingdown.store(true);

    if(_reporting)
      _reporting->stop();

    auto fs = Env::FsInterface::fs();
    size_t n = 0;
    do {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      if(!(++n % 10))
        SWC_LOGF(LOG_WARN,
          "In-process=%ld fs-use-count=%ld check=%lu",
          m_in_process.load(), fs.use_count(), n);
    } while(m_in_process || fs.use_count() > 2);

    int err;
    for(FS::SmartFd::Ptr fd; (fd = m_fds.pop_next()); ) {
      if(fd->valid()) {
        err = Error::OK;
        if(fd->flags() & O_WRONLY)
          Env::FsInterface::fs()->sync(err, fd);
        Env::FsInterface::fs()->close(err, fd);
      }
    }
  }

  SWC::Config::Property::V_GINT32         cfg_ram_percent_allowed;
  SWC::Config::Property::V_GINT32         cfg_ram_percent_reserved;
  SWC::Config::Property::V_GINT32         cfg_ram_release_rate;

  private:
  SWC::FsBroker::Metric::Reporting::Ptr   _reporting;
  Common::Resources                       _resources;
  Core::AtomicBool                        m_shuttingdown;
  Core::Atomic<int64_t>                   m_in_process;
  SWC::FsBroker::Fds                      m_fds;
  inline static std::shared_ptr<FsBroker> m_env = nullptr;

};


} // namespace Env



namespace FsBroker {


int32_t Fds::add(const FS::SmartFd::Ptr& smartfd) {
  int32_t fd;
  assign_fd: {
    Core::MutexSptd::scope lock(m_mutex);
    if(!emplace(++m_next_fd < 1 ? m_next_fd=1 : m_next_fd, smartfd).second)
      goto assign_fd;
    for(auto it = begin(); it != end(); ++it) {
      if(it->first != m_next_fd && it->second->fd() == smartfd->fd()) {
        erase(it);
        break;
      }
    }
    fd = m_next_fd;
  }
  if(auto& m = Env::FsBroker::metrics_track())
    m->fds->increment();
  return fd;
}

FS::SmartFd::Ptr Fds::remove(int32_t fd) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    auto it = find(fd);
    if(it == end())
      return nullptr;
    FS::SmartFd::Ptr smartfd = std::move(it->second);
    erase(it);
  }
  if(auto& m = Env::FsBroker::metrics_track())
    m->fds->decrement();
  return smartfd;
}


}}

#endif // swcdb_fsbroker_FsBrokerEnv_h
