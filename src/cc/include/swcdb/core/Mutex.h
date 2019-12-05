/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_Mutex_h
#define swc_core_Mutex_h


#include <mutex>
#include <thread>
#include <condition_variable>

#include "swcdb/core/LockAtomicUnique.h"
#include "swcdb/core/LockAtomicRecursive.h"

namespace SWC { namespace LockAtomic {

class RW final {
  public:

  explicit RW() : state({false, 0}), recurse(0) { }

  ~RW() { }

  void lock(const uint32_t& us_sleep = 0) {
    size_t tid = get_thread_id();
    if(state.load(std::memory_order_acquire).owner != tid) {
      uint16_t i = 0;

      State new_state({true, tid});
      State empty({false, 0});
      for(auto at=empty;
          !state.compare_exchange_weak(at, new_state, std::memory_order_seq_cst);
          at=empty)
        if(++i == 0)
          us_sleep ? 
            std::this_thread::sleep_for(std::chrono::microseconds(us_sleep))
            : std::this_thread::yield();  
      let_users(tid, us_sleep);
    }
    recurse++;
  }

  void unlock() {
    if(!--recurse)
      state.store({false, 0}, std::memory_order_release);
  }
  
  void lock_shared(const uint32_t& us_sleep = 0) {
    size_t tid = get_thread_id();
    for(uint16_t i = 0; ; ) {
      const State& s = state.load(std::memory_order_acquire);
      if(s.want_x && s.owner != tid)
        if(++i == 0)
          us_sleep ? 
            std::this_thread::sleep_for(std::chrono::microseconds(us_sleep))
            : std::this_thread::yield();
      else
        break;
    }
    get_user(tid, us_sleep).value++;
  }

  void unlock_shared() {
    auto& usr = get_user(get_thread_id());
    if(!--usr.value)
      usr.tid.store(0, std::memory_order_release);
  }

  class UniqueScope final {
    public:
    explicit UniqueScope(RW& m, const uint32_t& us_sleep = 0) : _m(m) {
      _m.lock(us_sleep);
    }
    ~UniqueScope() {
      _m.unlock();
    }
    private:
    RW& _m;
  };

  class SharedScope final {
    public:
    explicit SharedScope(RW& m, const uint32_t& us_sleep = 0) : _m(m) {
      _m.lock_shared(us_sleep);
    }
    ~SharedScope() {
      _m.unlock_shared();
    }
    private:
    RW& _m;
  };
  
  private:

  struct User final {
    std::atomic<size_t>   tid = 0;
    std::atomic<uint32_t> value = 0;
  };

  User& get_user(const size_t& tid, const uint32_t& us_sleep=0) {
    uint8_t n;
    for(n=0; n<8; n++) {
      auto& usr = users[n];
      if(usr.tid.load(std::memory_order_acquire) == tid)
        return usr;
    }

    size_t no;
    for(uint16_t i=0;;) {
      for(n=0; n<8; n++) {
        auto& usr = users[n];
        no = 0;
        if(usr.tid.compare_exchange_weak(no, tid, std::memory_order_seq_cst))
          return usr;
      }

      if(++i == 0)
        us_sleep ? 
          std::this_thread::sleep_for(std::chrono::microseconds(us_sleep))
          : std::this_thread::yield();
    }
    return users[0]; // won't happen
  }

  void let_users(const size_t& tid, const uint32_t& us_sleep) {
    uint8_t n;
    for(uint16_t i = 0;;) {
      for(n=0; n<8; n++) {
        auto& usr = users[n];
        if(usr.tid.load(std::memory_order_acquire) == tid
          || !usr.value.load(std::memory_order_acquire)) {
          if(n == 7)
            return;
        } else
          break;
      }
      if(++i == 0)
        us_sleep ? 
          std::this_thread::sleep_for(std::chrono::microseconds(us_sleep))
          : std::this_thread::yield();
    }
  }

  struct State final {
    bool    want_x;
    size_t  owner;
  };
  std::atomic<State>  state;
  uint32_t            recurse;
  User                users[8];
};

}



template<class M>
class SharedLock final {
  public:
  explicit SharedLock(M& m) : _m(m) {
    _m.lock_shared();
  }
    
  ~SharedLock() {
    _m.unlock_shared();
  }

  private:
  M& _m;
};

template<class M>
class UniqueLock final {
  public:
  explicit UniqueLock(M& m) : _m(m) {
    _m.lock();
  }

  ~UniqueLock() {
    _m.unlock();
  }
    
