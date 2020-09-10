/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_Malloc_H
#define swc_core_Malloc_H


extern 
void* operator new(unsigned long sz) __attribute__((__nothrow__));


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


#endif
