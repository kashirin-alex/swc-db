/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_QueueSafe_h
#define swc_core_QueueSafe_h

#include <queue>
#include "swcdb/core/Mutex.h"
  
namespace SWC { 


template <class ItemT>
class QueueSafe : private std::queue<ItemT> {
  public:

  void push(const ItemT& item) {
    auto support(m_mutex.lock());
    QBase::push(item);
    m_mutex.unlock(support);
  }

  bool push_and_is_1st(const ItemT& item) {
    bool chk;
    Mutex::scope lock(m_mutex);
    chk = QBase::empty();
    QBase::push(item);
    return chk;
  }

  ItemT& front() {
    Mutex::scope lock(m_mutex);
    return QBase::front();
  }

  bool empty() {
    Mutex::scope lock(m_mutex);
    return QBase::empty();
  }

  size_t size() {
    Mutex::scope lock(m_mutex);
    return QBase::size();
  }

  bool pop_and_more() {
    Mutex::scope lock(m_mutex);
    pop();
    return !QBase::empty();
  }

  private:
  Mutex                     m_mutex;

  typedef std::queue<ItemT> QBase;
  using QBase::empty;
  using QBase::size;
  using QBase::front;
  using QBase::pop;
};


}

#endif // swc_core_QueueSafe_h
