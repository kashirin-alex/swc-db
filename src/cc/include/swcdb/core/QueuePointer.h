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
    Pointer() : _other(nullptr) { }
    PtrT _other;
  };

  explicit QueuePointer(): _back(nullptr), _front(nullptr) { }

  ~QueuePointer() { }
  
  QueuePointer(const QueuePointer&) = delete;

  QueuePointer(const QueuePointer&&) = delete;
    
  QueuePointer& operator=(const QueuePointer&) = delete;


  bool push_and_is_1st(PtrT ptr) {
    bool is_1st;
    lock();
    is_1st = !_front;
    _push(ptr);
    unlock();
    return is_1st;
  }

  void push(PtrT ptr) {
    lock();
    _push(ptr);
    unlock();
  }
 
  bool empty() {
    bool more;
    lock();
    more = _front;
    unlock();
    return !more;
  }

  PtrT front() {
    PtrT ptr;
    lock();
    ptr = _front;
    unlock();
    return ptr;
  }
  
  bool pop_and_more() {
    bool more;
    lock();
    if(!(more = (_front = _front->_other)))
      _back = nullptr;
    unlock();
    return more;
  }

  bool pop(PtrT* ptr) {
    *ptr = next();
    return *ptr;
  }

  PtrT next() {
    PtrT ptr;
    lock();
    if((ptr = _front) && !(_front = _front->_other))
      _back = nullptr;
    unlock();
    return ptr;
  }


  private:
  
  SWC_CAN_INLINE
  void _push(PtrT ptr) {
    (_back ? _back->_other : _front) = ptr;
    _back = ptr;
  }

  PtrT  _back;
  PtrT  _front;

};


}} // namespace SWC::Core


#endif // swcdb_core_QueuePointer_h
