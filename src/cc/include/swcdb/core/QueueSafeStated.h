/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_QueueSafeStated_h
#define swc_core_QueueSafeStated_h

#include <queue>
#include "swcdb/core/LockAtomicUnique.h"

namespace SWC { 


template <class ItemT>
class QueueSafeStated : private std::queue<ItemT> {
  public:

  void push(const ItemT& item) {
    LockAtomic::Unique::Scope lock(m_mutex);
    QBase::push(item);
  }

  ItemT& front() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::front();
  }

  bool empty() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::empty();
  }

  size_t size() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::size();
  }

  bool activating() {
    LockAtomic::Unique::Scope lock(m_mutex);
    if(m_state || QBase::empty()) 
      return false;
    return m_state = true;
  }

  bool deactivating() {
    LockAtomic::Unique::Scope lock(m_mutex);
    pop();
    if(QBase::empty())
      m_state = false;
    return !m_state;
  }

  void deactivate() {
    LockAtomic::Unique::Scope lock(m_mutex);
    m_state = false;
  }

  private:
  LockAtomic::Unique        m_mutex;
  bool                      m_state = false;

  typedef std::queue<ItemT> QBase;
  using QBase::empty;
  using QBase::size;
  using QBase::front;
  using QBase::pop;
};


}

#endif // swc_core_QueueSafeStated_h
