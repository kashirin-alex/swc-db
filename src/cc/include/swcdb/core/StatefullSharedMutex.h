/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_StatefullSharedMutex_h
#define swc_core_StatefullSharedMutex_h


#include <mutex>
#include "swcdb/core/LockAtomicUnique.h"

namespace SWC { 

class StatefullSharedMutex final {
  public:

  StatefullSharedMutex() : m_unique(false), m_shared(0)  { }

  ~StatefullSharedMutex () { }

  void lock() {
    {
      LockAtomic::Unique::Scope lock(m_state);
      m_unique = true;
    }
    m_mutex.lock();
  }

  void unlock() {
    {
      LockAtomic::Unique::Scope lock(m_state);
      m_unique = false;
    }
    m_mutex.unlock();
  }

  bool try_lock() {
    LockAtomic::Unique::Scope lock(m_state);
    if(m_unique || m_shared)
      return false;
    bool ok = m_mutex.try_lock();
    if(ok)
      m_unique = true;
    return ok;
  }
  

  void lock_shared() {
    {
      LockAtomic::Unique::Scope lock(m_state);
      ++m_shared;
    }
    m_mutex.lock_shared();
  }

  void unlock_shared() {
    {
      LockAtomic::Unique::Scope lock(m_state);
      --m_shared;
    }
    m_mutex.unlock_shared();
  }
  
  bool try_lock_shared() { 
    LockAtomic::Unique::Scope lock(m_state);
    if(m_unique)
      return false;
    bool ok = m_mutex.try_lock_shared();
    if(ok)
      ++m_shared;
    return ok;
  }


  class scoped_lock final {
    public:
    explicit scoped_lock(StatefullSharedMutex& m) : _m(m) {
      _m.lock();
    }
    ~scoped_lock() {
      _m.unlock();
    }
    private:
    StatefullSharedMutex& _m;
  };

  class shared_lock final {
    public:
    explicit shared_lock(StatefullSharedMutex& m) : _m(m) {
      _m.lock_shared();
    }
    ~shared_lock() {
      _m.unlock_shared();
    }
    private:
    StatefullSharedMutex& _m;
  };
  
  
  private:

  std::shared_mutex   m_mutex;
  LockAtomic::Unique  m_state;
  bool                m_unique;
  size_t              m_shared;
};


}

#endif // swc_core_StatefullSharedMutex_h
