/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/Compat.h"



namespace SWC {


SWC_SHOULD_INLINE SWC_MALLOC_FUNC
void* Memory::allocate(const size_t sz) noexcept {
  for(;;) {
    void* ptr = malloc(sz);
    if(ptr || !sz) {// !sz, nullptr for zero bytes
      //printf(
      //  "alloc sz=" SWC_FMT_LU " p=" SWC_FMT_LU "\n",
      //  sz, uint64_t(ptr)
      //);
      return ptr;
    }
    printf(
      "Bad-alloc sz=" SWC_FMT_LU "\n",
      sz
    );
    std::this_thread::sleep_for(std::chrono::nanoseconds(sz));
  }
}

SWC_SHOULD_INLINE SWC_MALLOC_FUNC
void* Memory::allocate(const size_t sz, std::align_val_t al) noexcept {
  for(;;) {
    void* ptr = aligned_alloc(size_t(al), sz);
    if(ptr || !sz) {// !sz, nullptr for zero bytes
      //printf(
      //  "alloc sz=" SWC_FMT_LU " al=" SWC_FMT_LU " p=" SWC_FMT_LU "\n",
      //  sz, uint64_t(al), uint64_t(ptr)
      //);
      return ptr;
    }
    printf(
      "Bad-alloc sz=" SWC_FMT_LU " al=" SWC_FMT_LU "\n",
      sz, uint64_t(al)
    );
    std::this_thread::sleep_for(std::chrono::nanoseconds(sz));
  }
}


SWC_SHOULD_INLINE
void Memory::free(void* ptr) noexcept {
  //printf(
  //  "free p=" SWC_FMT_LU "\n",
  // uint64_t(ptr)
  //);
  std::free(ptr);
}


}
