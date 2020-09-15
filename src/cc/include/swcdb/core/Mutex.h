/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_Mutex_h
#define swc_core_Mutex_h

#include <atomic>
#include <mutex>
#include "swcdb/core/LockAtomicUnique.h"


namespace SWC { 

class Mutex final : private LockAtomic::Unique {
  public:

  explicit Mutex() { }

  Mutex(const Mutex&) = delete;

  Mutex(const Mutex&&) = delete;

  Mutex& operator=(const Mutex&) = delete;

  ~Mutex() { }

  bool lock() {
    if(LockAtomic::Unique::try_lock())
      return true;
    m_mutex.lock();
    LockAtomic::Unique::lock();
    return false;
  }

  bool try_full_lock(bool& support) {
    if(LockAtomic::Unique::try_lock()) {
      support = true;
      return true;
    }
    if(m_mutex.try_lock()) {
      if(LockAtomic::Unique::try_lock()) {
        support = false;
        return true;
      }
      m_mutex.unlock();
    }
    return false;
  }

  void unlock(const bool& support) {
    LockAtomic::Unique::unlock();
    if(!support)
      m_mutex.unlock();
  }
  
  class scope final {
    public:

    scope(Mutex& m) : _m(m), _support(m.lock()) { }

    ~scope() { _m.unlock(_support); }
    
    scope(const scope&) = delete;

    scope(const scope&&) = delete;

    scope& operator=(const scope&) = delete;

    private:
    Mutex&      _m;
    const bool  _support;
  };

  private:
  std::mutex m_mutex;
};

}

#endif // swc_core_Mutex_h
