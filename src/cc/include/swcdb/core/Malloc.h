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



#if !defined(__clang__)

extern void* operator new(size_t sz) __attribute__((__nothrow__));
extern void* operator new[](size_t sz) __attribute__((__nothrow__));

#else

#define SWC_INLINE_NEW_DEL
extern void* operator new(size_t sz);
extern void* operator new[](size_t sz);

#endif


extern void operator delete(void* ptr) noexcept;
extern void operator delete[](void* ptr) noexcept;

extern void operator delete(void* ptr, size_t sz) noexcept;
extern void operator delete[](void* ptr, size_t sz) noexcept;




/*! @} End of Core Group*/



namespace SWC { }


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Malloc.cc"
#endif

#endif // swcdb_core_Malloc_h
