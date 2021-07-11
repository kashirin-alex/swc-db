/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Vector_h
#define swcdb_core_Vector_h

#include <vector>

namespace SWC { namespace Core {

template<typename T, typename SizeT=uint32_t, SizeT GROW_SZ=0>
class Vector {
  constexpr static bool is_SimpleType = std::is_pointer_v<T> ||
                                        std::is_integral_v<T> ||
                                        std::is_reference_v<T>;
  public:

  using value_type          = T;
  using size_type           = SizeT;

  typedef value_type*       pointer;
  typedef const value_type* const_pointer;

  typedef value_type*       iterator;
  typedef const value_type* const_iterator;

  typedef value_type&       reference;
  typedef const value_type& const_reference;


  constexpr SWC_CAN_INLINE
  static size_type max_size() noexcept {
    return size_type(0) - size_type(1);
  }
  static_assert(max_size() > 0, "SWC-DB Vector supports only unsigned size!");


  constexpr SWC_CAN_INLINE
  Vector() noexcept : _data(nullptr), _cap(0), _size(0) { }

  template<typename... ArgsT>
  SWC_CAN_INLINE
  Vector(size_type sz, ArgsT&&... args)
          : _data(sz ? _allocate(sz, std::forward<ArgsT>(args)...) : nullptr),
            _cap(sz), _size(sz) {
  }

  constexpr SWC_CAN_INLINE
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
  Vector(const_iterator _b, const_iterator _e)
        : _data(nullptr), _cap(0), _size(0) {
    assign(_b, _e);
  }

  SWC_CAN_INLINE
  ~Vector() {
    if(_data) {
      if(!is_SimpleType) {
        for(auto& it : *this)
          it.~value_type();
      }
      _deallocate(_data, _cap);
    }
  }

  SWC_CAN_INLINE
  void clear() noexcept {
    if(is_SimpleType) {
      _size = 0;
    } else {
      for(pointer ptr = _data; _size; --_size, ++ptr)
        ptr->~value_type();
    }
  }

  SWC_CAN_INLINE
  void free() noexcept {
    if(_data) {
      clear();
      _deallocate(_data, _cap);
      _data = nullptr;
      _cap = 0;
    }
  }


