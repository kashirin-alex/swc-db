/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Array_h
#define swcdb_core_Array_h


#include <array>


namespace SWC { namespace Core {



template<typename ValueT, typename SizeT, SizeT SIZE>
class Array : private std::array<ValueT, SIZE> {

  constexpr static bool is_SimpleType = std::is_pointer_v<ValueT> ||
                                        std::is_integral_v<ValueT> ||
                                        std::is_reference_v<ValueT>;
  SizeT _size;
  using _Array = std::array<ValueT, SIZE>;

  public:
  static_assert(SizeT(-1) > 0, "SWC-DB Array supports only unsigned size!");

  using value_type = _Array::value_type;
  using size_type = SizeT;

  using typename _Array::const_iterator;
  using typename _Array::iterator;
  using _Array::begin;
  using _Array::cbegin;
  using _Array::front;
  using _Array::operator[];


  Array(const Array& other)             = delete;
  Array(Array&& other)                  = delete;
  Array& operator=(Array&& other)       = delete;
  Array& operator=(const Array& other)  = delete;


  SWC_CAN_INLINE
  Array() noexcept(is_SimpleType) : _size(0) { }

  SWC_CAN_INLINE
  Array(size_type sz, const ValueT& value) noexcept(is_SimpleType)
        : _size(sz) {
    for(auto ptr=_Array::data(); sz; ++ptr, --sz)
      *ptr = value;
  }

  SWC_CAN_INLINE
  Array(Array&& other, iterator it_b) noexcept(is_SimpleType)
        : _size(other.cend() - it_b) {
    const_iterator it_e = other.cend();
    for(auto ptr=_Array::data(); it_b < it_e; ++ptr, ++it_b)
      *ptr = std::move(*it_b);
    other._size -= _size;
  }

  //~Array() { }

  SWC_CAN_INLINE
  size_type size() const noexcept {
    return _size;
  }

  SWC_CAN_INLINE
  bool empty() const noexcept {
    return !_size;
  }

  SWC_CAN_INLINE
  bool full() const noexcept {
    return _size == SIZE;
  }

  SWC_CAN_INLINE
  ValueT& back() noexcept {
    return *(_Array::begin() + _size - 1);
  }

  SWC_CAN_INLINE
  const ValueT& back() const noexcept {
    return *(_Array::cbegin() + _size - 1);
  }

  SWC_CAN_INLINE
  iterator end() noexcept {
    return _Array::begin() + _size;
  }

  SWC_CAN_INLINE
  const_iterator end() const noexcept {
    return cend();
  }

  SWC_CAN_INLINE
  const_iterator cend() const noexcept {
    return _Array::cbegin() + _size;
  }


  SWC_CAN_INLINE
  size_t at(size_t pos, iterator& it) noexcept {
    if(pos >= _size)
      return _size;
    it = begin() + pos;
    return pos;
  }

  SWC_CAN_INLINE
  size_t at(size_t pos, const_iterator& it) const noexcept {
    if(pos >= _size)
      return _size;
    it = cbegin() + pos;
    return pos;
  }


  SWC_CAN_INLINE
  void insert(iterator it, ValueT&& value) noexcept {
    size_type offset = it - _Array::cbegin();
    if(_size > offset)
      _alter(it, _size - offset, 1);
    ++_size;
    *it = std::move(value);
  }

  SWC_CAN_INLINE
  void insert(iterator it, const ValueT& value) noexcept(is_SimpleType) {
    size_type offset = it - _Array::cbegin();
    if(_size > offset)
      _alter(it, _size - offset, 1);
    ++_size;
    *it = value;
  }

  SWC_CAN_INLINE
  void insert(iterator it, const_iterator first, const_iterator last)
                                              noexcept(is_SimpleType) {
    if(first < last) {
      size_type sz = last - first;
      size_type offset = it - _Array::cbegin();
      if(_size > offset)
        _alter(it, _size - offset, sz);
      _size += sz;
      for(size_type i=0; i<sz; ++it, ++i)
        *it = first[i];
    }
  }


  SWC_CAN_INLINE
  void push_back(ValueT&& value) noexcept {
    *(_Array::data() + (_size++)) = std::move(value);
  }

  SWC_CAN_INLINE
  void push_back(const ValueT& value) noexcept(is_SimpleType) {
    *(_Array::data() + (_size++)) = value;
  }


  SWC_CAN_INLINE
  void erase(iterator it, size_type amt=1) noexcept {
    _size -= amt;
    for(size_type remain=_size-(it-_Array::cbegin()); remain; --remain, ++it)
      *it = std::move(*(it + amt));
  }

  SWC_CAN_INLINE
  void erase(iterator first, iterator last) noexcept {
    erase(first, last - first);
  }

  private:

  SWC_CAN_INLINE
  static void _alter(iterator it, size_type remain, size_type amt) noexcept {
    iterator it_prev = it + remain - 1;
    for(it = it_prev + amt; remain; --remain, --it, --it_prev)
      *it = std::move(*it_prev);
  }


};


}} // namespace SWC::Core



#endif // swcdb_core_Array_h
