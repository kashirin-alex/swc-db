/**
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

// https://coliru.stacked-crooked.com/a/ca4fdaae820e15ed

#include <iostream>
#include <string>
#include <vector>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <map>
#include <chrono>
#include <future>
#include <cassert>
#include <unordered_map>
using namespace std::chrono_literals;

#include "../Mutex.h"

// for test comparison 
// https://github.com/AlexeyAB/object_threadsafe/blob/master/contfree_shared_mutex/safe_ptr.h
template<unsigned contention_free_count = 36, bool shared_flag = false>
class contention_free_shared_mutex {

	std::atomic<bool> want_x_lock;

  typedef std::array<std::atomic<int>, contention_free_count> array_slock_t;
        
	const std::shared_ptr<array_slock_t> shared_locks_array_ptr;  // 0 - unregistred, 1 registred & free, 2... - busy
	char avoid_falsesharing_1[64];

  array_slock_t& shared_locks_array;

	int recursive_xlock_count;

	enum index_op_t { 
    unregister_thread_op, 
    get_index_op, 
    register_thread_op 
  };

	typedef std::thread::id thread_id_t;
	std::atomic<thread_id_t> owner_thread_id;
	thread_id_t get_fast_this_thread_id() { 
    return std::this_thread::get_id();
  }

  struct unregister_t {
    int thread_index;
    std::shared_ptr<array_slock_t> array_slock_ptr;
    unregister_t(int index, std::shared_ptr<array_slock_t> const& ptr) : thread_index(index), array_slock_ptr(ptr) {}
    unregister_t(unregister_t &&src) : thread_index(src.thread_index), array_slock_ptr(std::move(src.array_slock_ptr)) {}
    ~unregister_t() { if (array_slock_ptr.use_count() > 0) (*array_slock_ptr)[thread_index]--; }
  };

  int get_or_set_index(index_op_t index_op = get_index_op, int set_index = -1) {
    thread_local static std::unordered_map<void *, unregister_t> thread_local_index_hashmap;
    // get thread index - in any cases
    auto it = thread_local_index_hashmap.find(this);
    if (it != thread_local_index_hashmap.cend())
      set_index = it->second.thread_index;

    if(index_op == unregister_thread_op) {  // unregister thread
      if (shared_locks_array[set_index] == 1) 
        // if isn't shared_lock now
        thread_local_index_hashmap.erase(this);
      else
        set_index = -1;

    } else if(index_op == register_thread_op) {  // register thread
      thread_local_index_hashmap.emplace(this, unregister_t(set_index, shared_locks_array_ptr));
      // remove info about deleted contfree-mutexes
      for (auto it = thread_local_index_hashmap.begin(), ite = thread_local_index_hashmap.end(); it != ite;) {
        if (it->second.array_slock_ptr->at(it->second.thread_index) < 0)    // if contfree-mtx was deleted
          it = thread_local_index_hashmap.erase(it);
        else
          ++it;
      }
    }
    return set_index;
  }


  public:

  contention_free_shared_mutex() 
    : shared_locks_array_ptr(std::make_shared<array_slock_t>()), 
      shared_locks_array(*shared_locks_array_ptr), 
      want_x_lock(false), 
      recursive_xlock_count(0),
		  owner_thread_id(thread_id_t()) {   
    for (auto &i : shared_locks_array) i = 0;
  }

  ~contention_free_shared_mutex() {
    for (auto &i : shared_locks_array) i = -1;
  }

  bool unregister_thread() { 
    return get_or_set_index(unregister_thread_op) >= 0; 
  }

  int register_thread() {
    int idx = get_or_set_index();
    if(idx > -1
       || shared_locks_array_ptr.use_count() > (int)shared_locks_array.size())
      return idx;

    // try once to register thread
    int unregistred_value = 0;
    for (idx = 0; idx < shared_locks_array.size(); ++idx) {
      if (shared_locks_array[idx] == 0
        && shared_locks_array[idx].compare_exchange_strong(unregistred_value, 1)) {
        get_or_set_index(register_thread_op, idx);   // thread registred success
        return idx;
      }
    }
    idx = -1;
    return idx;
  }

  void lock_shared() {
    int const register_index = register_thread();

    if (register_index >= 0) {
      int recursion_depth = shared_locks_array[register_index].load(std::memory_order_acquire);
      assert(recursion_depth >= 1);

      if (recursion_depth > 1)
        shared_locks_array[register_index].store(recursion_depth + 1, std::memory_order_release); // if recursive -> release
      else {
        shared_locks_array[register_index].store(recursion_depth + 1, std::memory_order_seq_cst); // if first -> sequential
        while(want_x_lock.load(std::memory_order_seq_cst)) {
          shared_locks_array[register_index].store(recursion_depth, std::memory_order_seq_cst);
          for (volatile size_t i = 0; want_x_lock.load(std::memory_order_seq_cst); ++i) 
						if (i % 100000 == 0) 
              std::this_thread::yield();
            shared_locks_array[register_index].store(recursion_depth + 1, std::memory_order_seq_cst);
        }
      }
      // (shared_locks_array[register_index] == 2 && want_x_lock == false) ||     // first shared lock
      // (shared_locks_array[register_index] > 2)                                 // recursive shared lock

    } else {
			if (owner_thread_id.load(std::memory_order_acquire) != get_fast_this_thread_id()) {
				size_t i = 0;
				for (bool flag = false; !want_x_lock.compare_exchange_weak(flag, true, std::memory_order_seq_cst); flag = false)
				if (++i % 100000 == 0) 
          std::this_thread::yield();
				owner_thread_id.store(get_fast_this_thread_id(), std::memory_order_release);
			}
			++recursive_xlock_count;
    }
  }

  void unlock_shared() {
    const int register_index = get_or_set_index();
    if(register_index >= 0) {
      const int recursion_depth = shared_locks_array[register_index].load(std::memory_order_acquire);
      assert(recursion_depth > 1);
      shared_locks_array[register_index].store(recursion_depth - 1, std::memory_order_release);
    
    } else if(!--recursive_xlock_count) {
			owner_thread_id.store(thread_id_t(), std::memory_order_release);
			want_x_lock.store(false, std::memory_order_release);
    }
  }

  void lock() {
    // forbidden upgrade S-lock to X-lock - this is an excellent opportunity to get deadlock
    int const register_index = get_or_set_index();
    if (register_index >= 0)
      assert(shared_locks_array[register_index].load(std::memory_order_acquire) == 1);

		if (owner_thread_id.load(std::memory_order_acquire) != get_fast_this_thread_id()) {
			size_t i = 0;
			for (bool flag = false; !want_x_lock.compare_exchange_weak(flag, true, std::memory_order_seq_cst); flag = false)
			  if (++i % 1000000 == 0) 
          std::this_thread::yield();
			owner_thread_id.store(get_fast_this_thread_id(), std::memory_order_release);

			for (auto &i : shared_locks_array)
				while (i.load(std::memory_order_seq_cst) > 1);
		}

		++recursive_xlock_count;
  }

  void unlock() {
    assert(recursive_xlock_count > 0);

    if (!--recursive_xlock_count) {
			owner_thread_id.store(thread_id_t(), std::memory_order_release);
			want_x_lock.store(false, std::memory_order_release);			
    }
  }

};
///  end for test comparison 


auto delay_r = std::chrono::nanoseconds(1);
auto delay_w = std::chrono::nanoseconds(1);

struct Atomic {
  std::atomic<size_t> count = 1;
  const size_t read() {
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
};
struct Dict {
  size_t count = 1;
  std::mutex m;
  const size_t read() {
    std::lock_guard<std::mutex> lk(m);
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    std::lock_guard<std::mutex> lk(m);
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
};
struct RWDict {
  size_t count = 1;
  std::shared_mutex rw_m;
  const size_t read() {
    std::shared_lock<std::shared_mutex> lk(rw_m);
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    std::lock_guard<std::shared_mutex> lk(rw_m);
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
};
struct ReccDict {
  size_t count = 1;
  std::recursive_mutex rw_m;
  const size_t read() {
    std::lock_guard<std::recursive_mutex> lk(rw_m);
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    std::lock_guard<std::recursive_mutex> lk(rw_m);
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
};
struct ReccDictChk {
  size_t count = 1;
  std::recursive_mutex rw_m;
  const size_t read(int chk=2) {
    std::lock_guard<std::recursive_mutex> lk(rw_m);
    std::this_thread::sleep_for(delay_r);
    if(chk) {
      //write(0);
      return read(--chk);
    }
    return count;
  }
  void write(int chk=2) {
    std::lock_guard<std::recursive_mutex> lk(rw_m);
    std::this_thread::sleep_for(delay_w);
    count = 1;
    if(chk) {
      //read(0);
      write(--chk);
    }
  }
};


struct LockAtomicUnique {
  size_t count = 1;
  SWC::LockAtomic::Unique m_mutex;
  const size_t read() {
    SWC::LockAtomic::Unique::Scope lk(m_mutex);
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    SWC::LockAtomic::Unique::Scope lk(m_mutex);
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
};
struct LockAtomicRecursive {
  size_t count = 1;
  SWC::LockAtomic::Recursive m_mutex;
  const size_t read() {
    SWC::LockAtomic::Recursive::Scope lk(m_mutex);
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    SWC::LockAtomic::Recursive::Scope lk(m_mutex);
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
};

struct LockAtomicRecursiveChk {
  size_t count = 1;
  SWC::LockAtomic::Recursive m_mutex;
  const size_t read(int chk=2) {
    SWC::LockAtomic::Recursive::Scope lk(m_mutex);
    std::this_thread::sleep_for(delay_r);
    if(chk) {
      //write(0);
      return read(--chk);
    }
    return count;
  }
  void write(int chk=2) {
    SWC::LockAtomic::Recursive::Scope lk(m_mutex);
    std::this_thread::sleep_for(delay_w);
    count = 1;
    if(chk) {
      //read(0);
      write(--chk);
    }
  }
};

struct LockAtomicRW {
  size_t count = 1;
  SWC::LockAtomic::RW m_mutex;
  const size_t read() {
    SWC::LockAtomic::RW::SharedScope lk(m_mutex);
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    SWC::LockAtomic::RW::UniqueScope lk(m_mutex);
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
}; 


struct LockAtomicRWChk {
  size_t count = 1;
  SWC::LockAtomic::RW m_mutex;
  const size_t read(int chk=2) {
    SWC::LockAtomic::RW::SharedScope lk(m_mutex);
    std::this_thread::sleep_for(delay_r);
    if(chk) {
      //write(0);
      return read(--chk);
    }
    return count;
  }
  void write(int chk=2) {
    SWC::LockAtomic::RW::UniqueScope lk(m_mutex);
    std::this_thread::sleep_for(delay_w);
    count = 1;
    if(chk) {
      //read(0);
      write(--chk);
    }
  }
}; 

struct ContFree {
  size_t count = 1;
  using Mutex = contention_free_shared_mutex<>;
  Mutex m_mutex;
  const size_t read() {
    SWC::SharedLock<Mutex> lk(m_mutex);
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    SWC::UniqueLock<Mutex>  lk(m_mutex);
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
}; 


/*
struct MDict {
  size_t count = 1;
  SWC::Mutex m_mutex;
  const size_t read() {
    SWC::SharedLock lk(m_mutex);
    std::this_thread::sleep_for(delay_r);
    return count;
  }
  void write() {
    SWC::UniqueLock lk(m_mutex);
    std::this_thread::sleep_for(delay_w);
    count = 1;
  }
};
*/



