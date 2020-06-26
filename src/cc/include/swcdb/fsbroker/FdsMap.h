/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fsbroker_FdsMap_h
#define swc_fsbroker_FdsMap_h

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
      Mutex::scope lock(m_mutex);
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
    Mutex::scope lock(m_mutex);
    
    auto it = find(fd);
    if(it == end())
      return nullptr;
    FS::SmartFd::Ptr smartfd = std::move(it->second);
    erase(it);
    return smartfd;
  }

  FS::SmartFd::Ptr select(int32_t fd) {
    Mutex::scope lock(m_mutex);
    
    auto it = find(fd);
    return it == end() ? nullptr : it->second;
  }

  FS::SmartFd::Ptr pop_next() {
    Mutex::scope lock(m_mutex);
    
    auto it = begin();
    if(it == end())
      return nullptr;
    FS::SmartFd::Ptr smartfd = std::move(it->second);
    erase(it);
    return smartfd;
  }

  private:
  Mutex               m_mutex;
  int32_t             m_next_fd;
};

} // namespace FsBroker


namespace Env {
class Fds final {
  public:

  static void init() {
    m_env = std::make_shared<Fds>();
  }

  static FsBroker::Fds::Ptr get(){
    SWC_ASSERT(m_env != nullptr);
    return m_env->m_fds;
  }

  Fds() : m_fds(new FsBroker::Fds()) {}

  ~Fds(){
    if(m_fds != nullptr)  
      delete m_fds;
  }

  private:
  FsBroker::Fds::Ptr                 m_fds = nullptr;
  inline static std::shared_ptr<Fds> m_env = nullptr;
};


}}

#endif // swc_fsbroker_FdsMap_h