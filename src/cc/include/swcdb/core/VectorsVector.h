/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_VectorsVector_h
#define swcdb_core_VectorsVector_h



namespace SWC { namespace Core {



template<typename VectorsT, typename VectorT,
         typename VectorT::size_type SIZE,
         typename VectorT::size_type GROW = SIZE/4,
         typename VectorT::size_type SPLIT = SIZE/2>
class VectorsVector : public VectorsT {

  public:

  using value_type = typename VectorT::value_type;

  constexpr SWC_CAN_INLINE
  VectorsVector() noexcept { }

  constexpr SWC_CAN_INLINE
  static size_t need_reserve(VectorT& vec) {
    if(GROW > 1 && vec.capacity() == vec.size() && vec.capacity() >= GROW) {
      size_t sz = vec.size() + GROW;
      if((sz > SIZE ? (sz = SIZE) : sz) > vec.capacity())
        return sz;
    }
    return 0;
  }



  /* Position by vector index
  class ConstIterator final {
    const VectorsT&               _vectors;
    typename VectorsT::size_type  _pos_vec;
    typename VectorT::size_type   _pos_item;
    public:

    constexpr SWC_CAN_INLINE
    ConstIterator(const VectorsT& a_vectors) noexcept
                  : _vectors(a_vectors), _pos_vec(0), _pos_item(0) {
    }

    constexpr SWC_CAN_INLINE
    ConstIterator(const VectorsT& a_vectors, size_t offset) noexcept
                  : _vectors(a_vectors), _pos_vec(0) {
      for(; _pos_vec < _vectors.size(); ++_pos_vec) {
        if(!offset || offset < _vectors[_pos_vec].size()) {
          _pos_item = offset;
          break;
        }
        offset -= _vectors[_pos_vec].size();
      }
    }

    constexpr SWC_CAN_INLINE
    ConstIterator(const ConstIterator& other) noexcept
                  : _vectors(other._vectors),
                    _pos_vec(other._pos_vec), _pos_item(other._pos_item) {
    }

    constexpr SWC_CAN_INLINE
    ConstIterator& operator=(const ConstIterator& other) noexcept {
      _pos_vec = other._pos_vec;
      _pos_item = other._pos_item;
      return *this;
    }

    constexpr SWC_CAN_INLINE
    ConstIterator& at(size_t offset) noexcept {
      for(_pos_vec = 0; _pos_vec < _vectors.size(); ++_pos_vec) {
        if(!offset || offset < _vectors[_pos_vec].size()) {
          _pos_item = offset;
          break;
        }
        offset -= _vectors[_pos_vec].size();
      }
      return *this;
    }

    constexpr SWC_CAN_INLINE
    operator bool() const noexcept {
      return _pos_vec != _vectors.size() &&
             _pos_item != _vectors[_pos_vec].size();
    }

    constexpr SWC_CAN_INLINE
    void operator++() noexcept {
      if(++_pos_item == _vectors[_pos_vec].size() &&
         ++_pos_vec != _vectors.size())
        _pos_item = 0;
    }

    constexpr SWC_CAN_INLINE
    const value_type& item() const noexcept {
      return _vectors[_pos_vec][_pos_item];
    }

  };


  class Iterator final {
    VectorsT&                     _vectors;
    typename VectorsT::size_type  _pos_vec;
    typename VectorT::size_type   _pos_item;
    public:

    constexpr SWC_CAN_INLINE
    Iterator(VectorsT& a_vectors) noexcept
            : _vectors(a_vectors), _pos_vec(0), _pos_item(0) {
    }

    constexpr SWC_CAN_INLINE
    Iterator(VectorsT& a_vectors, size_t offset) noexcept
            : _vectors(a_vectors), _pos_vec(0) {
      for(; _pos_vec < _vectors.size(); ++_pos_vec) {
        if(!offset || offset < _vectors[_pos_vec].size()) {
          _pos_item = offset;
          break;
        }
        offset -= _vectors[_pos_vec].size();
      }
    }

    constexpr SWC_CAN_INLINE
    Iterator(const Iterator& other) noexcept
            : _vectors(other._vectors),
              _pos_vec(other._pos_vec), _pos_item(other._pos_item) {
    }

    constexpr SWC_CAN_INLINE
    Iterator& operator=(const Iterator& other) noexcept {
      _pos_vec = other._pos_vec;
      _pos_item = other._pos_item;
      return *this;
    }

    constexpr SWC_CAN_INLINE
    Iterator& at(size_t offset) noexcept {
      for(_pos_vec = 0; _pos_vec < _vectors.size(); ++_pos_vec) {
        if(!offset || offset < _vectors[_pos_vec].size()) {
          _pos_item = offset;
          break;
        }
        offset -= _vectors[_pos_vec].size();
      }
      return *this;
    }

    constexpr SWC_CAN_INLINE
    operator bool() const noexcept {
      return _pos_vec != _vectors.size() &&
             _pos_item != _vectors[_pos_vec].size();
    }

    constexpr SWC_CAN_INLINE
    void operator++() noexcept {
      if(++_pos_item == _vectors[_pos_vec].size() &&
         ++_pos_vec != _vectors.size())
        _pos_item = 0;
    }

    constexpr SWC_CAN_INLINE
    value_type& item() noexcept {
      return _vectors[_pos_vec][_pos_item];
    }

    SWC_CAN_INLINE
    void insert(const value_type& value) {
       ensure();
      _vectors[_pos_vec].insert(
        _vectors[_pos_vec].cbegin() + _pos_item, value);
      //_vectors[_pos_vec].insert(_pos_item, value);
    }

    SWC_CAN_INLINE
    void insert(value_type&& value) {
       ensure();
      _vectors[_pos_vec].insert(
        _vectors[_pos_vec].cbegin() + _pos_item, std::move(value));
      //_vectors[_pos_vec].insert(_pos_item, std::move(value));
    }

    SWC_CAN_INLINE
    void ensure() {
      if(_vectors[_pos_vec].size() >= SIZE) {
        auto n_vector = _vectors.insert(
          _vectors.cbegin() + _pos_vec + 1, VectorT());
        //auto n_vector = _vectors.insert(_pos_vec + 1, VectorT());
        n_vector->reserve(SPLIT + 1);
        auto it_b = _vectors[_pos_vec].cbegin() + SPLIT;
        n_vector->assign         (it_b, _vectors[_pos_vec].cend());
        _vectors[_pos_vec].erase (it_b, _vectors[_pos_vec].cend());
        if(_pos_item >= SPLIT) {
          ++_pos_vec;
          _pos_item -= SPLIT;
        }

      } else if(size_t sz = need_reserve(_vectors[_pos_vec])) {
        _vectors[_pos_vec].reserve(sz);
      }
    }

    SWC_CAN_INLINE
    void remove() noexcept {
      _vectors[_pos_vec].erase(_vectors[_pos_vec].cbegin() + _pos_item);
      //_vectors[_pos_vec].erase(_pos_item);
      if(_vectors[_pos_vec].empty() && _vectors.size() > 1) {
        _vectors.erase(_vectors.cbegin() + _pos_vec);
        //_vectors.erase(_pos_vec);
        if(_pos_vec != _vectors.size())
          _pos_item = 0;
      } else if(_pos_item == _vectors[_pos_vec].size() &&
                ++_pos_vec != _vectors.size()) {
        _pos_item = 0;
      }
    }

    SWC_CAN_INLINE
    void remove(size_t number) noexcept {
      while(number) {
        if(_pos_item == 0 && number >= _vectors[_pos_vec].size()) {
          if(_vectors.size() == 1) {
            _vectors[_pos_vec].clear();
            _pos_item = 0;
            return;
          }
          number -= _vectors[_pos_vec].size();
          _vectors.erase(_vectors.cbegin() + _pos_vec);
          //_vectors.erase(_pos_vec);

        } else {
          size_t avail = _vectors[_pos_vec].size() - _pos_item;
          if(avail > number) {
            _vectors[_pos_vec].erase(
              _vectors[_pos_vec].cbegin() + _pos_item,
              _vectors[_pos_vec].cbegin() + _pos_item + number);
            return;
          }
          number -= avail;
          _vectors[_pos_vec].erase(
              _vectors[_pos_vec].cbegin() + _pos_item,
              _vectors[_pos_vec].cend());
          ++_pos_vec;
        }

        if(_pos_vec == _vectors.size())
          return;
        _pos_item = 0;
      }
    }

  };
  */



