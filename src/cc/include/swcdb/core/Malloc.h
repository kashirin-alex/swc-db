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
  void* operator new(const size_t sz)
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
  void* operator new(const size_t sz, std::align_val_t al)
  SWC_MALLOC_NEW_ATTRIBS;
extern SWC_MALLOC_ATTRIBS
  void* operator new[](size_t sz, std::align_val_t al)
  SWC_MALLOC_NEW_ATTRIBS;
extern SWC_MALLOC_ATTRIBS
  void* operator new(size_t sz, std::align_val_t al, const std::nothrow_t&)
  SWC_MALLOC_NEW_ATTRIBS;
extern SWC_MALLOC_ATTRIBS
  void* operator new[](size_t sz, std::align_val_t al, const std::nothrow_t&)
  SWC_MALLOC_NEW_ATTRIBS;


extern SWC_MALLOC_ATTRIBS
  void operator delete(void* ptr) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete[](void* ptr) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete(void* ptr, size_t sz) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete[](void* ptr, size_t sz) noexcept;

extern SWC_MALLOC_ATTRIBS
  void operator delete(void* ptr, std::align_val_t al) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete[](void* ptr, std::align_val_t al) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete(void* ptr, size_t sz, std::align_val_t al) noexcept;
extern SWC_MALLOC_ATTRIBS
  void operator delete[](void* ptr, size_t sz, std::align_val_t al) noexcept;



SWC_MALLOC_ATTRIBS
void* operator new(const size_t sz) {
  //printf("malloc using new size=" SWC_FMT_LU "\n", sz);
  for(;;) {
    void* ptr = std::malloc(sz);
    if(ptr || !sz) // !sz, nullptr for zero bytes
      return ptr;
    printf("Bad-malloc size=" SWC_FMT_LU "\n", sz);
    std::this_thread::sleep_for(std::chrono::nanoseconds(sz));
  }
}

SWC_MALLOC_ATTRIBS
void* operator new(size_t sz, const std::nothrow_t&) {
  return ::operator new(sz);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz) {
  //printf("Malloc using new[] size=" SWC_FMT_LU "\n", sz);
  return ::operator new(sz);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz, const std::nothrow_t&) {
  //printf("Malloc using new[] size=" SWC_FMT_LU "\n", sz);
  return ::operator new(sz);
}


SWC_MALLOC_ATTRIBS
void* operator new(const size_t sz, std::align_val_t al) {
  //printf("aligned_alloc using new size=" SWC_FMT_LU " aligned=" SWC_FMT_LU "\n",
  //       sz,uint64_t(al));
  for(;;) {
    void* ptr = std::aligned_alloc(size_t(al), sz);
    if(ptr || !sz) // !sz, nullptr for zero bytes
      return ptr;
    printf("Bad-aligned_alloc size=" SWC_FMT_LU " aligned=" SWC_FMT_LU "\n",
            sz, uint64_t(al));
    std::this_thread::sleep_for(std::chrono::nanoseconds(sz));
  }
}

SWC_MALLOC_ATTRIBS
void* operator new(size_t sz, std::align_val_t al, const std::nothrow_t&) {
  return ::operator new(sz, al);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz, std::align_val_t al) {
  //printf("Malloc using new[] size=" SWC_FMT_LU "\n", sz);
  return ::operator new(sz, al);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz, std::align_val_t al, const std::nothrow_t&) {
  //printf("Malloc using new[] size=" SWC_FMT_LU "\n", sz);
  return ::operator new(sz, al);
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
  //printf("Malloc using delete size=" SWC_FMT_LU "\n", sz);
  ::operator delete(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr, size_t ) noexcept {
  //printf("Malloc using delete[] size=" SWC_FMT_LU "\n", sz);
  ::operator delete(ptr);
}


SWC_MALLOC_ATTRIBS
void operator delete(void* ptr, std::align_val_t) noexcept {
  //printf("Malloc using align-delete\n");
  std::free(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr, std::align_val_t al) noexcept {
  //printf("Malloc using delete[]\n");
  ::operator delete(ptr, al);
}

SWC_MALLOC_ATTRIBS
void operator delete(void* ptr, size_t, std::align_val_t al) noexcept {
  //printf("Malloc using delete size=" SWC_FMT_LU "\n", sz);
  ::operator delete(ptr, al);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr, size_t, std::align_val_t al) noexcept {
  //printf("Malloc using delete[] size=" SWC_FMT_LU "\n", sz);
  ::operator delete(ptr, al);
}


/*! @} End of Core Group*/




namespace SWC { }


#endif // swcdb_core_Malloc_h
