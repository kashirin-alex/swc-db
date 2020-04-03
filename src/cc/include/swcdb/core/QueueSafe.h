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

  void push(const ItemT& item) {
    m_mutex.lock();
    QBase::push(item);
    m_mutex.unlock();
  }

  bool push_and_is_1st(const ItemT& item) {
    bool chk;
    m_mutex.lock();
    chk = QBase::empty();
    QBase::push(item);
    m_mutex.unlock();
    return chk;
  }

  ItemT& front() {
    LockAtomic::Unique::Scope lock(m_mutex);
    return QBase::front();
  }

  bool empty() {
    bool chk;
    m_mutex.lock();
    chk = QBase::empty();
    m_mutex.unlock();
    return chk;
  }

  size_t size() {
    size_t chk;
    m_mutex.lock();
    chk = QBase::size();
    m_mutex.unlock();
    return chk;
  }

  bool pop_and_more() {
    bool chk;
    m_mutex.lock();
    pop(); 
    chk = !QBase::empty();
    m_mutex.unlock();
    return chk;
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
