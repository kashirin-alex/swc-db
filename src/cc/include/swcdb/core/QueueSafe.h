/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_QueueSafe_h
#define swcdb_core_QueueSafe_h


#include "swcdb/core/MutexSptd.h"
#include <queue>


namespace SWC { namespace Core {


template <class ItemT>
class QueueSafe : private std::queue<ItemT> {
  typedef std::queue<ItemT> QBase;

  public:

  typedef ItemT  value_type;

  SWC_CAN_INLINE
  explicit QueueSafe() noexcept { }

  ~QueueSafe() noexcept { }

  QueueSafe(const QueueSafe&) = delete;

  QueueSafe(QueueSafe&& other) {
    MutexSptd::scope lock(other.m_mutex);
    QBase::operator=(std::move(other));
  }

  QueueSafe& operator=(const QueueSafe&) = delete;

  QueueSafe& operator=(QueueSafe&&) = delete;

  SWC_CAN_INLINE
  void push(const ItemT& item) {
    MutexSptd::scope lock(m_mutex);
    QBase::push(item);
  }

  SWC_CAN_INLINE
  void push(ItemT&& item) {
    MutexSptd::scope lock(m_mutex);
    QBase::push(std::move(item));
  }

  SWC_CAN_INLINE
  bool push_and_is_1st(const ItemT& item) {
    bool chk;
    MutexSptd::scope lock(m_mutex);
    chk = QBase::empty();
    QBase::push(item);
    return chk;
  }

  SWC_CAN_INLINE
  bool push_and_is_1st(ItemT&& item) {
    bool chk;
    MutexSptd::scope lock(m_mutex);
    chk = QBase::empty();
    QBase::push(std::move(item));
    return chk;
  }

  SWC_CAN_INLINE
  ItemT& front() noexcept {
    MutexSptd::scope lock(m_mutex);
    return QBase::front();
  }

  SWC_CAN_INLINE
  bool empty() noexcept {
    MutexSptd::scope lock(m_mutex);
    return QBase::empty();
  }

  SWC_CAN_INLINE
  size_t size() noexcept {
    MutexSptd::scope lock(m_mutex);
    return QBase::size();
  }

  SWC_CAN_INLINE
  bool pop_and_more() {
    MutexSptd::scope lock(m_mutex);
    QBase::pop();
    return !QBase::empty();
  }

  SWC_CAN_INLINE
  bool pop(ItemT* item) {
    MutexSptd::scope lock(m_mutex);
    if(QBase::empty())
      return false;
    *item = std::move(QBase::front());
    QBase::pop();
    return true;
  }


  private:

  MutexSptd   m_mutex;

};



}} // namespace SWC::Core


#endif // swcdb_core_QueueSafe_h
