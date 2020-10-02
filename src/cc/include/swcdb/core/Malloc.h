/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Malloc_h
#define swcdb_core_Malloc_h


extern 
void* operator new(unsigned long sz) 
    __attribute__((__nothrow__)); // __noclone__


#include <new>
#include <thread>
#include <chrono>


inline
void* operator new(unsigned long sz) {
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


namespace SWC { }


#endif // swcdb_core_Malloc_h
