/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_QueueSafeStated_h
#define swcdb_core_QueueSafeStated_h


#include "swcdb/core/MutexSptd.h"
#include <queue>


namespace SWC { namespace Core {


template <class ItemT>
class QueueSafeStated final : private std::queue<ItemT> {
  public:

  explicit QueueSafeStated() noexcept { }

  //~QueueSafeStated() { }

  QueueSafeStated(const QueueSafeStated&) = delete;

  QueueSafeStated(const QueueSafeStated&&) = delete;

  QueueSafeStated& operator=(const QueueSafeStated&) = delete;


  void push(const ItemT& item) {
    auto support(m_mutex.lock());
    QBase::push(item);
    m_mutex.unlock(support);
  }

  void push(ItemT&& item) {
    auto support(m_mutex.lock());
    QBase::push(std::move(item));
    m_mutex.unlock(support);
  }


  ItemT& front() noexcept {
    MutexSptd::scope lock(m_mutex);
    return QBase::front();
  }

  bool empty() noexcept {
    MutexSptd::scope lock(m_mutex);
    return QBase::empty();
  }

  size_t size() noexcept {
    MutexSptd::scope lock(m_mutex);
    return QBase::size();
  }

  bool is_active() noexcept {
    MutexSptd::scope lock(m_mutex);
    return m_state;
  }

  bool activating() noexcept {
    MutexSptd::scope lock(m_mutex);
    return (m_state || QBase::empty()) ? false : (m_state = true);
  }

  bool deactivating() {
    MutexSptd::scope lock(m_mutex);
    QBase::pop();
    if(QBase::empty())
      m_state = false;
    return !m_state;
  }

  void deactivate() noexcept {
    MutexSptd::scope lock(m_mutex);
    m_state = false;
  }


  bool activating(const ItemT& item) {
    MutexSptd::scope lock(m_mutex);
    if(m_state) {
      QBase::push(item);
      return false;
    }
    return m_state = true;
  }

  bool deactivating(ItemT* item) {
    MutexSptd::scope lock(m_mutex);
    if(QBase::empty()) {
      m_state = false;
    } else {
      *item = QBase::front();
      QBase::pop();
    }
    return !m_state;
  }


  bool activating(ItemT& item) {
    MutexSptd::scope lock(m_mutex);
    if(m_state) {
      QBase::push(std::move(item));
      return false;
    }
    return m_state = true;
  }

  bool deactivating(ItemT& item) {
    MutexSptd::scope lock(m_mutex);
    if(QBase::empty()) {
      m_state = false;
    } else {
      item = std::move(QBase::front());
      QBase::pop();
    }
    return !m_state;
  }


  private:
  MutexSptd                 m_mutex;
  bool                      m_state = false;

  typedef std::queue<ItemT> QBase;
};



}} // namespace SWC::Core


#endif // swcdb_core_QueueSafeStated_h
