/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fsbroker_FsBrokerEnv_h
#define swcdb_fsbroker_FsBrokerEnv_h


#include <unordered_map>


namespace SWC { namespace FsBroker {

class Fds final : private std::unordered_map<int32_t, FS::SmartFd::Ptr> {

  public:

  typedef Fds* Ptr;

  Fds() : m_next_fd(0) {}

  //~Fds() { }

  int32_t add(const FS::SmartFd::Ptr& smartfd) {
    assign_fd:
      Core::MutexSptd::scope lock(m_mutex);
      if(!emplace(++m_next_fd < 1 ? m_next_fd=1 : m_next_fd, smartfd).second)
        goto assign_fd;
      for(auto it = begin(); it != end(); ++it) {
        if(it->first != m_next_fd && it->second->fd() == smartfd->fd()) {
          erase(it);
          break;
        }
      }
      return m_next_fd;
  }

  FS::SmartFd::Ptr remove(int32_t fd) {
    Core::MutexSptd::scope lock(m_mutex);

    auto it = find(fd);
    if(it == end())
      return nullptr;
    FS::SmartFd::Ptr smartfd = std::move(it->second);
    erase(it);
    return smartfd;
  }

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
          : m_shuttingdown(false), m_in_process(0) {
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

  private:
  Core::AtomicBool                        m_shuttingdown;
  Core::Atomic<int64_t>                   m_in_process;
  SWC::FsBroker::Fds                      m_fds;
  inline static std::shared_ptr<FsBroker> m_env = nullptr;
};


}}

#endif // swcdb_fsbroker_FsBrokerEnv_h
