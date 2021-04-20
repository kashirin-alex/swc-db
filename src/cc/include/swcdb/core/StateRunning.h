/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_StateRunning_h
#define swcdb_core_StateRunning_h


#include "swcdb/core/Compat.h"


namespace SWC { namespace Core {


struct StateRunning final : private AtomicBool {

  SWC_CAN_INLINE
  explicit StateRunning(bool initial=false) noexcept
                        : AtomicBool(initial) {
  }

  StateRunning(const StateRunning&) = delete;

  StateRunning(const StateRunning&&) = delete;

  StateRunning& operator=(const StateRunning&) = delete;

  //SWC_CAN_INLINE
  //~StateRunning() noexcept { }

  SWC_CAN_INLINE
  void stop() noexcept {
    store(false);
  }

  SWC_CAN_INLINE
  bool running() noexcept {
    bool at = false;
    return !compare_exchange_weak(at, true);
  }

  SWC_CAN_INLINE
  operator bool() const noexcept {
    return load();
  }

};

}} // namespace SWC::Core


#endif // swcdb_core_StateRunning_h
