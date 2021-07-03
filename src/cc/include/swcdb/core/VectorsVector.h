/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_VectorsVector_h
#define swcdb_core_VectorsVector_h

#include <vector>



namespace SWC { namespace Core {



template<typename VectorT, size_t SIZE, size_t GROW=SIZE/4, size_t SPLIT=SIZE/2>
class VectorsVector : public std::vector<VectorT> {

  using VectorsT = std::vector<VectorT>;

  public:

  using value_type = typename VectorT::value_type;


  SWC_CAN_INLINE
  static size_t need_reserve(VectorT& vec) {
    if(vec.capacity() == vec.size() && vec.capacity() >= GROW) {
      size_t sz = vec.size() + GROW;
      if((sz > SIZE ? (sz = SIZE) : sz) > vec.capacity())
        return sz;
    }
    return 0;
  }


  class ConstIterator final {
    const VectorsT&                   _vectors;
    typename VectorsT::const_iterator _vector;
    typename VectorT::const_iterator  _item;
    public:

    SWC_CAN_INLINE
    ConstIterator(const VectorsT& _vectors) noexcept
                  : _vectors(_vectors), _vector(_vectors.cbegin()),
                    _item(_vector == _vectors.cend()
                            ? typename VectorT::const_iterator() : _vector->cbegin()) {
    }

    SWC_CAN_INLINE
    ConstIterator(const VectorsT& _vectors, size_t offset) noexcept
                  : _vectors(_vectors), _vector(_vectors.cbegin()) {
      for(; _vector != _vectors.cend(); ++_vector) {
        if(!offset || offset < _vector->size()) {
          _item = _vector->cbegin() + offset;
          break;
        }
        offset -= _vector->size();
      }
    }

    SWC_CAN_INLINE
    ConstIterator(const ConstIterator& other) noexcept
                  : _vectors(other._vectors),
                    _vector(other._vector), _item(other._item) {
    }

    SWC_CAN_INLINE
    ConstIterator& operator=(const ConstIterator& other) noexcept {
      _vector = other._vector;
      _item = other._item;
      return *this;
    }

    SWC_CAN_INLINE
    ConstIterator& at(size_t offset) noexcept {
      for(_vector=_vectors.cbegin(); _vector != _vectors.cend(); ++_vector) {
        if(!offset || offset < _vector->size()) {
          _item = _vector->cbegin() + offset;
          return *this;
        }
        offset -= _vector->size();
      }
      return *this;
    }

    SWC_CAN_INLINE
    operator bool() const noexcept {
      return _vector != _vectors.cend() && _item != _vector->cend();
    }

    SWC_CAN_INLINE
    void operator++() noexcept {
      if(++_item == _vector->cend() && ++_vector != _vectors.cend())
        _item = _vector->cbegin();
    }

    SWC_CAN_INLINE
    const value_type& item() const noexcept {
      return *_item;
    }

  };


  class Iterator final {
    VectorsT&                   _vectors;
    typename VectorsT::iterator _vector;
    typename VectorT::iterator  _item;
    public:

    SWC_CAN_INLINE
    Iterator(VectorsT& _vectors) noexcept
            : _vectors(_vectors), _vector(_vectors.begin()),
              _item(_vector == _vectors.cend()
                      ? typename VectorT::iterator() : _vector->begin()) {
    }

    SWC_CAN_INLINE
    Iterator(VectorsT& _vectors, size_t offset) noexcept
            : _vectors(_vectors), _vector(_vectors.begin()) {
      for(; _vector != _vectors.cend(); ++_vector) {
        if(!offset || offset < _vector->size()) {
          _item = _vector->begin() + offset;
          break;
        }
        offset -= _vector->size();
      }
    }

    SWC_CAN_INLINE
    Iterator(const Iterator& other) noexcept
            : _vectors(other._vectors),
              _vector(other._vector), _item(other._item) {
    }

    SWC_CAN_INLINE
    Iterator& operator=(const Iterator& other) noexcept {
      _vector = other._vector;
      _item = other._item;
      return *this;
    }

    SWC_CAN_INLINE
    Iterator& at(size_t offset) noexcept {
      for(_vector=_vectors.begin(); _vector != _vectors.cend(); ++_vector) {
        if(!offset || offset < _vector->size()) {
          _item = _vector->begin() + offset;
          return *this;
        }
        offset -= _vector->size();
      }
      return *this;
    }

    SWC_CAN_INLINE
    operator bool() const noexcept {
      return _vector != _vectors.cend() && _item != _vector->cend();
    }

    SWC_CAN_INLINE
    void operator++() noexcept {
      if(++_item == _vector->cend() && ++_vector != _vectors.cend())
        _item = _vector->begin();
    }

    SWC_CAN_INLINE
    value_type& item() noexcept {
      return *_item;
    }

