/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_QueueSafeStated_h
#define swc_core_QueueSafeStated_h

#include <queue>
#include "swcdb/core/Mutex.h"

namespace SWC { 


template <class ItemT>
class QueueSafeStated final : private std::queue<ItemT> {
  public:

  explicit QueueSafeStated() { }

  ~QueueSafeStated() { }

  QueueSafeStated(const QueueSafeStated&) = delete;

  QueueSafeStated(const QueueSafeStated&&) = delete;
    
  QueueSafeStated& operator=(const QueueSafeStated&) = delete;

  void push(const ItemT& item) {
    auto support(m_mutex.lock());
    QBase::push(item);
    m_mutex.unlock(support);
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

  bool is_active() {
    Mutex::scope lock(m_mutex);
    return m_state;
  }

  bool activating() {
    Mutex::scope lock(m_mutex);
    return (m_state || QBase::empty()) ? false : (m_state = true);
  }

  bool deactivating() {
    Mutex::scope lock(m_mutex);
    QBase::pop();
    if(QBase::empty())
      m_state = false;
    return !m_state;
  }

  void deactivate() {
    Mutex::scope lock(m_mutex);
    m_state = false;
  }

  bool activating(const ItemT& item) {
    Mutex::scope lock(m_mutex);
    if(m_state) {
      QBase::push(item);
      return false;
    }
    return m_state = true;
  }

  bool deactivating(ItemT* item) {
    Mutex::scope lock(m_mutex);
    if(QBase::empty()) {
      m_state = false;
    } else {
      *item = QBase::front();
      QBase::pop();
    }
    return !m_state;
  }

  private:
  Mutex                     m_mutex;
  bool                      m_state = false;

  typedef std::queue<ItemT> QBase;
};


}

#endif // swc_core_QueueSafeStated_h
