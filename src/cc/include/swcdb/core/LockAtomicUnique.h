/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_LockAtomicUnique_h
#define swc_core_LockAtomicUnique_h

#include <atomic>
#include <thread>

namespace SWC { namespace LockAtomic {

class Unique final {
  public:

  explicit Unique(): want(false) { }

  ~Unique() { }

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
  
  class Scope final {
    public:
    explicit Scope(Unique& m) : _m(m) {
      _m.lock();
    }
    explicit Scope(Unique& m, const uint32_t& us_sleep) : _m(m) {
      _m.lock(us_sleep);
    }
    ~Scope() {
      _m.unlock();
    }
    private:
    Unique& _m;
  };

  private:

  std::atomic<bool> want;
};

}}

#endif // swc_core_LockAtomicUnique_h