    SWC_CAN_INLINE
    void insert(const value_type& value) {
       ensure();
      _item = _vector->insert(_item, value);
    }

    SWC_CAN_INLINE
    void insert(value_type&& value) {
       ensure();
      _item = _vector->insert(_item, std::move(value));
    }

    SWC_CAN_INLINE
    void ensure() {
      if(_vector->size() >= SIZE) {
        auto it_b = _vector->cbegin() + SPLIT;
        auto n_vector = _vectors.insert(++_vector, VectorT());
        n_vector->reserve(SPLIT + 1);
        _vector = n_vector - 1;
        n_vector->assign(it_b, _vector->cend());
        _vector->erase  (it_b, _vector->cend());
        size_t offset = _item - _vector->cbegin();
        if(offset >= SPLIT)
          _item = (++_vector)->begin() + (offset - SPLIT);

      } else if(size_t sz = need_reserve(*_vector)) {
        size_t offset = _item - _vector->cbegin();
        _vector->reserve(sz);
        _item = _vector->begin() + offset;
      }
    }

    SWC_CAN_INLINE
    void remove() noexcept {
      _item = _vector->erase(_item);
      if(_vector->empty() && _vectors.size() > 1) {
        if((_vector = _vectors.erase(_vector)) != _vectors.cend())
          _item = _vector->begin();
      } else if(_item == _vector->cend() && ++_vector != _vectors.cend()) {
        _item = _vector->begin();
      }
    }

    SWC_CAN_INLINE
    void remove(size_t number) noexcept {
      while(number) {
        if(_item == _vector->begin() && number >= _vector->size()) {
          if(_vectors.size() == 1) {
            _vector->clear();
            _item = _vector->end();
            return;
          }
          number -= _vector->size();
          _vector = _vectors.erase(_vector);

        } else {
          size_t avail = _vector->cend() - _item;
          if(avail > number) {
            _item = _vector->erase(_item, _item + number);
            return;
          }
          number -= avail;
          _vector->erase(_item, _vector->cend());
          ++_vector;
        }

        if(_vector == _vectors.cend())
          return;
        _item = _vector->begin();
      }
    }

  };




  template<typename T>
  SWC_CAN_INLINE
  T get() noexcept {
    return T(*this);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get() const noexcept {
    return T(*this);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(size_t offset) noexcept {
    return T(*this, offset);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(size_t offset) const noexcept {
    return T(*this, offset);
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
  size_t size_of_internal() const noexcept {
    size_t sz = VectorsT::capacity() * sizeof(VectorT);
    for(auto& vector : *this)
      sz += vector.capacity() * sizeof(value_type);
    return sz;
  }

  SWC_CAN_INLINE
  size_t count() const noexcept {
    size_t sz = 0;
    for(auto& vector : *this)
      sz += vector.size();
    return sz;
  }


  SWC_CAN_INLINE
  value_type& front() noexcept {
    return VectorsT::front().front();
  }

  SWC_CAN_INLINE
  value_type& back() noexcept {
    return VectorsT::back().back();
  }

  SWC_CAN_INLINE
  const value_type& front() const noexcept {
    return VectorsT::front().front();
  }

  SWC_CAN_INLINE
  const value_type& back() const noexcept {
    return VectorsT::back().back();
  }

  SWC_CAN_INLINE
  value_type* operator[](size_t pos) noexcept {
    auto it = get<Iterator>(pos);
    return it ? &it.item() : nullptr;
  }


  SWC_CAN_INLINE
  void ensure() {
    if(VectorsT::empty() || VectorsT::back().size() >= SIZE) {
      VectorsT::emplace_back();
    } else if(size_t sz = need_reserve(VectorsT::back())) {
      VectorsT::back().reserve(sz);
    }
  }


  SWC_CAN_INLINE
  void push_back(const value_type& value) {
    ensure();
    VectorsT::back().push_back(value);
  }

  SWC_CAN_INLINE
  void push_back(value_type&& value) {
    ensure();
    VectorsT::back().push_back(std::move(value));
  }


  SWC_CAN_INLINE
  void add(VectorsVector&& other) {
    VectorsT::insert(VectorsT::end(), other.cbegin(), other.cend());
    other.clear();
    other.shrink_to_fit();
  }


  SWC_CAN_INLINE
  void split(size_t split_at, VectorsVector& to) {
    auto it = VectorsT::cbegin() + split_at;
    to.insert(to.end(), it, VectorsT::cend());
    VectorsT::erase(it, VectorsT::cend());
  }

  void print(std::ostream& out) const {
    out << "VectorsVector(" << VectorsT::size() << '/' << VectorsT::capacity()
        << " vectors=[";
    for(auto& vector : *this)
      out << vector.size() << '/' << vector.capacity() << ',';
    out << "])";
  }


};


}} // namespace SWC::Core



#endif // swcdb_core_VectorsVector_h
