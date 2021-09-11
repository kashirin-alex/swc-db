/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Buffer_h
#define swcdb_core_Buffer_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Exception.h"


namespace SWC { namespace Core {


template<typename T>
struct Buffer {

  typedef T                         value_type;
  typedef Buffer<value_type>        BufferT;
  typedef std::shared_ptr<BufferT>  Ptr;


  SWC_CAN_INLINE
  static value_type* allocate(size_t sz) {
    SWC_EXPECT(sz, Error::BAD_MEMORY_ALLOCATION);
    return new value_type[sz];
  }

  constexpr SWC_CAN_INLINE
  explicit Buffer() SWC_NOEXCEPT
                  : own(false), size(0), base(nullptr) {
  }

  SWC_CAN_INLINE
  Buffer(size_t sz)
        : own(sz), size(sz), base(size ? allocate(size) : nullptr) {
  }

  constexpr SWC_CAN_INLINE
  Buffer(value_type* data, size_t sz, bool take_ownership) SWC_NOEXCEPT
        : own(take_ownership), size(sz), base(data) {
  }

  constexpr SWC_CAN_INLINE
  Buffer(BufferT& other) SWC_NOEXCEPT
        : own(other.own), size(other.size), base(other.base) {
    if(own) {
      other.own = false;
      other.base = nullptr;
    }
  }

  constexpr SWC_CAN_INLINE
  Buffer(BufferT&& other) SWC_NOEXCEPT
        : own(other.own), size(other.size), base(other.base) {
    if(own) {
      other.own = false;
      other.size = 0;
      other.base = nullptr;
    }
  }

  template<typename OtherT>
  constexpr
  Buffer(OtherT& other) SWC_NOEXCEPT;


  SWC_CAN_INLINE
  ~Buffer() {
    _free();
  }

  SWC_CAN_INLINE
  void _free() SWC_NOEXCEPT {
    if(own && base)
      delete [] base;
  }

  SWC_CAN_INLINE
  void free() SWC_NOEXCEPT {
    if(base) {
      if(own)
        delete [] base;
      base = nullptr;
    }
    size = 0;
  }

  SWC_CAN_INLINE
  void reallocate(size_t len) {
    if(len) {
      _free();
      own = true;
      base = allocate(size = len);
    } else {
      free();
    }
  }

  SWC_CAN_INLINE
  void assign(const value_type* data, size_t len) {
    reallocate(len);
    if(len)
      memcpy(base, data, len);
  }

  void set(value_type* data, size_t len, bool take_ownership) SWC_NOEXCEPT {
    _free();
    own = take_ownership;
    size = len;
    base = data;
  }

  void set(BufferT& other) SWC_NOEXCEPT {
    _free();
    base = other.base;
    size = other.size;
    if((own = other.own)) {
      other.own = false;
      other.base = nullptr;
    }
  }

  template<typename OtherT>
  void set(OtherT& other) SWC_NOEXCEPT;

  bool          own;
  size_t        size : 56;
  value_type*   base;

};



template<typename BufferT>
struct BufferDyn : BufferT {

  using value_type  = typename BufferT::value_type;
  typedef std::shared_ptr<BufferDyn> Ptr;


  constexpr SWC_CAN_INLINE
  explicit BufferDyn() SWC_NOEXCEPT
                    : ptr(nullptr), mark(nullptr)  {
  }

  constexpr SWC_CAN_INLINE
  BufferDyn(size_t sz)
            : BufferT(sz), ptr(BufferT::base), mark(BufferT::base) {
  }

  constexpr SWC_CAN_INLINE
  BufferDyn(BufferDyn&& other) SWC_NOEXCEPT
            : BufferT(std::move(other)), ptr(other.ptr), mark(other.mark) {
    other.ptr = other.mark = nullptr;
  }

  //~BufferDyn() { }

  SWC_CAN_INLINE
  void free() {
    BufferT::free();
    ptr = mark = nullptr;
  }

  constexpr
  value_type* release(size_t* lenp) SWC_NOEXCEPT {
    value_type* rbuf = BufferT::base;
    if(lenp)
      *lenp = fill();
    BufferT::size = 0;
    BufferT::base = ptr = mark = nullptr;
    return rbuf;
  }

