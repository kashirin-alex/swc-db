/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_ArraysArraysVector_h
#define swcdb_core_ArraysArraysVector_h

#include <vector>

#include "swcdb/core/ArraysArray.h"


namespace SWC { namespace Core {



template<typename ArraysArrayT>
class ArraysArraysVector {

  using VectorT = std::vector<ArraysArrayT*>;
  VectorT _vector;

  public:

  using value_type = ArraysArrayT::value_type;


  class ConstIterator final {
    const VectorT&              _vector;
    VectorT::const_iterator     _array;
    ArraysArrayT::ConstIterator _item;

    public:

    SWC_CAN_INLINE
    ConstIterator(const VectorT& _vector) noexcept
                  : _vector(_vector), _array(_vector.cbegin()) {
      if(_array != _vector.cend())
        _item = (*_array)->GetConstIterator();
    }

    SWC_CAN_INLINE
    ConstIterator(const VectorT& _vector, size_t offset) noexcept
                  : _vector(_vector), _array(_vector.cbegin()) {
      for(; _array != _vector.cend(); ++_array) {
        if(offset && offset >= (*_array)->size())
          offset -= (*_array)->size();
        else if((_item = (*_array)->GetConstIterator(offset)))
          break;
      }
    }

    SWC_CAN_INLINE
    ConstIterator(const ConstIterator& other) noexcept
                  : _vector(other._vector), _array(other._array),
                    _item(other._item) {
    }

    SWC_CAN_INLINE
    ConstIterator& operator=(const ConstIterator& other) noexcept {
      _array = other._array;
      _item = other._item;
      return *this;
    }

    SWC_CAN_INLINE
    ConstIterator& at(size_t offset) noexcept {
      for(_array = _vector.cbegin(); _array != _vector.cend(); ++_array) {
        if(offset && offset >= (*_array)->size())
          offset -= (*_array)->size();
        else if((_item = (*_array)->GetConstIterator(offset)))
          break;
      }
      return *this;
    }

    SWC_CAN_INLINE
    operator bool() const noexcept {
      return _array != _vector.cend() && _item;
    }

    SWC_CAN_INLINE
    void operator++() noexcept {
      if(!++_item && ++_array != _vector.cend())
        _item = (*_array)->GetConstIterator();
    }

    SWC_CAN_INLINE
    const value_type& item() const noexcept {
      return _item.item();
    }

  };


  class Iterator final {
    VectorT&               _vector;
    VectorT::iterator      _array;
    ArraysArrayT::Iterator _item;

    public:
    SWC_CAN_INLINE
    Iterator(VectorT& _vector) noexcept
            : _vector(_vector), _array(_vector.begin()) {
      if(_array != _vector.cend())
        _item = (*_array)->GetIterator();
    }

    SWC_CAN_INLINE
    Iterator(VectorT& _vector, size_t offset) noexcept
            : _vector(_vector), _array(_vector.begin()) {
      for(; _array != _vector.cend(); ++_array) {
        if(offset && offset >= (*_array)->size())
          offset -= (*_array)->size();
        else if((_item = (*_array)->GetIterator(offset)))
          break;
      }
    }

    SWC_CAN_INLINE
    Iterator(const Iterator& other) noexcept
            : _vector(other._vector), _array(other._array),
              _item(other._item) {
    }

    SWC_CAN_INLINE
    Iterator& operator=(const Iterator& other) noexcept {
      _array = other._array;
      _item = other._item;
      return *this;
    }

    SWC_CAN_INLINE
    Iterator& at(size_t offset) noexcept {
      for(_array = _vector.begin(); _array != _vector.cend(); ++_array) {
        if(offset && offset >= (*_array)->size())
          offset -= (*_array)->size();
        else if((_item = (*_array)->GetIterator(offset)))
          break;
      }
      return *this;
    }

    SWC_CAN_INLINE
    operator bool() const noexcept {
      return _array != _vector.end() && _item;
    }

    SWC_CAN_INLINE
    void operator++() noexcept {
      if(!++_item && ++_array != _vector.cend())
        _item = (*_array)->GetIterator();
    }

    SWC_CAN_INLINE
    void insert(const value_type& value) {
      reserve();
      _item.insert(value);
    }

    SWC_CAN_INLINE
    void insert(value_type&& value) {
      reserve();
      _item.insert(std::move(value));
    }

    SWC_CAN_INLINE
    void remove() noexcept {
      _item.remove();
      bool no_item = (*_array)->empty();
      if(no_item) {
        delete *_array;
        _array = _vector.erase(_array);
      } else if(!_item) {
        ++_array;
        no_item = true;
      }
      if(no_item && _array != _vector.end())
        _item = (*_array)->GetIterator();
    }

    SWC_CAN_INLINE
    void remove(size_t number) noexcept {
      while(number) {
        _item.remove(number);
        if(_item) {
          break;
        } else if((*_array)->empty()) {
          delete *_array;
          _array = _vector.erase(_array);
        } else {
          ++_array;
        }
        if(_array == _vector.end())
          return;
        _item = (*_array)->GetIterator();
      }
    }

