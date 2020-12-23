/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_LockAtomicUnique_h
#define swcdb_core_LockAtomicUnique_h

#include <atomic>
#include <thread>
#include "swcdb/core/Compat.h"


namespace SWC { namespace Core {


class MutexAtomic {
  public:

  explicit MutexAtomic() noexcept : want(ATOMIC_FLAG_INIT) { }

  MutexAtomic(const MutexAtomic&) = delete;

  MutexAtomic(const MutexAtomic&&) = delete;
    
  MutexAtomic& operator=(const MutexAtomic&) = delete;

  ~MutexAtomic() { }

  SWC_CAN_INLINE
  bool try_lock() const noexcept {
    return !want.test_and_set(std::memory_order_acquire);
    /*
    bool at=false;
    return want.compare_exchange_weak(at, true);
    */
  }
  
  SWC_CAN_INLINE
  void lock() const noexcept {
    uint16_t i = 0;
    while(want.test_and_set(std::memory_order_acquire)) {
      if(!++i)
        std::this_thread::yield();
    }
    /*
    for(auto at=false;
        !want.compare_exchange_weak(at, true);
        at=false)
      if(!++i)
        std::this_thread::yield();
    */
  }

  void lock(const uint32_t& us_sleep) const noexcept {
    uint16_t i = 0;
    while(want.test_and_set(std::memory_order_acquire)) {
      if(!++i)
        std::this_thread::sleep_for(std::chrono::microseconds(us_sleep));
    }
    /*
    uint16_t i = 0;
    for(auto at=false;
        !want.compare_exchange_weak(at, true);
        at=false)
      if(!++i)
        std::this_thread::sleep_for(std::chrono::microseconds(us_sleep));
    */
  }

  SWC_CAN_INLINE
  void unlock() const noexcept {
    want.clear(std::memory_order_release);
    //want.store(false);
  }
  
  class scope final {
    public:

    scope(const MutexAtomic& m) noexcept : _m(m) {  _m.lock(); }

    ~scope() noexcept { _m.unlock(); }
    
    scope(const scope&) = delete;

    scope(const scope&&) = delete;

    scope& operator=(const scope&) = delete;

    private:
    const MutexAtomic&      _m;
  };

  private:

  mutable std::atomic_flag want;
  //mutable Core::AtomicBase<bool, std::memory_order_seq_cst> want;
};


}} //namespace SWC::Core



// SWC_LOCK_WITH_SUPPORT or use Core::MutexSptd with Core::MutexSptd::scope 
#define SWC_LOCK_WITH_SUPPORT(_mutex_, _state_, _code_, _return_) \
  if(_state_.try_lock()) { \
    _code_; \
    _state_.unlock(); \
  } else { \
    _mutex_.lock(); _state_.lock(); \
    _code_; \
    _state_.unlock(); _mutex_.unlock(); \
  } \
  _return_;

#endif // swcdb_core_LockAtomicUnique_h