  SWC_CAN_INLINE
  Vector& operator=(Vector&& other) noexcept {
    free();
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

  constexpr SWC_CAN_INLINE
  bool empty() const noexcept {
    return !_size;
  }

  constexpr SWC_CAN_INLINE
  size_type size() const noexcept {
    return _size;
  }

  constexpr SWC_CAN_INLINE
  size_type capacity() const noexcept {
    return _cap;
  }


  constexpr SWC_CAN_INLINE
  pointer data() noexcept {
    return _data;
  }

  constexpr SWC_CAN_INLINE
  const_pointer data() const noexcept {
    return _data;
  }


  constexpr SWC_CAN_INLINE
  iterator begin() noexcept {
    return _data;
  }

  constexpr SWC_CAN_INLINE
  const_iterator cbegin() const noexcept {
    return _data;
  }

  constexpr SWC_CAN_INLINE
  const_iterator begin() const noexcept {
    return cbegin();
  }


  constexpr SWC_CAN_INLINE
  iterator end() noexcept {
    return _data + _size;
  }

  constexpr SWC_CAN_INLINE
  const_iterator cend() const noexcept {
    return _data + _size;
  }

  constexpr SWC_CAN_INLINE
  const_iterator end() const noexcept {
    return cend();
  }


  constexpr SWC_CAN_INLINE
  reference front() noexcept {
    return *_data;
  }

  constexpr SWC_CAN_INLINE
  const_reference front() const noexcept {
    return *_data;
  }


  constexpr SWC_CAN_INLINE
  reference back() noexcept {
    return _data[_size - 1];
  }

  constexpr SWC_CAN_INLINE
  const_reference back() const noexcept {
    return _data[_size - 1];
  }


  constexpr SWC_CAN_INLINE
  reference operator[](size_type pos) noexcept {
    return _data[pos];
  }

  constexpr SWC_CAN_INLINE
  const_reference operator[](size_type pos) const noexcept {
    return _data[pos];
  }


  SWC_CAN_INLINE
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
  void reserve(size_type cap) {
    if(_cap < cap)
      _grow(cap - _cap);
  }

  SWC_CAN_INLINE
  void reserve() {
    if(_cap == _size) {
      if(_size) {
        size_type remain = max_size() - _cap;
        _grow(GROW_SZ ? (GROW_SZ < remain ? GROW_SZ : remain)
                      : (_size < remain ? _size : remain));
      } else {
        _grow(1);
      }
    }
  }

  template<typename... ArgsT>
  SWC_CAN_INLINE
  void resize(size_type sz, ArgsT&&... args) {
    if(sz > _size) {
      reserve(sz);
      for(pointer ptr = _data + _size; _size < sz; ++_size, ++ptr)
        _construct(ptr, std::forward<ArgsT>(args)...);
    } else if(sz < _size) {
      if(is_SimpleType) {
        _size = sz;
      } else {
        for(pointer ptr = _data + sz; sz < _size; --_size, ++ptr)
          ptr->~value_type();
      }
    }
  }


  template<typename... ArgsT>
  SWC_CAN_INLINE
  void push_back(ArgsT&&... args) {
    //if(max_size() == _size)
    //  throw std::out_of_range("Reached size_type limit!");
    reserve();
    push_back_unsafe(std::forward<ArgsT>(args)...);
  }
  template<typename... ArgsT>
  SWC_CAN_INLINE
  void push_back_unsafe(ArgsT&&... args) noexcept(is_SimpleType) {
    _construct(_data + _size, std::forward<ArgsT>(args)...);
    ++_size;
  }


  template<typename... ArgsT>
  SWC_CAN_INLINE
  reference emplace_back(ArgsT&&... args) {
    //if(max_size() == _size)
    //  throw std::out_of_range("Reached size_type limit!");
    reserve();
    return emplace_back_unsafe(std::forward<ArgsT>(args)...);
  }
  template<typename... ArgsT>
  SWC_CAN_INLINE
  reference emplace_back_unsafe(ArgsT&&... args) noexcept(is_SimpleType) {
    reference ref = *_construct(_data + _size, std::forward<ArgsT>(args)...);
    ++_size;
    return ref;
  }


  template<typename... ArgsT>
  SWC_CAN_INLINE
  iterator insert(size_type offset, ArgsT&&... args) {
    if(offset >= _size)
      return &emplace_back(std::forward<ArgsT>(args)...);

    if(_cap == _size) {
      size_type remain = max_size() - _cap;
      if(!remain)
        throw std::out_of_range("Reached size_type limit!");
      _cap += GROW_SZ ? (GROW_SZ < remain ? GROW_SZ : remain)
                      : (_size < remain ? _size : remain);
      _data = _allocate_insert(
        _data, _size, offset, _cap, std::forward<ArgsT>(args)...);
    } else {
      _construct(
        _alter(_data + offset, _size - offset, 1),
        std::forward<ArgsT>(args)...);
    }
    ++_size;
    return _data + offset;
  }

  template<typename... ArgsT>
  SWC_CAN_INLINE
  iterator insert(const_iterator it, ArgsT&&... args) {
    return insert(it - _data, std::forward<ArgsT>(args)...);
  }

  template<typename... ArgsT>
  SWC_CAN_INLINE
  iterator insert_unsafe(const_iterator it,
                         ArgsT&&... args) noexcept(is_SimpleType) {
    size_type offset = it - _data;
    return offset >= _size
      ? &emplace_back_unsafe(std::forward<ArgsT>(args)...)
      : _construct(
          _alter(_data + offset, (_size++) - offset, 1),
          std::forward<ArgsT>(args)...);
  }


  SWC_CAN_INLINE
  iterator insert(size_type offset,
                  const_iterator first, const_iterator last) {
    if(first == last)
      return _data + offset;

    size_type sz = last - first;
    size_type remain = _cap - _size;
    if(sz > remain) {
      if(max_size() - _size < sz)
        throw std::out_of_range("Reached size_type limit!");
      if(_size) {
        _cap += sz - remain;
        _data = _allocate_insert(_data, _size, offset, _cap, first, last);
        _size += sz;
        return _data + offset;
      }
      _grow(sz - remain);
    }
    pointer data_offset = _size
      ? (_size > offset
          ? _alter(_data + offset, _size - offset, sz)
          : _data + offset)
      : _data;
    _size += sz;
    pointer ptr = data_offset;
    for(size_type i=0; i<sz; ++ptr, ++i)
      _construct(ptr, first[i]);
    return data_offset;
  }

  SWC_CAN_INLINE
  iterator insert(const_iterator it,
                  const_iterator first, const_iterator last) {
    return insert(it - _data, first, last);
  }


  SWC_CAN_INLINE
  void assign(const_iterator first, const_iterator last) {
    clear();
    if(size_type sz = last - first) {
      reserve(sz);
      _size = sz;
      for(size_type i = 0; i < sz; ++i)
        _construct(_data + i, first[i]);
    }
  }


  SWC_CAN_INLINE
  iterator erase(size_type offset) noexcept(is_SimpleType) {
    if(offset >= _size)
      return end();

    --_size;
    pointer ptr = _data + offset;
    for(size_type remain= _size - offset; remain; --remain, ++ptr) {
      *ptr = std::move(*(ptr + 1));
    }
    if(!is_SimpleType)
      ptr->~value_type();
    return _data + offset;
  }

  SWC_CAN_INLINE
  iterator erase(const_iterator it) noexcept(is_SimpleType) {
    return erase(it - _data);
  }

  SWC_CAN_INLINE
  iterator erase(const_iterator first,
                 const_iterator last) noexcept(is_SimpleType) {
    size_type offset = first - _data;
    if(offset >= _size)
      return end();

    size_type amt = last - first;
    _size -= amt;
    pointer ptr = _data + offset;
    for(size_type remain = _size - offset; remain; --remain, ++ptr) {
      *ptr = std::move(*(ptr + amt));
    }
    if(!is_SimpleType) for(; ptr != last; ++ptr) {
      ptr->~value_type();
    }
    return _data + offset;
  }


  private:

  SWC_CAN_INLINE
  void _grow(size_type sz) {
    _data = _allocate_uinitialized(_data, _size, _cap += sz);
  }

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

  template<typename... ArgsT>
  SWC_CAN_INLINE
  static pointer _allocate_insert(pointer data_prev, size_type size_prev,
                                  size_type offset, size_type size,
                                  ArgsT&&... args) {
    if(offset == size)
      throw std::out_of_range("Offset above size_type limit!");
    pointer data = _allocate_uinitialized(size);
    pointer ptr = data;
    pointer ptr_prev = data_prev;
    size_type i = 0;
    for(; i < offset; ++ptr, ++ptr_prev, ++i) {
      _construct(ptr, std::move(*ptr_prev));
      if(!is_SimpleType)
        ptr_prev->~value_type();
    }
    _construct(ptr++, std::forward<ArgsT>(args)...);

    for(; i < size_prev; ++i, ++ptr, ++ptr_prev) {
      _construct(ptr, std::move(*ptr_prev));
      if(!is_SimpleType)
        ptr_prev->~value_type();
    }
    _deallocate(data_prev, size_prev);
    return data;
  }

  SWC_CAN_INLINE
  static pointer _allocate_insert(pointer data_prev, size_type size_prev,
                                  size_type offset, size_type size,
                                  const_iterator first, const_iterator last) {
    //if(offset >= size)
    //  throw std::out_of_range("Reached size_type limit!");
    pointer data = _allocate_uinitialized(size);
    pointer ptr = data;
    pointer ptr_prev = data_prev;
    size_type i = 0;
    for(; i < offset; ++ptr, ++ptr_prev, ++i) {
      _construct(ptr, std::move(*ptr_prev));
      if(!is_SimpleType)
        ptr_prev->~value_type();
    }
    for(iterator it=const_cast<iterator>(first); it != last; ++ptr, ++it) {
      _construct(ptr, *it);
    }
    for(; i < size_prev; ++i, ++ptr, ++ptr_prev) {
      _construct(ptr, std::move(*ptr_prev));
      if(!is_SimpleType)
        ptr_prev->~value_type();
    }
    _deallocate(data_prev, size_prev);
    return data;
  }

  SWC_CAN_INLINE
  static void _deallocate(pointer data, size_t) {
    ::operator delete(data);
    //std::free(data);
    //std::allocator<value_type> alloc;
    //alloc.deallocate(data, sz);
  }


  template<typename... ArgsT>
  SWC_CAN_INLINE
  static pointer _allocate(size_type sz, ArgsT&&... args) {
    pointer data = _allocate_uinitialized(sz);
    if(!is_SimpleType || sizeof...(args) > 0) {
      for(pointer ptr = data; sz; --sz, ++ptr)
        _construct(ptr, std::forward<ArgsT>(args)...);
    }
    //std::uninitialized_default_construct(data, data + sz);
    return data;
  }


  template<typename... ArgsT>
  SWC_CAN_INLINE
  static pointer _construct(pointer ptr,
                            ArgsT&&... args) noexcept(is_SimpleType) {
    new (ptr) value_type(std::forward<ArgsT>(args)...);
    return ptr;
  }


  SWC_CAN_INLINE
  static pointer _copy(pointer data,
                       const_pointer data_prev,
                       size_type size_prev) noexcept(is_SimpleType) {
    for(pointer ptr=data; size_prev; --size_prev, ++ptr, ++data_prev)
      _construct(ptr, *data_prev);
    //std::uninitialized_copy_n(data_prev, size_prev, data);
    return data;
  }

  SWC_CAN_INLINE
  static pointer _move(pointer data,
                       pointer data_prev,
                       size_type size_prev) noexcept(is_SimpleType) {
    for(pointer ptr=data; size_prev; --size_prev, ++ptr, ++data_prev) {
      _construct(ptr, std::move(*data_prev));
      if(!is_SimpleType)
        data_prev->~value_type();
    }
    //std::uninitialized_move_n(data_prev, size_prev, data);
    return data;
  }

  SWC_CAN_INLINE
  static pointer _alter(pointer data, size_type remain,
                        size_type amount) noexcept(is_SimpleType) {
    pointer prev = data + remain - 1;
    for(pointer ptr = prev + amount; remain; --remain, --ptr, --prev) {
      _construct(ptr, std::move(*prev));
      if(!is_SimpleType)
        prev->~value_type();
    }
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