template<typename D>
int ProfileCounterCode(int work_load = 10000, int read_threads = 3, int write_threads = 3, 
                       uint64_t update_wait = 0) {
    D counter;
    std::vector<std::future<size_t>> futures;
    std::vector<std::thread> writeThreads;
    for (int n = 0; n < read_threads; ++n) {
      std::packaged_task<size_t()> task([&]{
        size_t counted = 0;
        for (int k = 0; k < work_load; ++k)
          counted += counter.read();
        return counted;
      }); // wrap the function
      std::future<size_t> f1 = task.get_future();  // get a future
      futures.emplace_back(std::move(f1));
      std::thread t(std::move(task)); // launch on a thread
      t.detach();
    }
    
    for (int n = 0; n < write_threads; ++n) {
      writeThreads.emplace_back(std::thread([&]{
        for (int k = 0; k < work_load; ++k) {
          if(update_wait)
            std::this_thread::sleep_for(std::chrono::nanoseconds(update_wait));
          counter.write();
        }
      }));
    }
    
    for (auto& t : writeThreads) t.join();
    
    size_t whole_len = 0;
    for (auto& f : futures) {
      f.wait();
      whole_len += f.get();
    }
    
    assert(whole_len == work_load*read_threads);
    return whole_len;
}

template<typename D>
void ProfileAndPring(std::string msg) {
    using milli = std::chrono::milliseconds;
    auto start = std::chrono::high_resolution_clock::now();
    int whole_len = ProfileCounterCode<D>();
    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << msg
              << " : "
              << std::chrono::duration_cast<milli>(finish - start).count()
              << "ms, result="<< whole_len
              << "\n";          
}

