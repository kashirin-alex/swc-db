/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Cells_Mutable_h
#define swcdb_db_Cells_Mutable_h


#include "swcdb/db/Cells/ReqScan.h"
#include "swcdb/core/VectorsVector.h"


namespace SWC { namespace DB { namespace Cells {


class Mutable final {

  typedef Core::VectorsVector<std::vector<Cell*>, 6144> Container;

  /*
  typedef Core::VectorsVector<Core::Vector<Cell*, uint32_t>, 6144> Container;
  */

  /*
  #include "swcdb/core/ArraysArraysVector.h"
  typedef Core::ArraysArraysVector<
    Core::ArraysArray < Core::Array<Cell*, uint8_t, 127>, uint8_t, 127 >
  > Container;
  */
  /*
  > Vector: (no ptr Cell)
   Array<Cell, uint8_t, 83>           (83 x 48 bytes + 8 bytes)  = 3992 bytes
   ArraysArray<Array*, uint8_t, 100> (100 x 8 bytes + 8 bytes)   = 808 bytes
   ArraysArraysVector<ArraysArray*>  (N x   8 bytes + 16 bytes)   = 16 + 8xN
  > Vector: (Pointer Cell)
   Array<Cell*, uint8_t, 127>         (127  x 8 bytes + 8 bytes) = 1024 bytes
   ArraysArray<Array*, uint8_t, 100> (127  x 8 bytes + 8 bytes)  = 1024 bytes
   ArraysArraysVector<ArraysArray*>  (N    x 8 bytes + 16 bytes)  = 16 + 8xN
  */

  Container _container;
  size_t    _bytes;
  size_t    _size;
  constexpr static const uint8_t  NARROW_SIZE = 20;


  public:

  typedef std::shared_ptr<Mutable> Ptr;

  using Iterator      = Container::Iterator;
  using ConstIterator = Container::ConstIterator;

  const Types::KeySeq key_seq;
  Types::Column       type;
  uint32_t            max_revs;
  uint64_t            ttl;

  SWC_CAN_INLINE
  static Ptr make(const Types::KeySeq key_seq,
                  const uint32_t max_revs=1,
                  const uint64_t ttl_ns=0,
                  const Types::Column type=Types::Column::PLAIN) {
    return Ptr(new Mutable(key_seq, max_revs, ttl_ns, type));
  }


  SWC_CAN_INLINE
  Mutable(const Types::KeySeq key_seq,
          const uint32_t max_revs=1, const uint64_t ttl_ns=0,
          const Types::Column type=Types::Column::PLAIN)
          : _bytes(0), _size(0),
            key_seq(key_seq), type(type), max_revs(max_revs), ttl(ttl_ns) {
  }

  SWC_CAN_INLINE
  Mutable(const Types::KeySeq key_seq,
          const uint32_t max_revs, const uint64_t ttl_ns,
          const Types::Column type,
          const StaticBuffer& buffer)
          : _bytes(0), _size(0),
            key_seq(key_seq), type(type), max_revs(max_revs), ttl(ttl_ns) {
    add_sorted(buffer.base, buffer.size);
  }

  SWC_CAN_INLINE
  Mutable(Mutable&& other) noexcept
          : _container(std::move(other._container)),
            _bytes(other._bytes), _size(other._size),
            key_seq(other.key_seq),
            type(other.type), max_revs(other.max_revs), ttl(other.ttl) {
    other._bytes = 0;
    other._size = 0;
  }

  Mutable(const Mutable&)                   = delete;
  Mutable& operator=(const Mutable& other)  = delete;

  SWC_CAN_INLINE
  Mutable& operator=(Mutable&& other) noexcept {
    _container = std::move(other._container);
    _bytes = other._bytes;
    _size = other._size;
    other._bytes = 0;
    other._size = 0;
    type = other.type;
    max_revs = other.max_revs;
    ttl = other.ttl;
    return *this;
  }

  SWC_CAN_INLINE
  void take_sorted(Mutable& other) {
    if(!other.empty()) {
      if(empty()) {
        _container = std::move(other._container);
      } else {
        _container.add(std::move(other._container));
      }
      _bytes += other._bytes;
      _size += other._size;
      other._bytes = 0;
      other._size = 0;
    }
  }

