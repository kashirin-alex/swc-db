/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_QueueRunnable_h
#define swc_core_QueueRunnable_h

#include <queue>
#include <functional>
#include "swcdb/core/Mutex.h"

namespace SWC { 


class QueueRunnable final : private std::queue<std::function<void()>> {
  public:
  
  typedef std::function<void()>  Call_t;

  explicit QueueRunnable() { }

  QueueRunnable(const QueueRunnable&) = delete;

  QueueRunnable(const QueueRunnable&&) = delete;
    
  QueueRunnable& operator=(const QueueRunnable&) = delete;

  void push(const Call_t& call) {
    auto support(m_mutex.lock());
    QBase::push(call);
    m_mutex.unlock(support);
  }

  bool running() const {
    Mutex::scope lock(m_mutex);
    return m_runs;
  }

  bool empty() const {
    Mutex::scope lock(m_mutex);
    return QBase::empty();
  }

  size_t size() const {
    Mutex::scope lock(m_mutex);
    return QBase::size();
  }

  bool need_run() {
    Mutex::scope lock(m_mutex);
    return (m_runs || QBase::empty()) ? false : m_runs = true;
  }

  void run() {
    bool support;
    for(Call_t call;;) {
      support = m_mutex.lock();
      call = front();
      m_mutex.unlock(support);

      call();
      
      support = m_mutex.lock();
      QBase::pop();
      if(QBase::empty()) {
        m_runs = false;
        m_mutex.unlock(support);
        return;
      }
      m_mutex.unlock(support);
    }
  }

  void run(const Call_t& post) {
    bool support;
    for(Call_t call;;) {
      support = m_mutex.lock();
      call = front();
      m_mutex.unlock(support);

      call();
      
      support = m_mutex.lock();
      QBase::pop();
      if(!QBase::empty()) {
        m_mutex.unlock(support);
        continue;
      }
      m_mutex.unlock(support);

      post();

      support = m_mutex.lock();
      if(QBase::empty()) {
        m_runs = false;
        m_mutex.unlock(support);
        return;
      }
      m_mutex.unlock(support);
    }
  }

  private:
  mutable Mutex               m_mutex;
  bool                        m_runs = false;

  typedef std::queue<Call_t>  QBase;

};


}

#endif // swc_core_QueueRunnable_h
