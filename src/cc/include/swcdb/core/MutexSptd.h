/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Mutex_h
#define swcdb_core_Mutex_h

#include <mutex>
#include "swcdb/core/MutexAtomic.h"


namespace SWC { namespace Core {

class MutexSptd final : private MutexAtomic {
  public:

  explicit MutexSptd() noexcept { }

  MutexSptd(const MutexSptd&) = delete;

  MutexSptd(const MutexSptd&&) = delete;

  MutexSptd& operator=(const MutexSptd&) = delete;

  //~MutexSptd() noexcept { }

  bool lock() {
    if(MutexAtomic::try_lock())
      return true;
    m_mutex.lock();
    MutexAtomic::lock();
    return false;
  }

  bool try_full_lock(bool& support) {
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

  void unlock(const bool& support) {
    MutexAtomic::unlock();
    if(!support)
      m_mutex.unlock();
  }

  class scope final {
    public:

    scope(MutexSptd& m) : _m(m), _support(m.lock()) { }

    ~scope() { _m.unlock(_support); }

    scope(const scope&) = delete;

    scope(const scope&&) = delete;

    scope& operator=(const scope&) = delete;

    private:
    MutexSptd&   _m;
    const bool   _support;
  };

  private:
  std::mutex m_mutex;
};



}} //namespace SWC::Core


#endif // swcdb_core_Mutex_h
