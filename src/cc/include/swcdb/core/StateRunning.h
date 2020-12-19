/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_StateRunning_h
#define swcdb_core_StateRunning_h


#include "swcdb/core/Compat.h"


namespace SWC { namespace Core {
  
  
class StateRunning final : private std::atomic<bool> {
  public:

  SWC_CAN_INLINE
  explicit StateRunning(bool initial=false) noexcept
                        : std::atomic<bool>(initial) {
  }

  StateRunning(const StateRunning&) = delete;

  StateRunning(const StateRunning&&) = delete;
    
  StateRunning& operator=(const StateRunning&) = delete;
  
  SWC_CAN_INLINE
  ~StateRunning() noexcept { }

  SWC_CAN_INLINE
  void stop() noexcept {
    store(false, std::memory_order_relaxed);
  }

  SWC_CAN_INLINE
  bool running() noexcept {
    bool at = false;
    return !compare_exchange_weak(at, true, std::memory_order_relaxed);
  }

  SWC_CAN_INLINE
  operator bool() const noexcept {
    return load(std::memory_order_relaxed);
  }

};

}} // namespace SWC::Core


#endif // swcdb_core_StateRunning_h
