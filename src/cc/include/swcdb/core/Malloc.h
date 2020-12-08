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


/* Declare the Memory Allocator (Memory Manager)
   define MACROS: 
    SWCDB_MEM_ALLOC   - allocate n bytes
    SWCDB_MEM_FREE    - free pointer
    SWCDB_MEM_RELEASE - release n bytes to OS
*/

#if defined SWC_MALLOC
  #include "swcdb/core/MallocImpl.h"
  #define SWCDB_MEM_ALLOC(sz)   SWC::Core::Malloc::allocate(sz)
  #define SWCDB_MEM_FREE(ptr)   SWC::Core::Malloc::free(ptr)
  #define SWCDB_MEM_RELEASE(sz) SWC::Core::Malloc::release(sz)

#elif defined MIMALLOC
  #include <mimalloc.h> // -override
  #define SWCDB_MEM_ALLOC(sz)   mi_malloc(sz)
  #define SWCDB_MEM_FREE(ptr)   mi_free(ptr)
  #define SWCDB_MEM_RELEASE(sz) mi_collect(true);
  #if defined SWC_MALLOC_NOT_INSISTENT
    #include <mimalloc-new-delete.h>
  #endif

#elif defined TCMALLOC_MINIMAL || defined TCMALLOC
  #include <gperftools/malloc_extension.h>
  #define SWCDB_MEM_ALLOC(sz)   std::malloc(sz)
  #define SWCDB_MEM_FREE(ptr)   std::free(ptr)
  #define SWCDB_MEM_RELEASE(sz) MallocExtension::instance()->ReleaseFreeMemory()

#else
  #define SWCDB_MEM_ALLOC(sz)   std::malloc(sz)
  #define SWCDB_MEM_FREE(ptr)   std::free(ptr)
  #define SWCDB_MEM_RELEASE(sz)

#endif




// override new/delete for insistent allocation
#ifndef SWC_MALLOC_NOT_INSISTENT


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


static void* swcdb_malloc_insistent(size_t sz) noexcept
  __attribute__((__nothrow__, __noclone__));

static void* swcdb_malloc_insistent(size_t sz) noexcept {
  void *ptr;
  again: try {
    if(!(ptr = SWCDB_MEM_ALLOC(sz)) && sz) { // !sz, nullptr for zero bytes
      printf("Bad-Malloc size=%lu\n", sz);
      std::this_thread::sleep_for(std::chrono::nanoseconds(sz));
      goto again;
    }
  } catch(...) {
    goto again;
  }
  return ptr;
}


inline void* operator new(size_t sz) {
  //printf("New-Malloc size=%lu\n", sz);
  return swcdb_malloc_insistent(sz);
}

inline void* operator new[](size_t sz) {
  //printf("Malloc using new[] size=%lu\n", sz);
  return swcdb_malloc_insistent(sz);
}


inline void operator delete(void* ptr) noexcept {
  SWCDB_MEM_FREE(ptr);
}

inline void operator delete[](void* ptr) noexcept {
  //printf("Malloc using delete[]\n");
  SWCDB_MEM_FREE(ptr);
}

inline void operator delete(void* ptr, size_t ) noexcept {
  //printf("Malloc using delete size=%lu\n", sz);
  SWCDB_MEM_FREE(ptr);
}

inline void operator delete[](void* ptr, size_t ) noexcept {
  //printf("Malloc using delete[] size=%lu\n", sz);
  SWCDB_MEM_FREE(ptr);
}



#endif // ifndef SWC_MALLOC_NOT_INSISTENT




/*! @} End of Core Group*/



namespace SWC { }


#endif // swcdb_core_Malloc_h