  private:
  M& _m;
};

/************
class Mutex {
  public:

  explicit Mutex(): recursive_xlock_count(0),
                    want_x_lock(false), 
                    owner_tid(std::thread::id()) { 
  }

  virtual ~Mutex() { }
  
  void lock_shared() {
    lock();
  }

  void unlock_shared() {
    unlock();
  }

  
  void lock() {
    if(!own())
      take_ownership();
    ++recursive_xlock_count;
  }

  void unlock() {
    assert(recursive_xlock_count > 0);

    if(!--recursive_xlock_count) {
      tid_clear();
      want_x_lock.store(false, std::memory_order_release);      
    }
  }
  
  private:

  const bool own() const {
    return owner_tid.load(std::memory_order_acquire)
           == std::this_thread::get_id();
  }

  void take_ownership() {
    uint16_t i = 0;
    bool nowant = false;
    auto tid = std::this_thread::get_id(); 
    std::thread::id no_tid;

    while(!want_x_lock.compare_exchange_strong(nowant, true, std::memory_order_seq_cst)
       || !owner_tid.compare_exchange_strong(no_tid, tid, std::memory_order_seq_cst))
      if(++i == 0) std::this_thread::yield();
  }

  void tid_clear() {
    owner_tid.store(std::thread::id(), std::memory_order_release);
  }

  uint32_t                      recursive_xlock_count;
  std::atomic<bool>             want_x_lock;
  std::atomic<std::thread::id>  owner_tid;
};
*************/


/************
class Mutex {
  public:

  explicit Mutex(): m_mutex(PTHREAD_MUTEX_INITIALIZER), 
                    m_cond_unique(PTHREAD_COND_INITIALIZER), 
                    m_cond_share(PTHREAD_COND_INITIALIZER), 
                    m_shared(0), m_waiters(0), m_unique_id(0) {
  }

  virtual ~Mutex() {
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond_unique);
    pthread_cond_destroy(&m_cond_share);
  }

  void lock_shared() {
    pthread_mutex_lock(&m_mutex);

    while(m_waiters || m_unique_id)
      //|| (m_unique_id && !pthread_equal(m_unique_id, pthread_self())))
      pthread_cond_wait(&m_cond_share, &m_mutex);
    m_shared++;

    pthread_mutex_unlock(&m_mutex);
  }

  void unlock_shared() {
    pthread_mutex_lock(&m_mutex);
    --m_shared;
    if(m_waiters)
      pthread_cond_signal(&m_cond_unique);
      
    pthread_mutex_unlock(&m_mutex);
  }

  
  void lock() {
    //auto tid = pthread_self();
    pthread_mutex_lock(&m_mutex);
    
    m_waiters++;
    while(m_shared || m_unique_id) // (m_unique_id && !pthread_equal(m_unique_id, tid)))
      pthread_cond_wait(&m_cond_unique, &m_mutex);
    m_waiters--;

    m_unique_id = 1; // tid;

    pthread_mutex_unlock(&m_mutex);
  }

  void unlock() {
    pthread_mutex_lock(&m_mutex);

    m_unique_id = 0;
    if(m_waiters)
      pthread_cond_signal(&m_cond_unique);
    else
      pthread_cond_broadcast(&m_cond_share);

    pthread_mutex_unlock(&m_mutex);
  }
  
  private:

  pthread_mutex_t m_mutex;
  pthread_cond_t  m_cond_unique;
  pthread_cond_t  m_cond_share;
  uint32_t        m_shared, m_waiters;
  uint8_t         m_unique_id; // pthread_t
};
*************/


/************
class Mutex {
  public:

  explicit Mutex(): m_shared(0), m_waiters(0), m_unique_id(0) {}

  virtual ~Mutex() {}


  void lock_shared() {
    auto tid = pthread_self();
    std::unique_lock lock(m_mutex);
    if(!ok_read(tid))
      m_rcond.wait(lock, [this, tid]() { return ok_read(tid); });
    m_shared++;
  }

  void unlock_shared() {
    std::unique_lock<std::mutex> m_mutex;
    m_shared--;
    if(m_waiters)
      m_wcond.notify_one();
  }

  
  void lock() {
    auto tid = pthread_self();
    std::unique_lock lock(m_mutex);
    if(!ok_write(tid)) {
      m_waiters++;
      m_wcond.wait(lock, [this, tid]() { return ok_write(tid); });
      m_waiters--;
    }
    m_unique_id = tid;
  }

  void unlock() {
    std::unique_lock<std::mutex> m_mutex;
    m_unique_id = 0;
    if(m_waiters)
      m_wcond.notify_one();
    else
      m_rcond.notify_all();
  }
  
  private:

  const bool ok_read(const pthread_t& tid)  {
    return !m_waiters && (!m_unique_id || pthread_equal(m_unique_id, tid));
  }

  const bool ok_write(const pthread_t& tid) {
    return !m_shared && (!m_unique_id || pthread_equal(m_unique_id, tid));
  }
  
  std::mutex                m_mutex;
  std::condition_variable   m_wcond;
  std::condition_variable   m_rcond;
  uint32_t                  m_shared, m_waiters;
  pthread_t                 m_unique_id;
};
*************/


/************
class Mutex {
  public:

  explicit Mutex() : m_rwlock(PTHREAD_RWLOCK_INITIALIZER) { }

  virtual ~Mutex() {
    pthread_rwlock_destroy(&m_rwlock);
  }

  void lock_shared() {
    pthread_rwlock_rdlock(&m_rwlock);
  }

  void unlock_shared() {
    pthread_rwlock_unlock(&m_rwlock);
  }

  void lock() {
    //if(own())
    //  return;
    pthread_rwlock_wrlock(&m_rwlock);
    //m_writing = pthread_self();
  }

  void unlock() {
    //if(!own())
    //  return;
    //m_writing = 0;
    pthread_rwlock_unlock(&m_rwlock);
  }
  
  private:

  //const bool own() const {
  //  return !m_writing || pthread_equal(m_writing, pthread_self());
  //}

  pthread_rwlock_t m_rwlock;
  //pthread_t        m_writing;

};
*************/


  //typedef Mutex   Mutex_t;
  //typedef Reader  LockRead;
  //typedef Writer  LockWrite;

  //typedef std::shared_mutex           Mutex_t;
  //typedef std::shared_lock<Mutex_t>   LockRead;
  //typedef std::unique_lock<Mutex_t>   LockWrite;

}

#endif // swc_core_Mutex_h
