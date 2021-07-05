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

  typedef PtrT  value_type;

  struct Pointer {
    constexpr SWC_CAN_INLINE
    Pointer() noexcept : _other(nullptr) { }
    PtrT _other;
  };

  constexpr SWC_CAN_INLINE
  explicit QueuePointer() noexcept : _back(nullptr), _front(nullptr) { }

  QueuePointer(const QueuePointer&) = delete;

  QueuePointer(const QueuePointer&&) = delete;

  QueuePointer& operator=(const QueuePointer&) = delete;


  SWC_CAN_INLINE
  bool push_and_is_1st(PtrT& ptr) noexcept {
    lock();
    bool has(_front);
    _push(ptr);
    unlock();
    return !has;
  }

  SWC_CAN_INLINE
  void push(PtrT& ptr) noexcept {
    lock();
    _push(ptr);
    unlock();
  }

  SWC_CAN_INLINE
  bool empty() noexcept {
    lock();
    bool has(_front);
    unlock();
    return !has;
  }

  SWC_CAN_INLINE
  PtrT front() noexcept {
    PtrT ptr;
    lock();
    ptr = _front;
    unlock();
    return ptr;
  }

  SWC_CAN_INLINE
  bool pop_and_more() noexcept {
    lock();
    _front = std::move(_front->_other);
    bool more(_front);
    if(!more)
      _back = nullptr;
    unlock();
    return more;
  }

  SWC_CAN_INLINE
  bool pop(PtrT* ptr) noexcept {
    lock();
    *ptr = std::move(_front);
    if(*ptr && !(_front = std::move((*ptr)->_other)))
      _back = nullptr;
    unlock();
    return bool(*ptr);
  }

  SWC_CAN_INLINE
  PtrT next() noexcept {
    lock();
    PtrT ptr(std::move(_front));
    if(ptr && !(_front = std::move(ptr->_other)))
      _back = nullptr;
    unlock();
    return ptr;
  }


  private:

  constexpr SWC_CAN_INLINE
  void _push(PtrT& ptr) noexcept {
    (_back ? _back->_other : _front) = ptr;
    _back = ptr;
  }

  PtrT  _back;
  PtrT  _front;

};


}} // namespace SWC::Core


#endif // swcdb_core_QueuePointer_h
