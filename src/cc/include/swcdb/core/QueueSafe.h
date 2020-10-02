/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_QueueSafe_h
#define swcdb_core_QueueSafe_h

#include <queue>
#include "swcdb/core/MutexSptd.h"
  
namespace SWC { namespace Core {


template <class ItemT>
class QueueSafe : private std::queue<ItemT> {
  typedef std::queue<ItemT> QBase;

  public:

  explicit QueueSafe() { }

  ~QueueSafe() { }
  
  QueueSafe(const QueueSafe&) = delete;

  QueueSafe(const QueueSafe&&) = delete;
    
  QueueSafe& operator=(const QueueSafe&) = delete;

  void push(const ItemT& item) {
    auto support(m_mutex.lock());
    QBase::push(item);
    m_mutex.unlock(support);
  }

  bool push_and_is_1st(const ItemT& item) {
    bool chk;
    MutexSptd::scope lock(m_mutex);
    chk = QBase::empty();
    QBase::push(item);
    return chk;
  }

  ItemT& front() {
    MutexSptd::scope lock(m_mutex);
    return QBase::front();
  }

  bool empty() {
    MutexSptd::scope lock(m_mutex);
    return QBase::empty();
  }

  size_t size() {
    MutexSptd::scope lock(m_mutex);
    return QBase::size();
  }

  bool pop_and_more() {
    MutexSptd::scope lock(m_mutex);
    QBase::pop();
    return !QBase::empty();
  }

  bool pop(ItemT* item) {
    MutexSptd::scope lock(m_mutex);
    if(QBase::empty())
      return false;
    *item = QBase::front();
    QBase::pop();
    return true;
  }


  private:

  MutexSptd   m_mutex;

};



}} // namespace SWC::Core


#endif // swcdb_core_QueueSafe_h
