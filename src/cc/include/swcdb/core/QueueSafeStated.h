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
    m_mutex.lock();
    QBase::push(item);
    m_mutex.unlock();
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

  bool is_active() {
    bool chk;
    m_mutex.lock();
    chk = m_state;
    m_mutex.unlock();
    return chk;
  }

  bool activating() {
    bool chk;
    m_mutex.lock();
    chk = (m_state || QBase::empty()) ? false : m_state = true;
    m_mutex.unlock();
    return chk;
  }

  bool deactivating() {
    bool chk;
    m_mutex.lock();
    pop();
    if(QBase::empty())
      m_state = false;
    chk = !m_state;
    m_mutex.unlock();
    return chk;
  }

  void deactivate() {
    m_mutex.lock();
    m_state = false;
    m_mutex.unlock();
  }

  bool activating(const ItemT& item) {
    bool chk;
    m_mutex.lock();
    if(m_state) {
      QBase::push(item);
      chk = false;
    } else {
      chk = m_state = true;
    }
    m_mutex.unlock();
    return chk;
  }

  bool deactivating(ItemT* item) {
    bool chk;
    m_mutex.lock();
    if(QBase::empty()) {
      m_state = false;
    } else {
      *item = QBase::front();
      pop();
    }
    chk = !m_state;
    m_mutex.unlock();
    return chk;
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
