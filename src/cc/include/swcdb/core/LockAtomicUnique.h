/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_LockAtomicUnique_h
#define swc_core_LockAtomicUnique_h

#include <atomic>
#include <thread>


namespace SWC { namespace LockAtomic {

class Unique {
  public:

  explicit Unique(): want(false) { }

  Unique(const Unique&) = delete;

  Unique(const Unique&&) = delete;
    
  Unique& operator=(const Unique&) = delete;

  ~Unique() { }

  bool try_lock() {
    bool at=false;
    return want.compare_exchange_weak(at, true, std::memory_order_seq_cst);
  }
  
  void lock() {
    uint16_t i = 0;
    for(auto at=false;
        !want.compare_exchange_weak(at, true, std::memory_order_seq_cst);
        at=false)
      if(++i == 0)
        std::this_thread::yield();
  }

  void lock(const uint32_t& us_sleep) {
    uint16_t i = 0;
    for(auto at=false;
        !want.compare_exchange_weak(at, true, std::memory_order_seq_cst);
        at=false)
      if(++i == 0)
        std::this_thread::sleep_for(std::chrono::microseconds(us_sleep));
  }

  void unlock() {
    want.store(false, std::memory_order_release);
  }
  
  class scope final {
    public:

    scope(Unique& m) : _m(m) {  _m.lock(); }

    ~scope() { _m.unlock(); }
    
    scope(const scope&) = delete;

    scope(const scope&&) = delete;

    scope& operator=(const scope&) = delete;

    private:
    Unique&      _m;
  };

  private:

  std::atomic<bool> want;
};

}}

// SWC_LOCK_WITH_SUPPORT or use Mutex with Mutex::scope 
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

#endif // swc_core_LockAtomicUnique_h