  constexpr SWC_CAN_INLINE
  size_t remaining() const SWC_NOEXCEPT {
    return ptr ? BufferT::size - fill() : 0;
  }

  constexpr SWC_CAN_INLINE
  size_t fill() const SWC_NOEXCEPT {
    return ptr - BufferT::base;
  }

  constexpr SWC_CAN_INLINE
  bool empty() const SWC_NOEXCEPT {
    return ptr == BufferT::base;
  }

  constexpr SWC_CAN_INLINE
  void set_mark() SWC_NOEXCEPT {
    mark = ptr;
  }

  constexpr SWC_CAN_INLINE
  void clear() SWC_NOEXCEPT {
    ptr = BufferT::base;
  }

  SWC_CAN_INLINE
  void ensure(size_t len) {
    if(!len)
      return;
    if(!BufferT::base) {
      BufferT::own = true;
      ptr = mark = BufferT::base = BufferT::allocate(BufferT::size = len);

    } else {
      size_t ptr_offset = ptr - BufferT::base;
      size_t remain = BufferT::size - ptr_offset;
      if(len > remain) {
        len -= remain;
        const value_type* base_old = BufferT::base;
        size_t            size_old = BufferT::size;
        BufferT::size += len;
        BufferT::base = BufferT::allocate(BufferT::size);
        memcpy(BufferT::base, base_old, size_old);
        if(BufferT::own)
          delete [] base_old;
        else
          BufferT::own = true;
        mark = BufferT::base + (mark - base_old);
        ptr = BufferT::base + ptr_offset;
      }
    }
  }

  SWC_CAN_INLINE
  value_type* add_unchecked(const value_type* data, size_t len) SWC_NOEXCEPT {
    value_type* rptr = ptr;
    if(len) {
      memcpy(ptr, data, len);
      ptr += len;
    }
    return rptr;
  }

  value_type* add(const value_type* data, size_t len) {
    ensure(len);
    return add_unchecked(data, len);
  }

  SWC_CAN_INLINE
  value_type* add(const std::string& data) {
    return add(
      reinterpret_cast<const value_type*>(data.c_str()), data.length());
  }

  void add(const value_type data) {
    ensure(1);
    *ptr = data;
    ++ptr;
  }

  void set(const value_type* data, size_t len) {
    clear();
    ensure(len);
    add_unchecked(data, len);
  }

  void take_ownership(BufferDyn<BufferT>& other) SWC_NOEXCEPT {
    BufferT::_free();
    BufferT::own = other.own;
    BufferT::size = other.size;
    other.size = 0;
    BufferT::base = other.base;
    ptr = other.ptr;
    mark = other.mark;
    other.base = other.ptr = other.mark = nullptr;
  }

  void take_ownership(BufferT& other) SWC_NOEXCEPT {
    BufferT::_free();
    BufferT::own = other.own;
    BufferT::size = other.size;
    BufferT::base = ptr = mark = other.base;
    ptr += other.size;
    other.base = nullptr;
    other.size = 0;
  }

  value_type*  ptr;
  value_type*  mark;
};


typedef Buffer <uint8_t>          StaticBuffer;
typedef BufferDyn <StaticBuffer>  DynamicBuffer;



// StaticBuffer specializations to DynamicBuffer
template<>
template<>
constexpr SWC_CAN_INLINE
StaticBuffer::Buffer(DynamicBuffer& other) SWC_NOEXCEPT
                    : own(other.own), size(other.fill()), base(other.base) {
  if(own) {
    other.own = false;
    other.size = 0;
    other.base = other.ptr = other.mark = nullptr;
  }
}

template<>
template<>
SWC_CAN_INLINE
void StaticBuffer::set(DynamicBuffer& other) SWC_NOEXCEPT {
  _free();
  base = other.base;
  size = other.fill();
  if((own = other.own)) {
    other.own = false;
    other.size = 0;
    other.base = other.ptr = other.mark = nullptr;
  }
}

} // namespace Core




/*!
 *  \addtogroup Core
 *  @{
 */

using StaticBuffer = Core::StaticBuffer;
using DynamicBuffer = Core::DynamicBuffer;

/*! @} End of Core Group*/



} // namespace SWC


#endif // swcdb_core_Buffer_h
