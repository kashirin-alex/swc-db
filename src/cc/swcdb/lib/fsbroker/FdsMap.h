/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_fsbroker_FdsMap_h
#define swc_app_fsbroker_FdsMap_h

#include <unordered_map>


namespace SWC { namespace server { namespace FsBroker {

typedef std::unordered_map<int32_t, FS::SmartFdPtr> FdsMap;
typedef std::pair<int32_t, FS::SmartFdPtr> FdpPair;

class Fds {

  public:

  Fds() : m_next_fd(0) {}
  virtual ~Fds(){}

  int32_t add(FS::SmartFdPtr fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    while(!m_fds.insert(FdpPair(++m_next_fd, fd)).second);
    return m_next_fd;
  }

  FS::SmartFdPtr remove(int32_t fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_fds.find(fd);
    if(it == m_fds.end())
      return nullptr;
    FS::SmartFdPtr smartfd = it->second;
    m_fds.erase(it);
    return smartfd;
  }

  FS::SmartFdPtr select(int32_t fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_fds.find(fd);
    if(it != m_fds.end())
      return it->second;
    return nullptr;
  }

  FS::SmartFdPtr pop_next() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_fds.begin();
    if(it == m_fds.end())
      return nullptr;
    FS::SmartFdPtr smartfd = it->second;
    m_fds.erase(it);
    return smartfd;
  }

  private:
  std::mutex   m_mutex;
  int32_t      m_next_fd;
  std::unordered_map<int32_t, FS::SmartFdPtr> m_fds;
};
typedef std::shared_ptr<Fds> FdsPtr;

}}



class EnvFds {
  
  public:

  static void init() {
    m_env = std::make_shared<EnvFds>();
  }

  static server::FsBroker::FdsPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_fds;
  }

  EnvFds() : m_fds(std::make_shared<server::FsBroker::Fds>()) {}

  virtual ~EnvFds(){}

  private:
  server::FsBroker::FdsPtr              m_fds = nullptr;
  inline static std::shared_ptr<EnvFds> m_env = nullptr;
};

}

#endif // swc_app_fsbroker_FdsMap_h