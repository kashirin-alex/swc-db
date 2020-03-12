/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_LockAtomicRecursive_h
#define swc_core_LockAtomicRecursive_h

#include <atomic>
#include <thread>

namespace SWC { namespace LockAtomic {

inline const size_t get_thread_id() {
  return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

class Recursive final {
  public:

  explicit Recursive(): owner(0), recurse(0) { }
  ~Recursive() { }

  void lock() {
    size_t tid = get_thread_id();
    if(owner.load(std::memory_order_acquire) != tid) {
      uint16_t i = 0;
      for(size_t at=0;
          !owner.compare_exchange_weak(at, tid, std::memory_order_seq_cst);
          at=0)
        if(++i == 0) 
          std::this_thread::yield();
    }
    recurse++;
  }

  void lock(const uint32_t& us_sleep) {
    size_t tid = get_thread_id();
    if(owner.load(std::memory_order_acquire) != tid) {
      uint16_t i = 0;
      for(size_t at=0;
          !owner.compare_exchange_weak(at, tid, std::memory_order_seq_cst);
          at=0)
        if(++i == 0) 
          std::this_thread::sleep_for(std::chrono::microseconds(us_sleep));
    }
    recurse++;
  }

  void unlock() {
    if(!--recurse)
      owner.store(0, std::memory_order_release);
  }
  
  class Scope final {
    public:
    explicit Scope(Recursive& m) : _m(m) {
      _m.lock();
    }
    explicit Scope(Recursive& m, const uint32_t& us_sleep) : _m(m) {
      _m.lock(us_sleep);
    }
    ~Scope() {
      _m.unlock();
    }
    private:
    Recursive& _m;
  };
  
  private:

  std::atomic<size_t>  owner;
  uint32_t             recurse;
};

}}

#endif // swc_core_LockAtomicRecursive_h