  constexpr SWC_CAN_INLINE
  void configure(const uint32_t revs=1, const uint64_t ttl_ns=0,
                 const Types::Column typ=Types::Column::PLAIN) noexcept {
    type = typ;
    max_revs = revs;
    ttl = ttl_ns;
  }

  SWC_CAN_INLINE
  ~Mutable() {
    for(auto it = get<ConstIterator>(); it; ++it)
      delete it.item();
  }

  SWC_CAN_INLINE
  void free() {
    for(auto it = get<ConstIterator>(); it; ++it)
      delete it.item();
    _container.clear();
    _container.shrink_to_fit();
    _bytes = 0;
    _size = 0;
  }

  SWC_CAN_INLINE
  void reset(const uint32_t revs=1, const uint64_t ttl_ns=0,
             const Types::Column typ=Types::Column::PLAIN) {
    free();
    configure(revs, ttl_ns, typ);
  }


  constexpr SWC_CAN_INLINE
  size_t size() const noexcept {
    return _size;
  }

  constexpr SWC_CAN_INLINE
  size_t size_bytes() const noexcept {
    return _bytes;
  }

  constexpr SWC_CAN_INLINE
  bool empty() const noexcept {
    return !_size;
  }

  constexpr SWC_CAN_INLINE
  size_t size_of_internal() const noexcept {
    return  _bytes
          + _size * sizeof(Cell)
          + _container.size_of_internal();
  }


  constexpr SWC_CAN_INLINE
  Cell& front() noexcept {
    return *_container.front();
  }

  constexpr SWC_CAN_INLINE
  Cell& back() noexcept {
    return *_container.back();
  }

  constexpr SWC_CAN_INLINE
  Cell& front() const noexcept {
    return *_container.front();
  }

  constexpr SWC_CAN_INLINE
  Cell& back() const noexcept {
    return *_container.back();
  }

  constexpr SWC_CAN_INLINE
  bool has_one_key() const noexcept {
    return front().key.equal(back().key);
  }


  template<typename T>
  constexpr SWC_CAN_INLINE
  T get() noexcept {
    return _container.get<T>();
  }

  template<typename T>
  constexpr SWC_CAN_INLINE
  T get() const noexcept {
    return _container.get<T>();
  }

  template<typename T>
  constexpr SWC_CAN_INLINE
  T get(size_t offset) noexcept {
    return _container.get<T>(offset);
  }

