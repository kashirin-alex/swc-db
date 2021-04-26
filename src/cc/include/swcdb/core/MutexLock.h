/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_MutexLock_h
#define swcdb_core_MutexLock_h


#include <mutex>
#include <shared_mutex>



namespace SWC { namespace Core {



struct SharedLock final : public std::shared_lock<std::shared_mutex> {

  //SWC_SHOULD_NOT_INLINE
  SharedLock(std::shared_mutex& m) noexcept
            : std::shared_lock<std::shared_mutex>(m, std::defer_lock) {
    _again: try {
      std::shared_lock<std::shared_mutex>::lock();
    } catch(...) {
      goto _again;
    }
  }

  //~SharedLock() noexcept { }
  SharedLock(const SharedLock&)             = delete;
  SharedLock(SharedLock&&)                  = delete;
  SharedLock& operator=(const SharedLock&)  = delete;
  SharedLock& operator=(SharedLock&&)       = delete;
};



template<typename MutexT>
struct ScopedLock final {

  MutexT& _m;

  //SWC_SHOULD_NOT_INLINE
  ScopedLock(MutexT& m) noexcept : _m(m) {
    _again: try {
      _m.lock();
    } catch(...) {
      goto _again;
    }
  }

  ~ScopedLock() noexcept {
    _m.unlock();
  }

  ScopedLock(const ScopedLock&)             = delete;
  ScopedLock(ScopedLock&&)                  = delete;
  ScopedLock& operator=(const ScopedLock&)  = delete;
  ScopedLock& operator=(ScopedLock&&)       = delete;
};



template<typename MutexT>
struct UniqueLock final : public std::unique_lock<MutexT> {

  //SWC_SHOULD_NOT_INLINE
  UniqueLock(MutexT& m) noexcept
            : std::unique_lock<MutexT>(m, std::defer_lock) {
    _again: try {
      std::unique_lock<MutexT>::lock();
    } catch(...) {
      goto _again;
    }
  }

  //~UniqueLock() noexcept { }
  UniqueLock(const UniqueLock&)             = delete;
  UniqueLock(UniqueLock&&)                  = delete;
  UniqueLock& operator=(const UniqueLock&)  = delete;
  UniqueLock& operator=(UniqueLock&&)       = delete;
};



}} //namespace SWC::Core



#endif // swcdb_core_MutexLock_h
