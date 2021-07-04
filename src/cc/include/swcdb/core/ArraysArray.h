/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_ArraysArray_h
#define swcdb_core_ArraysArray_h


#include "swcdb/core/Array.h"


namespace SWC { namespace Core {


// ArraysArray< Core::Array<value_type, uint8_t, 83>, uint8_t, 100>


template<typename ArrayT, typename SizeT, SizeT SIZE, SizeT SPLIT = SIZE/2>
class ArraysArray  {

  using Arrays = Core::Array<ArrayT*, SizeT, SIZE>;
  Arrays _arrays;

  public:
  static_assert(SizeT(-1) > 0,
                "SWC-DB ArraysArray supports only unsigned size!");

  using value_type = ArrayT::value_type;
  using size_type = SizeT;


  class ConstIterator final {
    const Arrays*          _arrays;
    Arrays::const_iterator _array;
    ArrayT::const_iterator _item;

    public:

    SWC_CAN_INLINE
    ConstIterator() noexcept : _arrays(nullptr) { }

    SWC_CAN_INLINE
    ConstIterator(const Arrays* _arrays) noexcept
                  : _arrays(_arrays), _array(_arrays->cbegin()),
                    _item((*_array)->cbegin()) {
    }

    SWC_CAN_INLINE
    ConstIterator(const Arrays* _arrays, size_t& offset) noexcept
                  : _arrays(_arrays), _array(_arrays->cbegin()) {
      for(; _array != _arrays->cend() &&
            (offset -= (*_array)->at(offset, _item));
            ++_array);
    }

    SWC_CAN_INLINE
    ConstIterator(const ConstIterator& other) noexcept
                  : _arrays(other._arrays), _array(other._array),
                    _item(other._item) {
    }

    SWC_CAN_INLINE
    ConstIterator& operator=(const ConstIterator& other) noexcept {
      _arrays = other._arrays;
      _array = other._array;
      _item = other._item;
      return *this;
    }

    SWC_CAN_INLINE
    operator bool() const noexcept {
      return _array != _arrays->cend() && _item != (*_array)->cend();
    }

    SWC_CAN_INLINE
    bool operator++() noexcept {
      if(++_item == (*_array)->cend()) {
        if(++_array == _arrays->cend())
          return false;
        _item = (*_array)->cbegin();
      }
      return true;
    }

    SWC_CAN_INLINE
    const value_type& item() const noexcept {
      return *_item;
    }

  };


  class Iterator final {
    Arrays*           _arrays;
    Arrays::iterator  _array;
    ArrayT::iterator  _item;

    public:
    SWC_CAN_INLINE
    Iterator() noexcept : _arrays(nullptr) { }

    SWC_CAN_INLINE
    Iterator(Arrays* _arrays) noexcept
            : _arrays(_arrays), _array(_arrays->begin()),
              _item((*_array)->begin()) {
    }

    SWC_CAN_INLINE
    Iterator(Arrays* _arrays, size_t& offset) noexcept
            : _arrays(_arrays), _array(_arrays->begin()) {
      for(; _array != _arrays->cend() &&
            (offset -= (*_array)->at(offset, _item));
            ++_array);
    }

    SWC_CAN_INLINE
    Iterator(const Iterator& other) noexcept
            : _arrays(other._arrays), _array(other._array),
              _item(other._item) {
    }

    SWC_CAN_INLINE
    Iterator& operator=(const Iterator& other) noexcept {
      _arrays = other._arrays;
      _array = other._array;
      _item = other._item;
      return *this;
    }

    SWC_CAN_INLINE
    operator bool() const noexcept {
      return _array != _arrays->cend() && _item != (*_array)->cend();
    }

    SWC_CAN_INLINE
    bool operator++() noexcept {
      if(++_item == (*_array)->cend()) {
        if(++_array == _arrays->cend())
          return false;
        _item = (*_array)->begin();
      }
      return true;
    }

    SWC_CAN_INLINE
    void insert(const value_type& value) {
      reserve();
      (*_array)->insert(_item, value);
    }

    SWC_CAN_INLINE
    void insert(value_type&& value) {
      reserve();
      (*_array)->insert(_item, std::move(value));
    }

