/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_QueuePointer_h
#define swcdb_core_QueuePointer_h

#include "swcdb/core/MutexAtomic.h"
  
namespace SWC { namespace Core {


template <typename PtrT>
class QueuePointer : private MutexAtomic {
  public:

  struct Pointer {
    Pointer() noexcept : _other(nullptr) { }
    PtrT _other;
  };

  explicit QueuePointer() noexcept : _back(nullptr), _front(nullptr) { }

  QueuePointer(const QueuePointer&) = delete;

  QueuePointer(const QueuePointer&&) = delete;

  QueuePointer& operator=(const QueuePointer&) = delete;


  bool push_and_is_1st(PtrT& ptr) noexcept {
    lock();
    bool has(_front);
    _push(ptr);
    unlock();
    return !has;
  }

  void push(PtrT& ptr) noexcept {
    lock();
    _push(ptr);
    unlock();
  }

  bool empty() noexcept {
    lock();
    bool has(_front);
    unlock();
    return !has;
  }

  PtrT front() noexcept {
    PtrT ptr;
    lock();
    ptr = _front;
    unlock();
    return ptr;
  }

  bool pop_and_more() noexcept {
    lock();
    _front = std::move(_front->_other);
    bool more(_front);
    if(!more)
      _back = nullptr;
    unlock();
    return more;
  }

  bool pop(PtrT* ptr) noexcept {
    lock();
    *ptr = std::move(_front);
    if(*ptr && !(_front = std::move((*ptr)->_other)))
      _back = nullptr;
    unlock();
    return bool(*ptr);
  }

  PtrT next() noexcept {
    lock();
    PtrT ptr(std::move(_front));
    if(ptr && !(_front = std::move(ptr->_other)))
      _back = nullptr;
    unlock();
    return ptr;
  }


  private:

  SWC_CAN_INLINE
  void _push(PtrT& ptr) noexcept {
    (_back ? _back->_other : _front) = ptr;
    _back = ptr;
  }

  PtrT  _back;
  PtrT  _front;

};


}} // namespace SWC::Core


#endif // swcdb_core_QueuePointer_h
