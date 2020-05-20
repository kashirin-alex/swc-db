/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_Semaphore_h
#define swc_core_Semaphore_h


#include <mutex>
#include <condition_variable>


namespace SWC { 
  
class Semaphore final {
  public:

  explicit Semaphore(size_t sz=1);

  Semaphore(const Semaphore&) = delete;

  Semaphore(const Semaphore&&) = delete;
    
  Semaphore& operator=(const Semaphore&) = delete;
  
  ~Semaphore();

  void acquire();

  void release();

  void wait_until_under_and_acquire(size_t sz);

  void wait_until_under(size_t sz);

  void wait_all();

  private:
  std::mutex                m_mutex;
  std::condition_variable   m_cv;
  const size_t              m_sz;
  size_t                    m_count;
};

}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Semaphore.cc"
#endif 

#endif // swc_core_Semaphore_h
