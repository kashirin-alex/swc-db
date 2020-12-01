/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Malloc_h
#define swcdb_core_Malloc_h


/*!
 *  \addtogroup Core
 *  @{
 */



extern void* operator new(size_t sz) __attribute__((__nothrow__));
extern void* operator new[](size_t sz) __attribute__((__nothrow__));
// __noclone__

extern void operator delete(void* ptr) noexcept;
extern void operator delete[](void* ptr) noexcept;

extern void operator delete(void* ptr, size_t sz) noexcept;
extern void operator delete[](void* ptr, size_t sz) noexcept;


#include <new>
#include <thread>
#include <chrono>


inline void* operator new(size_t sz) {
  //printf("New-Malloc size=%lu\n", sz);
  void *ptr;
  again: try {
    if(!(ptr = std::malloc(sz)) && sz) { // !sz, nullptr for zero bytes
      printf("Bad-Malloc size=%lu\n", sz);
      std::this_thread::sleep_for(std::chrono::nanoseconds(sz));
      goto again;
    }
  } catch(...) {
    goto again;
  }
  return ptr;
}

inline void* operator new[](size_t sz) {
  //printf("Malloc using new[] size=%lu\n", sz);
  return ::operator new(sz);
}


inline void operator delete(void* ptr) noexcept {
  std::free(ptr);
}

inline void operator delete[](void* ptr) noexcept {
  //printf("Malloc using delete[]\n");
  ::operator delete(ptr);
}

inline void operator delete(void* ptr, size_t ) noexcept {
  //printf("Malloc using delete size=%lu\n", sz);
  ::operator delete(ptr);
}

inline void operator delete[](void* ptr, size_t ) noexcept {
  //printf("Malloc using delete[] size=%lu\n", sz);
  ::operator delete(ptr);
}


/*! @} End of Core Group*/



namespace SWC { }


#endif // swcdb_core_Malloc_h
