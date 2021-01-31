/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fsbroker_FsBrokerEnv_h
#define swcdb_fsbroker_FsBrokerEnv_h

#include <unordered_map>
#include <shared_mutex>

namespace SWC { namespace FsBroker {

class Fds final : private std::unordered_map<int32_t, FS::SmartFd::Ptr> {

  public:

  typedef Fds* Ptr;

  Fds() : m_next_fd(0) {}

  ~Fds() { }

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
    m_env = std::make_shared<FsBroker>();
  }

  static SWC::FsBroker::Fds& fds() {
    //SWC_ASSERT(m_env);
    return m_env->m_fds;
  }

  FsBroker() noexcept {}

  ~FsBroker() { }

  private:
  SWC::FsBroker::Fds                      m_fds;
  inline static std::shared_ptr<FsBroker> m_env = nullptr;
};


}}

#endif // swcdb_fsbroker_FsBrokerEnv_h
