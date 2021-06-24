/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Vector_h
#define swcdb_core_Vector_h

#include <vector>

namespace SWC { namespace Core {

template<typename T, typename SizeT=uint32_t>
class Vector {
  public:

  using value_type          = T;
  using size_type           = SizeT;

  typedef value_type*       pointer;
  typedef const value_type* const_pointer;

  typedef value_type*       iterator;
  typedef const value_type* const_iterator;

  typedef value_type&       reference;
  typedef const value_type& const_reference;


  SWC_CAN_INLINE
  static constexpr size_type max_size() noexcept {
    return size_type(0) - size_type(1);
  }


  SWC_CAN_INLINE
  Vector() noexcept : _data(nullptr), _cap(0), _size(0) { }

  SWC_CAN_INLINE
  Vector(size_type sz)
          : _data(sz ? _allocate(sz) : nullptr), _cap(sz), _size(sz) {
  }

  SWC_CAN_INLINE
  Vector(Vector&& other) noexcept
        : _data(other._data), _cap(other._cap), _size(other._size) {
    other._data = nullptr;
    other._cap = other._size = 0;
  }

  SWC_CAN_INLINE
  Vector(const Vector& other)
        : _data(
            other._size
              ? _copy(
                  _allocate_uinitialized(other._size),
                  other._data, other._size
                )
              : nullptr
          ),
          _cap(other._size), _size(other._size) {
  }

  SWC_CAN_INLINE
  ~Vector() {
    if(_data)
      _deallocate(_data, _cap);
  }

  SWC_CAN_INLINE
  void free() noexcept {
    if(_data) {
      _deallocate(_data, _cap);
      _data = nullptr;
      _size = _cap = 0;
    }
  }


  SWC_CAN_INLINE
  Vector& operator=(Vector&& other) noexcept {
    _data = other._data;
    _cap = other._cap;
    _size = other._size;
    other._data = nullptr;
    other._cap = other._size = 0;
    return *this;
  }

  SWC_CAN_INLINE
  Vector& operator=(const Vector& other) {
    free();
    if((_cap = _size = other._size)) {
      _data = _copy(_allocate_uinitialized(_size), other._data, _size);
    }
    return *this;
  }

  SWC_CAN_INLINE
  void swap(Vector& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_cap, other._cap);
    std::swap(_size, other._size);
  }

  SWC_CAN_INLINE
  bool empty() const noexcept {
    return !_size;
  }

  SWC_CAN_INLINE
  size_type size() const noexcept {
    return _size;
  }

  SWC_CAN_INLINE
  size_type capacity() const noexcept {
    return _cap;
  }


  SWC_CAN_INLINE
  pointer data() noexcept {
    return _data;
  }

  SWC_CAN_INLINE
  const_pointer data() const noexcept {
    return _data;
  }


  SWC_CAN_INLINE
  iterator begin() noexcept {
    return _data;
  }

  SWC_CAN_INLINE
  const_iterator cbegin() const noexcept {
    return _data;
  }

  SWC_CAN_INLINE
  const_iterator begin() const noexcept {
    return cbegin();
  }


  SWC_CAN_INLINE
  iterator end() noexcept {
    return _data + _size;
  }

  SWC_CAN_INLINE
  const_iterator cend() const noexcept {
    return _data + _size;
  }

  SWC_CAN_INLINE
  const_iterator end() const noexcept {
    return cend();
  }


  SWC_CAN_INLINE
  reference front() noexcept {
    return *_data;
  }

  SWC_CAN_INLINE
  const_reference front() const noexcept {
    return *_data;
  }


  SWC_CAN_INLINE
  reference back() noexcept {
    return _data[_size - 1];
  }

  SWC_CAN_INLINE
  const_reference back() const noexcept {
    return _data[_size - 1];
  }


  SWC_CAN_INLINE
  reference operator[](size_type pos) noexcept {
    return _data[pos];
  }

  SWC_CAN_INLINE
  const_reference operator[](size_type pos) const noexcept {
    return _data[pos];
  }


  SWC_CAN_INLINE
  void clear() noexcept {
    for(pointer ptr = _data; _size; --_size, ++ptr)
      ptr->~value_type();
  }

  void shrink_to_fit(size_type sz=0) {
    if(_cap > sz || _cap > _size) {
      if(_size) {
        _cap = sz > _size ? sz : _size;
        _data = _allocate_uinitialized(_data, _size, _cap);
      } else {
        free();
      }
    }
  }

  SWC_CAN_INLINE
  void reserve(size_type sz) {
    if(_cap < sz)
      _data = _allocate_uinitialized(_data, _size, (_cap = sz));
  }

  template<typename... ArgsT>
  SWC_CAN_INLINE
  void resize(size_type sz, ArgsT&&... args) {
    if(sz > _size) {
      reserve(sz);
      for(pointer ptr = _data + _size; _size < sz; ++_size, ++ptr)
        _construct(ptr, std::forward<ArgsT>(args)...);
    } else if(sz < _size) {
      for(pointer ptr = _data + sz; sz < _size; --_size, ++ptr)
        ptr->~value_type();
    }
  }


  template<typename... ArgsT>
  SWC_CAN_INLINE
  void push_back(ArgsT&&... args) {
    reserve(_size + 1);
    _construct(_data + _size, std::forward<ArgsT>(args)...);
    ++_size;
  }

  template<typename... ArgsT>
  SWC_CAN_INLINE
  reference emplace_back(ArgsT&&... args) {
    reserve(_size + 1);
    reference ref = *_construct(_data + _size, std::forward<ArgsT>(args)...);
    ++_size;
    return ref;
  }

