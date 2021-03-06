/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Malloc_h
#define swcdb_core_Malloc_h

#include <new>
#include <thread>
#include <chrono>

/*!
 *  \addtogroup Core
 *  @{
 */



#if !defined(__clang__)
#define SWC_MALLOC_NEW_ATTRIBS __attribute__((__nothrow__))
#define SWC_MALLOC_ATTRIBS SWC_CAN_INLINE
#else
#define SWC_MALLOC_NEW_ATTRIBS
#define SWC_MALLOC_ATTRIBS
#endif



extern SWC_MALLOC_ATTRIBS
  void* operator new(size_t sz)
  SWC_MALLOC_NEW_ATTRIBS;
extern SWC_MALLOC_ATTRIBS
  void* operator new[](size_t sz)
  SWC_MALLOC_NEW_ATTRIBS;
extern SWC_MALLOC_ATTRIBS
  void* operator new(size_t sz, const std::nothrow_t&)
  SWC_MALLOC_NEW_ATTRIBS;
extern SWC_MALLOC_ATTRIBS
  void* operator new[](size_t sz, const std::nothrow_t&)
  SWC_MALLOC_NEW_ATTRIBS;

extern SWC_MALLOC_ATTRIBS
  void operator delete(void* ptr) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete[](void* ptr) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete(void* ptr, size_t sz) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete[](void* ptr, size_t sz) noexcept;


SWC_MALLOC_ATTRIBS
void* operator new(size_t sz) {
  //printf("Malloc using new size=%lu\n", sz);
  _do: try {
    void* ptr = std::malloc(sz);
    if(ptr || !sz) // !sz, nullptr for zero bytes
      return ptr;
  } catch(...) { }
  printf("Bad-Malloc size=%lu\n", sz);
  std::this_thread::sleep_for(std::chrono::nanoseconds(sz));
  goto _do;
}

SWC_MALLOC_ATTRIBS
void* operator new(size_t sz, const std::nothrow_t&) {
  return ::operator new(sz);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz) {
  //printf("Malloc using new[] size=%lu\n", sz);
  return ::operator new(sz);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz, const std::nothrow_t&) {
  //printf("Malloc using new[] size=%lu\n", sz);
  return ::operator new(sz);
}


SWC_MALLOC_ATTRIBS
void operator delete(void* ptr) noexcept {
  //printf("Malloc using delete\n");
  std::free(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr) noexcept {
  //printf("Malloc using delete[]\n");
  ::operator delete(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete(void* ptr, size_t ) noexcept {
  //printf("Malloc using delete size=%lu\n", sz);
  ::operator delete(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr, size_t ) noexcept {
  //printf("Malloc using delete[] size=%lu\n", sz);
  ::operator delete(ptr);
}


/*! @} End of Core Group*/




namespace SWC { }


#endif // swcdb_core_Malloc_h