    SWC_CAN_INLINE
    void remove() noexcept {
      (*_array)->erase(_item);
      if((*_array)->empty()) {
        delete *_array;
        _arrays->erase(_array);
        if(_array != _arrays->cend())
          _item = (*_array)->begin();
      } else if(_item == (*_array)->cend() &&
                ++_array != _arrays->cend()) {
        _item = (*_array)->begin();
      }
    }

    SWC_CAN_INLINE
    void remove(size_t& number) noexcept {
      do {
        if(_item == (*_array)->begin() && number >= (*_array)->size()) {
          number -= (*_array)->size();
          delete *_array;
          _arrays->erase(_array);
        } else {
          size_type avail = (*_array)->cend() - _item;
          if(avail > number) {
            (*_array)->erase(_item, number);
            number = 0;
            return;
          }
          number -= avail;
          (*_array)->erase(_item, avail);
          ++_array;
        }
        if(_array == _arrays->cend())
          return;
        _item = (*_array)->begin();
      } while(number);
    }

    SWC_CAN_INLINE
    value_type& item() const noexcept {
      return *_item;
    }

    SWC_CAN_INLINE
    bool array_full() const noexcept {
      return (*_array)->full();
    }

    private:

    SWC_CAN_INLINE
    void reserve() {
      if(array_full()) {
        auto at = (*_array)->begin() + SPLIT;
        _arrays->insert(_array + 1, new ArrayT(std::move(**_array), at));
        if(_item >= at)
          _item = (*(++_array))->begin() + (_item - at);
      }
    }

  };


  ArraysArray(const ArraysArray& other)             = delete;
  ArraysArray(ArraysArray&& other)                  = delete;
  ArraysArray& operator=(ArraysArray&& other)       = delete;
  ArraysArray& operator=(const ArraysArray& other)  = delete;


  SWC_CAN_INLINE
  ArraysArray() noexcept { }

  SWC_CAN_INLINE
  ~ArraysArray() noexcept {
    clear();
  }

  SWC_CAN_INLINE
  void clear() noexcept {
    for(auto array : _arrays)
      delete array;
  }


  template<typename T>
  SWC_CAN_INLINE
  T get() noexcept {
    return T(&_arrays);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get() const noexcept {
    return T(&_arrays);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(size_t offset) noexcept {
    return T(&_arrays, offset);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(size_t offset) const noexcept {
    return T(&_arrays, offset);
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
  size_type size() const noexcept {
    return _arrays.size();
  }

  SWC_CAN_INLINE
  size_t count() const noexcept {
    size_t sz = 0;
    for(auto array : _arrays)
      sz += array->size();
    return sz;
  }

  SWC_CAN_INLINE
  size_t size_of_internal() const noexcept {
    return _arrays.size() * sizeof(ArrayT);
  }

  SWC_CAN_INLINE
  bool full() const noexcept {
    return _arrays.full();
  }

  SWC_CAN_INLINE
  bool full_back() const noexcept {
    return _arrays.full() && _arrays.back()->full();
  }

  SWC_CAN_INLINE
  bool empty() const noexcept {
    return _arrays.empty();
  }

  SWC_CAN_INLINE
  value_type& front() noexcept {
    return _arrays.front()->front();
  }

  SWC_CAN_INLINE
  value_type& back() noexcept {
    return _arrays.back()->back();
  }

  SWC_CAN_INLINE
  const value_type& front() const noexcept {
    return _arrays.front()->front();
  }

  SWC_CAN_INLINE
  const value_type& back() const noexcept {
    return _arrays.back()->back();
  }

  SWC_CAN_INLINE
  value_type* operator[](size_t pos) noexcept {
    auto it = get<Iterator>(pos);
    return it ? &it->item() : nullptr;
  }

  SWC_CAN_INLINE
  void add_array() {
    _arrays.push_back(new ArrayT);
  }

  SWC_CAN_INLINE
  void push_back(const value_type& value) {
    if(_arrays.empty() || _arrays.back()->full())
      add_array();
    _arrays.back()->push_back(value);
  }

  SWC_CAN_INLINE
  void push_back(value_type&& value) {
    if(_arrays.empty() || _arrays.back()->full())
      add_array();
    _arrays.back()->push_back(std::move(value));
  }


  void print(std::ostream& out) const {
    out << "ArraysArray(size=" << size_t(_arrays.size())
        << " arrays=[";
    for(auto array : _arrays)
      out << size_t(array->size()) << ',';
    out << "])";
  }

};


}} // namespace SWC::Core



#endif // swcdb_core_ArraysArray_h
