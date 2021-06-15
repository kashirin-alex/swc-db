/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Mutex_h
#define swcdb_core_Mutex_h


#include "swcdb/core/MutexAtomic.h"
#include <mutex>


namespace SWC { namespace Core {

class MutexSptd final : private MutexAtomic {
  public:

  SWC_CAN_INLINE
  explicit MutexSptd() noexcept { }

  MutexSptd(const MutexSptd&)             = delete;
  MutexSptd(MutexSptd&&)                  = delete;
  MutexSptd& operator=(const MutexSptd&)  = delete;
  MutexSptd& operator=(MutexSptd&&)       = delete;

  //~MutexSptd() noexcept { }

  SWC_CAN_INLINE
  bool lock_except() {
    if(MutexAtomic::try_lock())
      return true;
    m_mutex.lock();
    MutexAtomic::lock();
    return false;
  }

  SWC_CAN_INLINE
  bool lock() noexcept {
    if(MutexAtomic::try_lock())
      return true;
    //while(__gthread_mutex_lock(m_mutex.native_handle()))
    //  std::this_thread::yield();
    _again: try {
      m_mutex.lock();
    } catch(...) {
      std::this_thread::yield();
      goto _again;
    }
    MutexAtomic::lock();
    return false;
  }

  //SWC_CAN_INLINE
  bool try_full_lock(bool& support) noexcept {
    if(MutexAtomic::try_lock()) {
      support = true;
      return true;
    }
    if(m_mutex.try_lock()) {
      if(MutexAtomic::try_lock()) {
        support = false;
        return true;
      }
      m_mutex.unlock();
    }
    return false;
  }

  SWC_CAN_INLINE
  void unlock(const bool& support) noexcept {
    MutexAtomic::unlock();
    if(!support)
      m_mutex.unlock();
  }

  class scope_except final {
    public:

    SWC_CAN_INLINE
    scope_except(MutexSptd& m) : _m(m), _support(m.lock_except()) { }

    SWC_CAN_INLINE
    ~scope_except() noexcept { _m.unlock(_support); }

    scope_except(const scope_except&)             = delete;
    scope_except(scope_except&&)                  = delete;
    scope_except& operator=(const scope_except&)  = delete;
    scope_except& operator=(scope_except&&)       = delete;

    private:
    MutexSptd&   _m;
    const bool   _support;
  };

  class scope final {
    public:

    //SWC_CAN_INLINE
    scope(MutexSptd& m) noexcept : _m(m), _support(m.lock()) { }

    SWC_CAN_INLINE
    ~scope() noexcept { _m.unlock(_support); }

    scope(const scope&)             = delete;
    scope(scope&&)                  = delete;
    scope& operator=(const scope&)  = delete;
    scope& operator=(scope&&)       = delete;

    private:
    MutexSptd&   _m;
    const bool   _support;
  };

  private:
  std::mutex m_mutex;
};



}} //namespace SWC::Core


#endif // swcdb_core_Mutex_h
