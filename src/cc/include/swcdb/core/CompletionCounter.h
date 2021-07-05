/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_CompletionCounter_h
#define swcdb_core_CompletionCounter_h


#include "swcdb/core/Compat.h"


namespace SWC { namespace Core {

template<class CountT=uint32_t>
struct CompletionCounter final: private Core::Atomic<CountT> {

  constexpr SWC_CAN_INLINE
  explicit CompletionCounter(CountT start=0) noexcept
                            : Core::Atomic<CountT>(start) {
  };

  CompletionCounter(const CompletionCounter&) = delete;

  CompletionCounter(const CompletionCounter&&) = delete;

  CompletionCounter& operator=(const CompletionCounter&) = delete;

  //~CompletionCounter() noexcept { }

  constexpr SWC_CAN_INLINE
  void increment(CountT v=1) noexcept {
    Core::Atomic<CountT>::fetch_add(v);
  }

  constexpr SWC_CAN_INLINE
  bool is_last() noexcept {
    return Core::Atomic<CountT>::fetch_sub(1) == 1;
  }

  constexpr SWC_CAN_INLINE
  CountT count() const noexcept {
    return Core::Atomic<CountT>::load();
  }

  constexpr SWC_CAN_INLINE
  CountT increment_and_count() noexcept {
    return Core::Atomic<CountT>::add_rslt(1);
  }

  constexpr SWC_CAN_INLINE
  CountT decrement_and_count() noexcept {
    return Core::Atomic<CountT>::sub_rslt(1);
  }

};

}} // namespace SWC::Core



#endif // swcdb_core_CompletionCounter_h