  /* Position by vector iterator */
  class ConstIterator final {
    const VectorsT&                   _vectors;
    typename VectorsT::const_iterator _vector;
    typename VectorT::const_iterator  _item;
    public:

    constexpr SWC_CAN_INLINE
    ConstIterator(const VectorsT& a_vectors) noexcept
        : _vectors(a_vectors), _vector(_vectors.cbegin()),
          _item(_vector == _vectors.cend()
                  ? typename VectorT::const_iterator() : _vector->cbegin()) {
    }

    constexpr SWC_CAN_INLINE
    ConstIterator(const VectorsT& a_vectors, size_t offset) noexcept
        : _vectors(a_vectors), _vector(_vectors.cbegin()) {
      for(auto vec_end = _vectors.cend(); _vector != vec_end; ++_vector) {
        if(!offset || offset < _vector->size()) {
          _item = _vector->cbegin() + offset;
          break;
        }
        offset -= _vector->size();
      }
    }

    constexpr SWC_CAN_INLINE
    ConstIterator(const ConstIterator& other) noexcept
        : _vectors(other._vectors), _vector(other._vector),
          _item(other._item) {
    }

    constexpr SWC_CAN_INLINE
    ConstIterator& operator=(const ConstIterator& other) noexcept {
      _vector = other._vector;
      _item = other._item;
      return *this;
    }