  template<typename... ArgsT>
  SWC_CAN_INLINE
  iterator insert(const_iterator it, ArgsT&&... args) {
    size_type offset;
    if(!_size || (offset = it - _data) == _size)
      return &emplace_back(std::forward<ArgsT>(args)...);

    reserve(_size + 1);
    return _construct(
      _alter(_data + offset, (_size++) - offset, 1), std::forward<ArgsT>(args)...);
  }

  SWC_CAN_INLINE
  iterator insert(const_iterator it, iterator first, iterator last) {
    if(!_size) {
      assign(first, last);
      return _data;
    }

    size_type offset = it - _data;
    size_type amt = last - first;
    reserve(_size + amt);

    pointer data_offset = _alter(_data + offset, _size - offset, amt);
    _size += amt;
    for(pointer ptr=data_offset; first != last; ++first, ++ptr)
      _construct(ptr, *first);
    return data_offset;
  }

  SWC_CAN_INLINE
  void assign(const_iterator first, const_iterator last) {
    clear();
    size_type sz = last - first;
    reserve(sz);
    _size = sz;
    for(size_type i = 0; i < sz; ++i)
      _construct(_data + i, first[i]);
  }


  SWC_CAN_INLINE
  iterator erase(const_iterator it) {
    size_type offset = it - _data;
    if(offset >= _size)
      return end();

    --_size;
    pointer ptr = _data + offset;
    for(size_type remain= _size - offset; remain; --remain, ++ptr)
      *ptr = std::move(*(ptr + 1));
    return _data + offset;
  }

  SWC_CAN_INLINE
  iterator erase(const_iterator first, const_iterator last) {
    size_type offset = first - _data;
    if(offset >= _size)
      return end();

    size_type amt = last - first;
    _size -= amt;
    pointer ptr = _data + offset;
    for(size_type remain = _size - offset; remain; --remain, ++ptr)
      *ptr = std::move(*(ptr + amt));
    return _data + offset;
  }

  private:


  SWC_CAN_INLINE
  static pointer _allocate_uinitialized(size_type size) {
    return static_cast<pointer>(::operator new(sizeof(value_type) * size));
    //return static_cast<pointer>(
      //std::aligned_alloc(alignof(value_type), sizeof(value_type) * size));
  }

  /* IF can grow in-place
  SWC_CAN_INLINE
  static pointer _allocate_uinitialized(pointer data_prev, size_t data_prev,
                                        size_type size) {
    pointer data = _allocate_uinitialized(size, data_prev);
    if(data != data_prev) {
      _move(data, data_prev, size_prev);
      _deallocate(data_prev, size_prev);
    }
    return data;
  */

  SWC_CAN_INLINE
  static pointer _allocate_uinitialized(pointer data_prev, size_t size_prev,
                                        size_type sz) {
    pointer data = _move(_allocate_uinitialized(sz), data_prev, size_prev);
    _deallocate(data_prev, size_prev);
    return data;
    //return static_cast<pointer>(
    //  std::realloc(data_prev, sizeof(value_type) * sz));
    //std::allocator<value_type> alloc;
    //pointer data = _move(alloc.allocate(sz), data_prev, size_prev);
    //_deallocate(data_prev, size_prev);
    //return data;
  }

  SWC_CAN_INLINE
  static void _deallocate(pointer data, size_t) {
    ::operator delete(data);
    //std::free(data);
    //std::allocator<value_type> alloc;
    //alloc.deallocate(data, sz);
  }


  SWC_CAN_INLINE
  static pointer _allocate(size_type sz) {
    pointer data = _allocate_uinitialized(sz);
    for(pointer ptr = data; sz; --sz, ++ptr)
      _construct(ptr);
    //std::uninitialized_default_construct(ptr, ptr + sz);
    return data;
  }


  template<typename... ArgsT>
  SWC_CAN_INLINE
  static pointer _construct(pointer ptr, ArgsT&&... args) {
    new (ptr) value_type(std::forward<ArgsT>(args)...);
    return ptr;
  }


  SWC_CAN_INLINE
  static pointer _copy(pointer data,
                       const_pointer data_prev, size_type size_prev) {
    for(pointer ptr=data; size_prev; --size_prev, ++ptr, ++data_prev)
      _construct(ptr, *data_prev);
    //std::uninitialized_copy_n(data_prev, size_prev, data);
    return data;
  }

  SWC_CAN_INLINE
  static pointer _move(pointer data,
                       pointer data_prev, size_type size_prev) {
    for(pointer ptr=data; size_prev; --size_prev, ++ptr, ++data_prev)
      _construct(ptr, std::move(*data_prev));
    //std::uninitialized_move_n(data_prev, size_prev, data);
    return data;
  }

  SWC_CAN_INLINE
  static pointer _alter(pointer data, size_type remain, size_type amount) {
    pointer prev = data + remain - 1;
    for(pointer ptr = prev + amount; remain; --remain, --ptr, --prev) {
      _construct(ptr, std::move(*prev));
    }
    //std::uninitialized_move_n(prev, size, data);
    return data;
  }


  pointer     _data;
  size_type   _cap;
  size_type   _size;


  /*
  template<typename vT>
  SWC_CAN_INLINE
  T* find(const vT& value) {
    for(auto it=Get<Iterator>(value); it; ++it) {
      if(value == *it.item)
        return &*it.item;
    }
    return nullptr;
  }

  template<typename vT>
  SWC_CAN_INLINE
  const T* cfind(const vT& value) {
    for(auto it=Get<ConstIterator>(value); it; ++it) {
      if(value == *it.item)
        return &*it.item;
    }
    return nullptr;
  }
  */

};


}} // namespace SWC::Core



#endif // swcdb_core_Vector_h
