/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Compat.h"
#include <new>
#include <thread>
#include <chrono>


#if !defined(__clang__)
#define SWC_INLINE_NEW_DEL SWC_SHOULD_INLINE
#else
#define SWC_INLINE_NEW_DEL
#endif



SWC_INLINE_NEW_DEL
void* operator new(size_t sz) {
  //printf("New-Malloc size=%lu\n", sz);
  _do: try {
    void* ptr = std::malloc(sz);
    if(ptr || !sz) // !sz, nullptr for zero bytes
      return ptr;
  } catch(...) { }
  printf("Bad-Malloc size=%lu\n", sz);
  std::this_thread::sleep_for(std::chrono::nanoseconds(sz));
  goto _do;
}

SWC_INLINE_NEW_DEL
void* operator new[](size_t sz) {
  //printf("Malloc using new[] size=%lu\n", sz);
  return ::operator new(sz);
}


SWC_INLINE_NEW_DEL
void operator delete(void* ptr) noexcept {
  std::free(ptr);
}

SWC_INLINE_NEW_DEL
void operator delete[](void* ptr) noexcept {
  //printf("Malloc using delete[]\n");
  ::operator delete(ptr);
}

SWC_INLINE_NEW_DEL
void operator delete(void* ptr, size_t ) noexcept {
  //printf("Malloc using delete size=%lu\n", sz);
  ::operator delete(ptr);
}

SWC_INLINE_NEW_DEL
void operator delete[](void* ptr, size_t ) noexcept {
  //printf("Malloc using delete[] size=%lu\n", sz);
  ::operator delete(ptr);
}


#undef SWC_INLINE_NEW_DEL