    constexpr SWC_CAN_INLINE
    ConstIterator& at(size_t offset) noexcept {
      _vector = _vectors.cbegin();
      for(auto vec_end = _vectors.cend(); _vector != vec_end; ++_vector) {
        if(!offset || offset < _vector->size()) {
          _item = _vector->cbegin() + offset;
          break;
        }
        offset -= _vector->size();
      }
      return *this;
    }

    constexpr SWC_CAN_INLINE
    operator bool() const noexcept {
      return _vector != _vectors.cend() && _item != _vector->cend();
    }

    constexpr SWC_CAN_INLINE
    void operator++() noexcept {
      if(++_item == _vector->cend() && ++_vector != _vectors.cend())
        _item = _vector->cbegin();
    }

    constexpr SWC_CAN_INLINE
    const value_type& item() const noexcept {
      return *_item;
    }

  };


  class Iterator final {
    VectorsT&                   _vectors;
    typename VectorsT::iterator _vector;
    typename VectorT::iterator  _item;
    public:

    constexpr SWC_CAN_INLINE
    Iterator(VectorsT& a_vectors) noexcept
        : _vectors(a_vectors), _vector(_vectors.begin()),
          _item(_vector == _vectors.cend()
                  ? typename VectorT::iterator() : _vector->begin()) {
    }

    constexpr SWC_CAN_INLINE
    Iterator(VectorsT& a_vectors, size_t offset) noexcept
        : _vectors(a_vectors), _vector(_vectors.begin()) {
      for(auto vec_end = _vectors.cend(); _vector != vec_end; ++_vector) {
        if(!offset || offset < _vector->size()) {
          _item = _vector->begin() + offset;
          break;
        }
        offset -= _vector->size();
      }
    }

    constexpr SWC_CAN_INLINE
    Iterator(const Iterator& other) noexcept
        : _vectors(other._vectors), _vector(other._vector),
          _item(other._item) {
    }

