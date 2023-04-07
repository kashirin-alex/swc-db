/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Cells_Mutable_h
#define swcdb_db_Cells_Mutable_h


#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/Interval.h"
#include "swcdb/core/VectorsVector.h"


namespace SWC { namespace DB { namespace Cells {

class ReqScan;


class Mutable final {

  /*
  using VectorT = std::vector<Cell*>;
  typedef Core::VectorsVector<
    std::vector<VectorT>,
    VectorT,
    8192
  > Container;
  */

  /**/
  using VectorT = Core::Vector<Cell*, uint32_t>;
  typedef Core::VectorsVector<
    Core::Vector<VectorT, uint32_t, 1>,
    VectorT,
    8192
  > Container;
  /**/

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
  Mutable(const Types::KeySeq a_key_seq,
          const uint32_t a_max_revs=1, const uint64_t ttl_ns=0,
          const Types::Column a_type=Types::Column::PLAIN)
          : _container(), _bytes(0), _size(0),
            key_seq(a_key_seq), type(a_type), max_revs(a_max_revs),
            ttl(ttl_ns) {
  }

  SWC_CAN_INLINE
  Mutable(const Types::KeySeq a_key_seq,
          const uint32_t a_max_revs, const uint64_t ttl_ns,
          const Types::Column a_type,
          const StaticBuffer& buffer)
          : _container(), _bytes(0), _size(0),
            key_seq(a_key_seq), type(a_type), max_revs(a_max_revs),
            ttl(ttl_ns) {
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
    free();
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

  void configure(const uint32_t revs, const uint64_t ttl_ns,
                 const Types::Column typ, bool finalized);

  SWC_CAN_INLINE
  ~Mutable() noexcept {
    for(auto it = get<ConstIterator>(); it; ++it)
      delete it.item();
  }

  SWC_CAN_INLINE
  void free() noexcept {
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
    configure(revs, ttl_ns, typ, false);
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
  constexpr// SWC_CAN_INLINE
  T get(size_t offset) noexcept {
    return _container.get<T>(offset);
  }

  template<typename T>
  constexpr// SWC_CAN_INLINE
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

  SWC_CAN_INLINE
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


  SWC_CAN_INLINE
  void takeout(size_t pos, Cell*& cell) {
    if(Iterator it = get<Iterator>(pos)) {
      _remove(*it.item());
      cell = std::move(it.item());
      it.remove();
    }
  }

  SWC_CAN_INLINE
  void takeout_begin(size_t pos, Cell*& cell) {
    takeout(pos, cell);
  }

  SWC_CAN_INLINE
  void takeout_end(size_t pos, Cell*& cell) {
    takeout(_size - pos, cell);
  }


  SWC_CAN_INLINE
  void add_sorted(const Cell& cell, bool no_value=false) {
    add_sorted(new Cell(cell, no_value));
  }

  SWC_CAN_INLINE
  void add_sorted(Cell* cell) {
    _add(*cell);
    _container.push_back(cell);
  }

  SWC_CAN_INLINE
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
  void add_raw(const DynamicBuffer& cells, bool finalized) {
    size_t offset_hint = 0;
    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    for(Cell cell; remain; ) {
      cell.read(&ptr, &remain, false);
      add_raw(cell, &offset_hint, finalized);
    }
  }

  SWC_CAN_INLINE
  void add_raw(const DynamicBuffer& cells, const DB::Cell::Key& upto_key,
                                           const DB::Cell::Key& from_key,
                                           uint32_t skip, bool malformed,
                                           bool finalized) {
    size_t offset_hint = 0;
    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    for(Cell cell; remain; ) {
      cell.read(&ptr, &remain, false);
      if((malformed && !skip) || (
         (!upto_key.empty() &&
          DB::KeySeq::compare(key_seq, upto_key, cell.key)!=Condition::GT) ||
         DB::KeySeq::compare(key_seq, from_key, cell.key)==Condition::GT) ) {
        add_raw(cell, &offset_hint, finalized);
      } else {
        --skip;
      }
    }
  }

  SWC_CAN_INLINE
  void add_raw(const Cell& e_cell, bool finalized) {
    size_t offset = 0;
    add_raw(e_cell, &offset, finalized);
  }

  SWC_CAN_INLINE
  void add_raw(const Cell& e_cell, size_t* offsetp, bool finalized) {
    Iterator it = get<Iterator>(e_cell.key, *offsetp);
    add_raw(e_cell, offsetp, it, finalized);
  }

  SWC_CAN_INLINE
  void add_raw(const Cell& e_cell, size_t* offsetp, Iterator& it,
               bool finalized) {
    e_cell.removal()
      ? _add_remove(e_cell, it, *offsetp)
      : (Types::is_counter(type)
          ? (finalized
              ? _add_counter(e_cell, it, *offsetp)
              : _add_unfinalized(e_cell, it, *offsetp))
          : (max_revs == 1
              ? _add_plain_version_single(e_cell, it, *offsetp)
              : _add_plain_version_multi(e_cell, it, *offsetp)
            )
        );
  }

  SWC_CAN_INLINE
  void finalize_raw() {
    if(size() > 1) {
      if(Types::is_counter(type)) {
        _finalize_counter();
      }
    }
  }

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold,
                      uint32_t max_cells);

  bool write_and_free(const DB::Cell::Key& key_start,
                      const DB::Cell::Key& key_finish,
                      DynamicBuffer& cells, uint32_t threshold);

  bool write_and_free(DynamicBuffer& cells, uint32_t threshold);


  SWC_CAN_INLINE
  void write(DynamicBuffer& cells) const {
    cells.ensure(_bytes);
    for(auto it = get<ConstIterator>(); it; ++it) {
      if(!it.item()->has_expired(ttl))
        it.item()->write(cells);
    }
  }

  SWC_CAN_INLINE
  void check_sequence(const char* msg, bool w_assert=true) const {
    auto it = get<ConstIterator>();
    for(Cell* cell; it; ) {
      cell = it.item();
      if(!it.next())
        break;
      if(DB::KeySeq::compare(key_seq, it.item()->key, cell->key) == Condition::GT) {
        SWC_LOG_OUT(
          LOG_ERROR,
          SWC_LOG_OSTREAM << "BAD cells-sequence at " << msg;
          cell->key.print(SWC_LOG_OSTREAM << "\n current-");
          it.item()->key.print(SWC_LOG_OSTREAM << "\n next-");
        );
        SWC_ASSERT(!w_assert);
      }
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
                  Mutable& cells, bool finalized) const {
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

      cells.add_raw(*cell, finalized);
    }
    return true;
  }


  SWC_CAN_INLINE
  bool split(Mutable& cells, size_t count, size_t bytes, bool loaded) {
    if(_size < 2 || (count > _size && bytes > _bytes))
      return false;

    if(_container.size() > 1) { // try container split
      size_t split_at = _container.size() / 2;
      for(auto it = _container.cbegin() + split_at - 1; ; ++it) {
        auto it_nxt = it + 1;
        if(!it->back()->key.equal(it_nxt->front()->key)) {
          _container.split(split_at, cells._container);
          for(auto it_cell = cells.get<ConstIterator>(); it_cell; ++it_cell) {
            cells._add(*it_cell.item());
          }
          _size -= cells._size;
          _bytes -= cells._bytes;
          if(!loaded)
            cells.free();
          return true;
        }
        if(++split_at == _container.size())
          break;
      }
    }

    if(!count)
      count = _size / 2;
    if(!bytes)
      bytes = _bytes / 2;
    size_t chk_size = 0;
    size_t chk_bytes = 0;
    bool found_at = false;
    auto it = get<Iterator>();
    Iterator it_at = get<Iterator>();
    for(Cell* cell; it; ) {
      cell = it.item();
      if(!it.next())
        break;
      ++chk_size;
      chk_bytes += cell->encoded_length();
      if((chk_size >= count || chk_bytes >= bytes) &&
         !cell->key.equal(it.item()->key)) {
        it_at = it;
        found_at = true;
        break;
      }
    }
    if(!found_at)
      return false;

    for(; it; ++it) {
      if(!loaded || it.item()->has_expired(ttl)) {
        delete it.item();
      } else {
        cells.add_sorted(it.item());
      }
    }
    it_at.remove(_size - chk_size);
    _size = chk_size;
    _bytes = chk_bytes;
    return true;
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

  void _add_unfinalized(const Cell& e_cell, Iterator& it, size_t& offset);

  void _add_plain_version_single(const Cell& e_cell,
                                 Iterator& it, size_t& offset);

  void _add_plain_version_multi(const Cell& e_cell,
                                Iterator& it, size_t& offset);

  void _add_counter(const Cell& e_cell, Iterator& it, size_t& offset);

  void _finalize_counter();

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

  SWC_CAN_INLINE
  void _adjust_copy(Cell& cell, const Cell& other) {
    _bytes -= cell.encoded_length();
    cell.copy(other);
    _bytes += cell.encoded_length();
  }

  SWC_CAN_INLINE
  void _insert(Iterator& it, const Cell& cell) {
    Cell* add = new Cell(cell);
    _add(*add);
    it.insert(add);
  }

  SWC_CAN_INLINE
  void _remove(Iterator& it) {
    _remove(*it.item());
    delete it.item();
    it.remove();
  }

  SWC_CAN_INLINE
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

  SWC_CAN_INLINE
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


#include "swcdb/db/Cells/ReqScan.h"

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Mutable.cc"
#endif

#endif // swcdb_db_Cells_Mutable_h
