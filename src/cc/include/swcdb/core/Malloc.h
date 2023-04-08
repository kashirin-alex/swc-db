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



namespace SWC {

class Memory {
  public:

  static SWC_MALLOC_FUNC
  void* allocate(const size_t sz) noexcept;

  static SWC_MALLOC_FUNC
  void* allocate(const size_t sz, std::align_val_t al) noexcept;

  static
  void free(void* ptr) noexcept;

};

}



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
  return SWC::Memory::allocate(sz);
}

SWC_MALLOC_ATTRIBS
void* operator new(size_t sz, const std::nothrow_t&) {
  return SWC::Memory::allocate(sz);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz) {
  return SWC::Memory::allocate(sz);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz, const std::nothrow_t&) {
  return SWC::Memory::allocate(sz);
}


SWC_MALLOC_ATTRIBS
void* operator new(const size_t sz, std::align_val_t al) {
  return SWC::Memory::allocate(sz, al);
}

SWC_MALLOC_ATTRIBS
void* operator new(size_t sz, std::align_val_t al, const std::nothrow_t&) {
  return SWC::Memory::allocate(sz, al);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz, std::align_val_t al) {
  return SWC::Memory::allocate(sz, al);
}

SWC_MALLOC_ATTRIBS
void* operator new[](size_t sz, std::align_val_t al, const std::nothrow_t&) {
  return SWC::Memory::allocate(sz, al);
}



SWC_MALLOC_ATTRIBS
void operator delete(void* ptr) noexcept {
  SWC::Memory::free(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr) noexcept {
  SWC::Memory::free(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete(void* ptr, size_t ) noexcept {
  SWC::Memory::free(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr, size_t ) noexcept {
  SWC::Memory::free(ptr);
}


SWC_MALLOC_ATTRIBS
void operator delete(void* ptr, std::align_val_t) noexcept {
  SWC::Memory::free(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr, std::align_val_t) noexcept {
  SWC::Memory::free(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete(void* ptr, size_t, std::align_val_t) noexcept {
  SWC::Memory::free(ptr);
}

SWC_MALLOC_ATTRIBS
void operator delete[](void* ptr, size_t, std::align_val_t) noexcept {
  SWC::Memory::free(ptr);
}


/*! @} End of Core Group*/




namespace SWC { }

//#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Malloc.cc"
//#endif

#endif // swcdb_core_Malloc_h
