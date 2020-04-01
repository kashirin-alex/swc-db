/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_QueueSafe_h
#define swc_core_QueueSafe_h

#include <queue>
#include "swcdb/core/LockAtomicUnique.h"

namespace SWC { 


template <class ItemT>
class QueueSafe : private std::queue<ItemT> {
  public:

  ItemT& front() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::front();
  }

  void push(const ItemT& item) {
    LockAtomic::Unique::Scope lock(m_mutex);
    QBase::push(item);
  }

  bool push_and_is_1st(const ItemT& item) {
    LockAtomic::Unique::Scope lock(m_mutex);
    bool was = QBase::empty();
    QBase::push(item);
    return was;
  }

  bool empty() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::empty();
  }

  size_t size() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::size();
  }

  bool pop_and_more() {
    LockAtomic::Unique::Scope lock(m_mutex);
    pop();
    return !QBase::empty();
  }

  private:
  LockAtomic::Unique        m_mutex;

  typedef std::queue<ItemT> QBase;
  using QBase::empty;
  using QBase::size;
  using QBase::front;
  using QBase::pop;
};


}

#endif // swc_core_QueueSafe_h
