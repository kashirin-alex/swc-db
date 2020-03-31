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
    LockAtomic::Unique::Scope lock(m_mutex);
    QBase::push(call);
  }

  bool running() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return m_runs;
  }

  bool empty() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::empty();
  }

  size_t size() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::size();
  }

  bool need_run() {
    LockAtomic::Unique::Scope lock(m_mutex);
    if(m_runs || QBase::empty()) 
      return false;
    m_runs = true;
    return true;
  }

  void run() {
    for(Call_t call;;) {
      {
        LockAtomic::Unique::Scope lock(m_mutex);
        call = front();
      }

      call();
      
      {
        LockAtomic::Unique::Scope lock(m_mutex);
        pop();
        if(QBase::empty()) {
          m_runs = false;
          return;
        }
      }
    }
  }

  void run(const Call_t& post) {
    for(Call_t call;;) {
      {
        LockAtomic::Unique::Scope lock(m_mutex);
        call = front();
      }

      call();
      
      {
        LockAtomic::Unique::Scope lock(m_mutex);
        pop();
        if(!QBase::empty())
          continue;
      }

      post();

      {
        LockAtomic::Unique::Scope lock(m_mutex);
        if(QBase::empty()) {
          m_runs = false;
          return;
        }
      }
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