  template<typename T>
  constexpr SWC_CAN_INLINE
  T get(size_t offset) const noexcept {
    return _container.get<T>(offset);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(const Specs::Interval& specs) const {
    return specs.offset_key.empty()
      ? (specs.range_begin.empty() ? get<T>() : get<T>(specs.range_begin))
      : get<T>(specs.offset_key);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(const DB::Cell::Key& key) {
    size_t offset = 0;
    return get<T>(key, offset);
  }

  template<typename T>
  //SWC_CAN_INLINE
  T get(const DB::Cell::Key& key, size_t& offset) {
    T it = get<T>();
    if(key.empty() || _size <= NARROW_SIZE) {
      offset = 0;
      return it;
    }
    size_t step = _size;
    if(1 < offset && offset < _size) {
      if((step -= offset) <= NARROW_SIZE)
        step = NARROW_SIZE;
    } else {
      offset = step >>= 1;
    }
    try_get:
      if(!(it.at(offset)))
        return it.at(offset = 0);
      if(DB::KeySeq::compare(key_seq, it.item()->key, key) == Condition::GT) {
        if(step < NARROW_SIZE + max_revs)
          return it;
        if(offset + (step >>= 1) >= _size)
          offset = _size - (step >>= 1);
        else
          offset += step;
        goto try_get;
      }
      if((step >>= 1) <= NARROW_SIZE)
        step += max_revs;
      if(offset < step)
        return it.at(offset = 0);
      offset -= step;
    goto try_get;
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(const DB::Cell::Key& key) const {
    size_t offset = 0;
    return get<T>(key, offset);
  }

  template<typename T>
  //SWC_CAN_INLINE
  T get(const DB::Cell::Key& key, size_t& offset) const {
    T it = get<T>();
    if(key.empty() || _size <= NARROW_SIZE) {
      offset = 0;
      return it;
    }
    size_t step = _size;
    if(1 < offset && offset < _size) {
      if((step -= offset) <= NARROW_SIZE)
        step = NARROW_SIZE;
    } else {
      offset = step >>= 1;
    }
    try_get:
      if(!(it.at(offset)))
        return it.at(offset = 0);
      if(DB::KeySeq::compare(key_seq, it.item()->key, key) == Condition::GT) {
        if(step < NARROW_SIZE + max_revs)
          return it;
        if(offset + (step >>= 1) >= _size)
          offset = _size - (step >>= 1);
        else
          offset += step;
        goto try_get;
      }
      if((step >>= 1) <= NARROW_SIZE)
        step += max_revs;
      if(offset < step)
        return it.at(offset = 0);
      offset -= step;
    goto try_get;
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
  Cell* operator[](size_t pos) noexcept {
    auto it = get<Iterator>(pos);
    return it ? it.item() : nullptr;
  }

  constexpr SWC_CAN_INLINE
  void get(int32_t pos, DB::Cell::Key& key) const {
    if((pos < 0 && _size < size_t(-pos)) || size_t(pos) >= _size)
      return;
    auto it = get<ConstIterator>(pos < 0 ? _size + pos : pos);
    if(it)
      key.copy(it.item()->key);
  }

  SWC_CAN_INLINE
  bool get(const DB::Cell::Key& key, Condition::Comp comp,
           DB::Cell::Key& res) const {
    Condition::Comp chk;
    for(auto it = get<ConstIterator>(key); it; ++it) {
      if((chk = DB::KeySeq::compare(key_seq, key, it.item()->key))
                  == Condition::GT
        || (comp == Condition::GE && chk == Condition::EQ)){
        res.copy(it.item()->key);
        return true;
      }
    }
    return false;
  }


  constexpr SWC_CAN_INLINE
  void takeout(size_t pos, Cell*& cell) {
    if(Iterator it = get<Iterator>(pos)) {
      _remove(*it.item());
      cell = std::move(it.item());
      it.remove();
    }
  }

  constexpr SWC_CAN_INLINE
  void takeout_begin(size_t pos, Cell*& cell) {
    takeout(pos, cell);
  }

  constexpr SWC_CAN_INLINE
  void takeout_end(size_t pos, Cell*& cell) {
    takeout(_size - pos, cell);
  }


  constexpr SWC_CAN_INLINE
  void add_sorted(const Cell& cell, bool no_value=false) {
    add_sorted(new Cell(cell, no_value));
  }

  constexpr SWC_CAN_INLINE
  void add_sorted(Cell* cell) {
    _add(*cell);
    _container.push_back(cell);
  }

  constexpr SWC_CAN_INLINE
  size_t add_sorted(const uint8_t* ptr, size_t remain) {
    size_t count = 0;
    _bytes += remain;
    while(remain) {
      _container.push_back(new Cell(&ptr, &remain, true));
      ++count;
    }
    _size += count;
    return count;
  }

  SWC_CAN_INLINE
  void add_raw(const DynamicBuffer& cells) {
    size_t offset_hint = 0;
    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    for(Cell cell; remain; ) {
      cell.read(&ptr, &remain);
      add_raw(cell, &offset_hint);
    }
  }

  SWC_CAN_INLINE
  void add_raw(const DynamicBuffer& cells, const DB::Cell::Key& upto_key,
                                           const DB::Cell::Key& from_key,
                                           uint32_t skip, bool malformed) {
    size_t offset_hint = 0;
    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    for(Cell cell; remain; ) {
      cell.read(&ptr, &remain);
      if((malformed && !skip) || (
         (!upto_key.empty() &&
          DB::KeySeq::compare(key_seq, upto_key, cell.key)!=Condition::GT) ||
         DB::KeySeq::compare(key_seq, from_key, cell.key)==Condition::GT) ) {
        add_raw(cell, &offset_hint);
      } else {
        --skip;
      }
    }
  }

  SWC_CAN_INLINE
  void add_raw(const Cell& e_cell) {
    size_t offset = 0;
    add_raw(e_cell, &offset);
  }

  SWC_CAN_INLINE
  void add_raw(const Cell& e_cell, size_t* offsetp) {
    Iterator it = get<Iterator>(e_cell.key, *offsetp);

    if(e_cell.removal())
      _add_remove(e_cell, it, *offsetp);

    else if(Types::is_counter(type))
      _add_counter(e_cell, it, *offsetp);

    else
      _add_plain(e_cell, it, *offsetp);
  }

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold,
                      uint32_t max_cells);

  bool write_and_free(const DB::Cell::Key& key_start,
                      const DB::Cell::Key& key_finish,
                      DynamicBuffer& cells, uint32_t threshold);

  bool write_and_free(DynamicBuffer& cells, uint32_t threshold);


  constexpr SWC_CAN_INLINE
  void write(DynamicBuffer& cells) const {
    cells.ensure(_bytes);
    for(auto it = get<ConstIterator>(); it; ++it) {
      if(!it.item()->has_expired(ttl))
        it.item()->write(cells);
    }
  }


  SWC_CAN_INLINE
  void scan(ReqScan* req) const {
    if(_size) {
      max_revs == 1
        ? scan_version_single(req)
        : scan_version_multi(req);
    }
  }

  void scan_version_single(ReqScan* req) const;

  void scan_version_multi(ReqScan* req) const;

  void scan_test_use(const Specs::Interval& specs, DynamicBuffer& result,
                     size_t& count, size_t& skips) const;

  SWC_CAN_INLINE
  bool scan_after(const DB::Cell::Key& after, const DB::Cell::Key& to,
                  Mutable& cells) const {
    if(!_size)
      return false;

    const Cell* cell;
    for(auto it=get<ConstIterator>(after); it; ++it) {
      cell = it.item();
      if(!to.empty() &&
         DB::KeySeq::compare(key_seq, to, cell->key) == Condition::GT)
        return false;
      if(cell->has_expired(ttl) ||
         (!after.empty() &&
          DB::KeySeq::compare(key_seq, after, cell->key) != Condition::GT))
        continue;

      cells.add_raw(*cell);
    }
    return true;
  }


  SWC_CAN_INLINE
  bool can_split() const noexcept {
    return _container.size() > 1;
  }

  bool split(Mutable& cells, bool loaded);


  void split(Mutable& cells) {
    size_t split_at = _container.size() / 2;
    if(!split_at)
      return;
    cells.free();
    _container.split(split_at, cells._container);
    for(auto it = cells.get<ConstIterator>(); it; ++it) {
      cells._add(*it.item());
    }
    _size -= cells._size;
    _bytes -= cells._bytes;
  }


  SWC_CAN_INLINE
  void expand(Interval& interval) const {
    expand_begin(interval);
    if(_size > 1)
      expand_end(interval);
  }

  SWC_CAN_INLINE
  void expand_begin(Interval& interval) const {
    interval.expand_begin(front());
  }

  SWC_CAN_INLINE
  void expand_end(Interval& interval) const {
    interval.expand_end(back());
  }

  void print(std::ostream& out, bool with_cells=false) const;


  private:

  void _add_remove(const Cell& e_cell, Iterator& it, size_t& offset);

  void _add_plain(const Cell& e_cell, Iterator& it, size_t& offset);

  void _add_counter(const Cell& e_cell, Iterator& it, size_t& offset);

  constexpr SWC_CAN_INLINE
  void _add(const Cell& cell) noexcept {
    _bytes += cell.encoded_length();
    ++_size;
  }

  constexpr SWC_CAN_INLINE
  void _remove(const Cell& cell) noexcept {
    _bytes -= cell.encoded_length();
    --_size;
  }

  constexpr SWC_CAN_INLINE
  void _insert(Iterator& it, const Cell& cell) {
    Cell* add = new Cell(cell);
    _add(*add);
    it.insert(add);
  }

  constexpr SWC_CAN_INLINE
  void _remove(Iterator& it) {
    _remove(*it.item());
    delete it.item();
    it.remove();
  }

  constexpr SWC_CAN_INLINE
  void _remove(Iterator& it, size_t number, bool wdel = true) {
    if(wdel) {
      Iterator it_del(it);
      for(auto c = number; c && it_del; ++it_del,--c) {
        _remove(*it_del.item());
        delete it_del.item();
      }
    }
    it.remove(number);
  }

  constexpr SWC_CAN_INLINE
  void _remove_overhead(Iterator& it, const DB::Cell::Key& key,
                        uint32_t revs) {
    while(it && key.equal(it.item()->key)) {
      if(it.item()->flag == INSERT && ++revs > max_revs)
        _remove(it);
      else
        ++it;
    }
  }


};



}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Mutable.cc"
#endif

#endif // swcdb_db_Cells_Mutable_h
