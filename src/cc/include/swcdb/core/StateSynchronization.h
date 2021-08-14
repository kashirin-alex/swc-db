/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_StateSynchronization_h
#define swcdb_core_StateSynchronization_h


#include "swcdb/core/Compat.h"


namespace SWC { namespace Core {


class StateSynchronization {
  public:

  SWC_CAN_INLINE
  StateSynchronization() noexcept : m_ack(false) { }

  SWC_CAN_INLINE
  void wait() {
    Core::UniqueLock lock_wait(m_mutex);
    m_cv.wait(lock_wait, [this] { return m_ack; });
  }

  SWC_CAN_INLINE
  void acknowledge() noexcept {
    Core::UniqueLock lock_wait(m_mutex);
    m_ack = true;
    m_cv.notify_all();
  }

  SWC_CAN_INLINE
  void reset() noexcept {
    m_ack = false;
  }

  private:
  std::mutex                        m_mutex;
  std::condition_variable           m_cv;
  bool                              m_ack;

};



}}


#endif // swcdb_core_StateSynchronization_h
