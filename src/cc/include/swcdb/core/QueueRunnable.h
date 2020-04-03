/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_QueueRunnable_h
#define swc_core_QueueRunnable_h

#include <queue>
#include <functional>
#include "swcdb/core/LockAtomicUnique.h"

namespace SWC { 


class QueueRunnable : private std::queue<std::function<void()>> {
  public:
  
  typedef std::function<void()>  Call_t;

  void push(const Call_t& call) {
    m_mutex.lock();
    QBase::push(call);
    m_mutex.unlock();
  }

  bool running() {
    bool chk;
    m_mutex.lock();
    chk = m_runs;
    m_mutex.unlock();
    return chk;
  }

  bool empty() {
    bool chk;
    m_mutex.lock();
    chk = QBase::empty();
    m_mutex.unlock();
    return chk;
  }

  size_t size() {
    size_t chk;
    m_mutex.lock();
    chk = QBase::size();
    m_mutex.unlock();
    return chk;
  }

  bool need_run() {
    bool chk;
    m_mutex.lock();
    chk = (m_runs || QBase::empty()) ? false : m_runs = true;
    m_mutex.unlock();
    return chk;
  }

  void run() {
    for(Call_t call;;) {
      m_mutex.lock();
      call = front();
      m_mutex.unlock();

      call();
      
      m_mutex.lock();
      pop();
      if(QBase::empty()) {
        m_runs = false;
        m_mutex.unlock();
        return;
      }
      m_mutex.unlock();
    }
  }

  void run(const Call_t& post) {
    for(Call_t call;;) {
      m_mutex.lock();
      call = front();
      m_mutex.unlock();

      call();
      
      m_mutex.lock();
      pop();
      if(!QBase::empty()) {
        m_mutex.unlock();
        continue;
      }
      m_mutex.unlock();

      post();

      m_mutex.lock();
      if(QBase::empty()) {
        m_runs = false;
        m_mutex.unlock();
        return;
      }
      m_mutex.unlock();
    }
  }

  private:
  LockAtomic::Unique          m_mutex;
  bool                        m_runs = false;

  typedef std::queue<Call_t>  QBase;
  using QBase::empty;
  using QBase::size;
  using QBase::front;
  using QBase::pop;
};


}

#endif // swc_core_QueueRunnable_h