    constexpr SWC_CAN_INLINE
    Iterator& operator=(const Iterator& other) noexcept {
      _vector = other._vector;
      _item = other._item;
      return *this;
    }

    constexpr SWC_CAN_INLINE
    Iterator& at(size_t offset) noexcept {
      _vector = _vectors.begin();
      for(auto vec_end = _vectors.cend(); _vector != vec_end; ++_vector) {
        if(!offset || offset < _vector->size()) {
          _item = _vector->begin() + offset;
          break;
        }
        offset -= _vector->size();
      }
      return *this;
    }

    constexpr SWC_CAN_INLINE
    operator bool() const noexcept {
      return _vector != _vectors.cend() && _item != _vector->cend();
    }

    constexpr SWC_CAN_INLINE
    void operator++() noexcept {
      if(++_item == _vector->cend() && ++_vector != _vectors.cend())
        _item = _vector->begin();
    }

    constexpr SWC_CAN_INLINE
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
  /**/



  template<typename T>
  constexpr SWC_CAN_INLINE
  T get() noexcept {
    return T(*this);
  }

  template<typename T>
  constexpr SWC_CAN_INLINE
  T get() const noexcept {
    return T(*this);
  }

  template<typename T>
  constexpr SWC_CAN_INLINE
  T get(size_t offset) noexcept {
    return T(*this, offset);
  }

  template<typename T>
  constexpr SWC_CAN_INLINE
  T get(size_t offset) const noexcept {
    return T(*this, offset);
  }


  constexpr SWC_CAN_INLINE
  Iterator GetIterator() noexcept {
    return get<Iterator>();
  }

  constexpr SWC_CAN_INLINE
  Iterator GetIterator(size_t offset) noexcept {
    return get<Iterator>(offset);
  }

  constexpr SWC_CAN_INLINE
  ConstIterator GetConstIterator() const noexcept {
    return get<ConstIterator>();
  }

  constexpr SWC_CAN_INLINE
  ConstIterator GetConstIterator(size_t offset) const noexcept {
    return get<ConstIterator>(offset);
  }


  constexpr SWC_CAN_INLINE
  size_t size_of_internal() const noexcept {
    size_t sz = VectorsT::capacity() * sizeof(VectorT);
    for(auto& vec : *this)
      sz += vec.capacity() * sizeof(value_type);
    return sz;
  }

  constexpr SWC_CAN_INLINE
  size_t count() const noexcept {
    size_t sz = 0;
    for(auto& vec : *this)
      sz += vec.size();
    return sz;
  }


  constexpr SWC_CAN_INLINE
  value_type& front() noexcept {
    return VectorsT::front().front();
  }

  constexpr SWC_CAN_INLINE
  value_type& back() noexcept {
    return VectorsT::back().back();
  }

  constexpr SWC_CAN_INLINE
  const value_type& front() const noexcept {
    return VectorsT::front().front();
  }

  constexpr SWC_CAN_INLINE
  const value_type& back() const noexcept {
    return VectorsT::back().back();
  }

  constexpr SWC_CAN_INLINE
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
    VectorsT::reserve(VectorsT::size() + other.size());
    for(auto it = other.begin(); it != other.cend(); ++it)
      VectorsT::emplace_back(std::move(*it));
    other.clear();
    other.shrink_to_fit();
  }


  SWC_CAN_INLINE
  void split(size_t split_at, VectorsVector& to) {
    to.reserve(to.size() + split_at);
    for(auto it = VectorsT::begin() + split_at; it != VectorsT::cend(); ++it)
      to.emplace_back(std::move(*it));
    VectorsT::erase(VectorsT::cbegin() + split_at, VectorsT::cend());
  }

  void print(std::ostream& out) const {
    out << "VectorsVector(" << VectorsT::size() << '/' << VectorsT::capacity()
        << " vectors=[";
    for(auto& vec : *this)
      out << vec.size() << '/' << vec.capacity() << ',';
    out << "])";
  }


};


}} // namespace SWC::Core



#endif // swcdb_core_VectorsVector_h