int main() {
  std::cout << " size of:\n";
  std::cout << " # std::mutex                     = " << sizeof(std::mutex) << "\n";
  std::cout << " # std::shared_mutex              = " << sizeof(std::shared_mutex) << "\n";
  std::cout << " # std::recursive_mutex           = " << sizeof(std::recursive_mutex) << "\n";
  std::cout << " # contention_free_shared_mutex   = " << sizeof(contention_free_shared_mutex<>) << "\n";
  
  std::cout << " # SWC::LockAtomic::Unique        = " << sizeof(SWC::LockAtomic::Unique) << "\n";
  std::cout << " # SWC::LockAtomic::Recursive     = " << sizeof(SWC::LockAtomic::Recursive) << "\n";
  std::cout << " # SWC::LockAtomic::RW            = " << sizeof(SWC::LockAtomic::RW) << "\n";
  //std::cout << " # SWC::Mutex             = " << sizeof(SWC::Mutex) << "\n";
  
  /*
  std::cout << " test-expect-deadlock: \n ";
  std::cout << "  SWC::LockAtomic::Unique \n";
  SWC::LockAtomic::Unique m;
  SWC::LockAtomic::Unique::Scope lk1(m);
  std::thread t([m=&m](){ 
    std::cout << "  thread 1 \n";
    SWC::LockAtomic::Unique::Scope lk2(*m, 2000);
    std::cout << "  thread 1 BAD\n";
  });
  t.join();
  std::this_thread::sleep_for(std::chrono::seconds(3));
  */
  

  for (int t = 0; t < 5; ++t) {
    std::cout << " check " << t << ":\n";  
    ProfileAndPring<Atomic>                 ("# std::atomic                     ");

    ProfileAndPring<LockAtomicRW>           ("# SWC::LockAtomic::RW             ");  
    ProfileAndPring<LockAtomicRWChk>        ("# SWC::LockAtomic::RW/chk         ");   
    ProfileAndPring<LockAtomicUnique>       ("# SWC::LockAtomic::Unique         ");   
    ProfileAndPring<LockAtomicRecursive>    ("# SWC::LockAtomic::Recursive      ");   
    ProfileAndPring<LockAtomicRecursiveChk> ("# SWC::LockAtomic::Recursive/chk  "); 

    ProfileAndPring<ContFree>               ("# contention_free_shared_mutex    "); 

    ProfileAndPring<Dict>                   ("# std::mutex                      ");
    ProfileAndPring<RWDict>                 ("# std::shared_mutex               ");
    ProfileAndPring<ReccDict>               ("# std::recursive_mutex            ");
    ProfileAndPring<ReccDictChk>            ("# std::recursive_mutex/chk        ");  
  }

}