    SWC_CAN_INLINE
    value_type& item() const noexcept {
      return _item.item();
    }

    private:

    SWC_CAN_INLINE
    void reserve() {
      if((*_array)->full() && _item.array_full()) {
        _array = _vector.insert(++_array, new ArraysArrayT);
        (*_array)->add_array();
        _item = (*_array)->GetIterator();
      }
    }

  };


  SWC_CAN_INLINE
  ArraysArraysVector() noexcept { }

  SWC_CAN_INLINE
  ArraysArraysVector(ArraysArraysVector&& other) noexcept
                    : _vector(std::move(other._vector)) {
  }

  SWC_CAN_INLINE
  ArraysArraysVector& operator=(ArraysArraysVector&& other) noexcept {
    _vector = std::move(other._vector);
    return *this;
  }

  SWC_CAN_INLINE
  ~ArraysArraysVector() noexcept {
    for(auto array : _vector)
      delete array;
  }

  SWC_CAN_INLINE
  void clear() noexcept {
    for(auto array : _vector)
      delete array;
    _vector.clear();
    _vector.shrink_to_fit();
  }

  SWC_CAN_INLINE
  void shrink_to_fit() noexcept {
    _vector.shrink_to_fit();
  }

  template<typename T>
  SWC_CAN_INLINE
  T get() noexcept {
    return T(_vector);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get() const noexcept {
    return T(_vector);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(size_t offset) noexcept {
    return T(_vector, offset);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(size_t offset) const noexcept {
    return T(_vector, offset);
  }


  SWC_CAN_INLINE
  Iterator GetIterator() noexcept {
    return get<Iterator>();
  }

  SWC_CAN_INLINE
  Iterator GetIterator(size_t offset) noexcept {
    return get<Iterator>(offset);
  }

  SWC_CAN_INLINE
  ConstIterator GetConstIterator() const noexcept {
    return get<ConstIterator>();
  }

  SWC_CAN_INLINE
  ConstIterator GetConstIterator(size_t offset) const noexcept {
    return get<ConstIterator>(offset);
  }


  SWC_CAN_INLINE
  size_t size() const noexcept {
    return _vector.size();
  }

  SWC_CAN_INLINE
  size_t count() const noexcept {
    size_t sz = 0;
    for(auto array : _vector)
      sz += array->count();
    return sz;
  }

  SWC_CAN_INLINE
  size_t size_of_internal() const noexcept {
    size_t sz = sizeof(ArraysArrayT*) + sizeof(ArraysArrayT);
    sz *= _vector.size();
    for(auto array : _vector)
      sz += array->size_of_internal();
    return sz;
  }

  SWC_CAN_INLINE
  bool empty() const noexcept {
    return _vector.empty();
  }

  SWC_CAN_INLINE
  value_type& front() noexcept {
    return _vector.front()->front();
  }

  SWC_CAN_INLINE
  value_type& back() noexcept {
    return _vector.back()->back();
  }

  SWC_CAN_INLINE
  const value_type& front() const noexcept {
    return _vector.front()->front();
  }

  SWC_CAN_INLINE
  const value_type& back() const noexcept {
    return _vector.back()->back();
  }

  SWC_CAN_INLINE
  value_type* operator[](size_t pos) noexcept {
    auto it = get<Iterator>(pos);
    return it ? &it->item() : nullptr;
  }


  SWC_CAN_INLINE
  void add_array() {
    _vector.push_back(new ArraysArrayT);
  }

  SWC_CAN_INLINE
  void push_back(const value_type& value) {
    if(_vector.empty() || _vector.back()->full_back())
      add_array();
    _vector.back()->push_back(value);
  }

  SWC_CAN_INLINE
  void push_back(value_type&& value) {
    if(_vector.empty() || _vector.back()->full_back())
      add_array();
    _vector.back()->push_back(std::move(value));
  }


  SWC_CAN_INLINE
  void add(ArraysArraysVector&& other) {
    _vector.reserve(_vector.size() + other._vector.size());
    for(auto array : other._vector)
      _vector.push_back(array);
    other._vector.clear();
    other._vector.shrink_to_fit();
  }

  SWC_CAN_INLINE
  void split(size_t split_at, ArraysArraysVector& to) {
    auto it = _vector.begin() + split_at;
    to._vector.reserve(split_at + 1);
    for(; it != _vector.end(); ++it)
      to._vector.push_back(*it);
    _vector.erase(it, _vector.end());
  }


  void print(std::ostream& out) const {
    out << "ArraysArraysVector(size=" << size_t(_vector.size())
        << " array=[";
    for(auto array : _vector)
      array->print(out << '\n');
    out << "])";
  }

};


}} // namespace SWC::Core



#endif // swcdb_core_ArraysArraysVector_h
