/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Semaphore_h
#define swcdb_core_Semaphore_h


#include "swcdb/core/Compat.h"
#include <condition_variable>


namespace SWC { namespace Core {

class Semaphore final {
  public:

  explicit Semaphore(size_t sz=1, size_t pre_acquire = 0) noexcept;

  Semaphore(const Semaphore&) = delete;

  Semaphore(const Semaphore&&) = delete;

  Semaphore& operator=(const Semaphore&) = delete;

  ~Semaphore() noexcept;

  size_t available();

  bool has_pending();

  void acquire();

  void release();

  void wait_until_under_and_acquire(size_t sz);

  void wait_until_under(size_t sz);

  size_t wait_available();

  void wait_all();

  private:
  std::mutex                m_mutex;
  std::condition_variable   m_cv;
  const size_t              m_sz;
  size_t                    m_count;
};


}} //namespace SWC::Core



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Semaphore.cc"
#endif

#endif // swcdb_core_Semaphore_h
