/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Cells_Mutable_h
#define swcdb_db_Cells_Mutable_h


#include "swcdb/db/Cells/ReqScan.h"
#include "swcdb/core/VectorsVector.h"
//#include "swcdb/core/ArraysArraysVector.h"


namespace SWC { namespace DB { namespace Cells {


class Mutable final {

  typedef Core::VectorsVector<std::vector<Cell*>, 4096> Container;

  /*
  typedef Core::ArraysArraysVector<
    Core::ArraysArray < Core::Array<Cell*, uint8_t, 127>, uint8_t, 127 >
  > Container;
  */
  /*
  "swcdb/core/ArraysArraysVector.h"
  > Vector: (no ptr Cell)
   Array<Cell, uint8_t, 83>           (83 x 48 bytes + 8 bytes)  = 3992 bytes
   ArraysArray<Array*, uint8_t, 100> (100 x 8 bytes + 8 bytes)   = 808 bytes
   ArraysArraysVector<ArraysArray*>  (N x   8 bytes + 16 bytes)   = 16 + 8xN
  > Vector: (Pointer Cell)
   Array<Cell*, uint8_t, 127>         (127  x 8 bytes + 8 bytes) = 1024 bytes
   ArraysArray<Array*, uint8_t, 100> (127  x 8 bytes + 8 bytes)  = 1024 bytes
   ArraysArraysVector<ArraysArray*>  (N    x 8 bytes + 16 bytes)  = 16 + 8xN
  */


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


  explicit Mutable(const Types::KeySeq key_seq,
                   const uint32_t max_revs=1, const uint64_t ttl_ns=0,
                   const Types::Column type=Types::Column::PLAIN);

  explicit Mutable(const Types::KeySeq key_seq,
                   const uint32_t max_revs, const uint64_t ttl_ns,
                   const Types::Column type,
                   const StaticBuffer& buffer);

  explicit Mutable(Mutable&& other) noexcept;

  Mutable(const Mutable&) = delete;

  Mutable& operator=(const Mutable& other) = delete;

  Mutable& operator=(Mutable&& other) noexcept;

  void take_sorted(Mutable& other);

  ~Mutable();

  void free();

  SWC_CAN_INLINE
  void reset(const uint32_t revs=1, const uint64_t ttl_ns=0,
             const Types::Column typ=Types::Column::PLAIN) {
    free();
    configure(revs, ttl_ns, typ);
  }

  void configure(const uint32_t revs=1, const uint64_t ttl_ns=0,
                 const Types::Column typ=Types::Column::PLAIN) noexcept;


  template<typename T>
  SWC_CAN_INLINE
  T get() noexcept {
    return _container.get<T>();
  }

  template<typename T>
  SWC_CAN_INLINE
  T get() const noexcept {
    return _container.get<T>();
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(size_t offset) noexcept {
    return _container.get<T>(offset);
  }

  template<typename T>
  SWC_CAN_INLINE
  T get(size_t offset) const noexcept {
    return _container.get<T>(offset);
  }


  Iterator GetIterator() noexcept {
    return get<Iterator>();
  }

  Iterator GetIterator(size_t offset) noexcept {
    return get<Iterator>(offset);
  }

  ConstIterator GetConstIterator() const noexcept {
    return get<ConstIterator>();
  }

  ConstIterator GetConstIterator(size_t offset) const noexcept {
    return get<ConstIterator>(offset);
  }


  SWC_CAN_INLINE
  size_t size() const noexcept {
    return _size;
  }

  SWC_CAN_INLINE
  size_t size_bytes() const noexcept {
    return _bytes;
  }

  SWC_CAN_INLINE
  bool empty() const noexcept {
    return !_size;
  }

  size_t size_of_internal() const noexcept;

  Cell& front() noexcept;

  Cell& back() noexcept;

  Cell& front() const noexcept;

  Cell& back() const noexcept;

  Cell* operator[](size_t idx) noexcept;

  bool has_one_key() const noexcept;


  void add_sorted(const Cell& cell, bool no_value=false);

  SWC_CAN_INLINE
  void add_sorted_no_cpy(Cell* cell) {
    _add(cell);
    _container.push_back(cell);
  }

  size_t add_sorted(const uint8_t* ptr, size_t remain);


  void add_raw(const DynamicBuffer& cells);

  void add_raw(const DynamicBuffer& cells, const DB::Cell::Key& upto_key,
                                           const DB::Cell::Key& from_key,
                                           uint32_t skip, bool malformed);

  void add_raw(const Cell& e_cell);

  void add_raw(const Cell& e_cell, size_t* offsetp);

  Cell* takeout(size_t idx);

  SWC_CAN_INLINE
  Cell* takeout_begin(size_t idx) {
    return takeout(idx);
  }

  SWC_CAN_INLINE
  Cell* takeout_end(size_t idx) {
    return takeout(_size - idx);
  }

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold,
                      uint32_t max_cells);

  bool write_and_free(const DB::Cell::Key& key_start,
                      const DB::Cell::Key& key_finish,
                      DynamicBuffer& cells, uint32_t threshold);

  bool write_and_free(DynamicBuffer& cells, uint32_t threshold);


  void print(std::ostream& out, bool with_cells=false) const;


  void get(int32_t idx, DB::Cell::Key& key) const;

  bool get(const DB::Cell::Key& key, Condition::Comp comp,
           DB::Cell::Key& res) const;

  void write(DynamicBuffer& cells) const;


  void scan(ReqScan* req) const;

  void scan_version_single(ReqScan* req) const;

  void scan_version_multi(ReqScan* req) const;

  void scan_test_use(const Specs::Interval& specs, DynamicBuffer& result,
                     size_t& count, size_t& skips) const;

  bool scan_after(const DB::Cell::Key& after, const DB::Cell::Key& to,
                  Mutable& cells) const;

  void expand(Interval& interval) const;

  void expand_begin(Interval& interval) const;

  void expand_end(Interval& interval) const;


  bool split(Mutable& cells, bool loaded);

  SWC_CAN_INLINE
  bool can_split() const noexcept {
    return _container.size() > 1;
  }

  void split(Mutable& cells);

  private:

  void _add_remove(const Cell& e_cell, size_t* offsetp);

  void _add_plain(const Cell& e_cell, size_t* offsetp);

  void _add_counter(const Cell& e_cell, size_t* offsetp);


  size_t _narrow(const DB::Cell::Key& key, size_t offset_hint = 0) const;

  size_t _narrow(const Specs::Interval& specs) const;


  void _add(Cell* cell) noexcept;

  void _remove(Cell* cell) noexcept;

  Cell* _insert(Iterator& it, const Cell& cell);

  void _remove(Iterator& it);

  void _remove(Iterator& it, size_t number, bool wdel = true);

  void _remove_overhead(Iterator& it, const DB::Cell::Key& key,
                        uint32_t revs);


  Container             _container;
  size_t                _bytes;
  size_t                _size;

};



SWC_CAN_INLINE
Mutable::Mutable(const Types::KeySeq key_seq,
                 const uint32_t max_revs, const uint64_t ttl_ns,
                 const Types::Column type)
                : key_seq(key_seq),
                  type(type), max_revs(max_revs), ttl(ttl_ns),
                  _bytes(0), _size(0) {
}

SWC_CAN_INLINE
Mutable::Mutable(const Types::KeySeq key_seq,
                 const uint32_t max_revs, const uint64_t ttl_ns,
                 const Types::Column type,
                 const StaticBuffer& buffer)
                : key_seq(key_seq),
                  type(type), max_revs(max_revs), ttl(ttl_ns),
                  _bytes(0), _size(0) {
  add_sorted(buffer.base, buffer.size);
}

SWC_CAN_INLINE
Mutable::Mutable(Mutable&& other) noexcept
                : key_seq(other.key_seq), type(other.type),
                  max_revs(other.max_revs), ttl(other.ttl),
                  _container(std::move(other._container)),
                  _bytes(other._bytes), _size(other._size) {
  other._bytes = 0;
  other._size = 0;
}

SWC_CAN_INLINE
Mutable& Mutable::operator=(Mutable&& other) noexcept {
  type = other.type;
  max_revs = other.max_revs;
  ttl = other.ttl;
  _container = std::move(other._container);
  _bytes = other._bytes;
  _size = other._size;
  other._bytes = 0;
  other._size = 0;
  return *this;
}

SWC_CAN_INLINE
Mutable::~Mutable() {
  for(auto it = GetConstIterator(); it; ++it)
    delete it.item();
}

SWC_CAN_INLINE
void Mutable::free() {
  for(auto it = GetConstIterator(); it; ++it)
    delete it.item();
  _container.clear();
  _container.shrink_to_fit();
  _bytes = 0;
  _size = 0;
}

SWC_CAN_INLINE
void Mutable::configure(const uint32_t revs, const uint64_t ttl_ns,
                        const Types::Column typ) noexcept {
  type = typ;
  max_revs = revs;
  ttl = ttl_ns;
}

SWC_CAN_INLINE
size_t Mutable::size_of_internal() const noexcept {
  return  _bytes
        + _size * sizeof(Cell)
        + _container.size_of_internal();
}

SWC_CAN_INLINE
Cell& Mutable::front() noexcept {
  return *_container.front();
}

SWC_CAN_INLINE
Cell& Mutable::back() noexcept {
  return *_container.back();
}

SWC_CAN_INLINE
Cell& Mutable::front() const noexcept {
  return *_container.front();
}

SWC_CAN_INLINE
Cell& Mutable::back() const noexcept {
  return *_container.back();
}

SWC_CAN_INLINE
Cell* Mutable::operator[](size_t pos) noexcept {
  auto it = get<Iterator>(pos);
  return it ? it.item() : nullptr;
}

SWC_CAN_INLINE
bool Mutable::has_one_key() const noexcept {
  return front().key.equal(back().key);
}


SWC_CAN_INLINE
void Mutable::add_raw(const Cell& e_cell) {
  size_t offset = _narrow(e_cell.key);

  if(e_cell.removal())
    _add_remove(e_cell, &offset);

  else if(Types::is_counter(type))
    _add_counter(e_cell, &offset);

  else
    _add_plain(e_cell, &offset);
}

SWC_CAN_INLINE
void Mutable::add_raw(const Cell& e_cell, size_t* offsetp) {
  *offsetp = _narrow(e_cell.key, *offsetp);

  if(e_cell.removal())
    _add_remove(e_cell, offsetp);

  else if(Types::is_counter(type))
    _add_counter(e_cell, offsetp);

  else
    _add_plain(e_cell, offsetp);
}

SWC_CAN_INLINE
void Mutable::add_sorted(const Cell& cell, bool no_value) {
  Cell* add = new Cell(cell, no_value);
  _add(add);
  _container.push_back(add);
}

SWC_CAN_INLINE
void Mutable::scan(ReqScan* req) const {
  if(_size) {
    max_revs == 1
     ? scan_version_single(req)
     : scan_version_multi(req);
  }
}

SWC_CAN_INLINE
void Mutable::get(int32_t idx, DB::Cell::Key& key) const {
  if((idx < 0 && _size < size_t(-idx)) || size_t(idx) >= _size)
    return;
  auto it = GetConstIterator(idx < 0 ? _size + idx : idx);
  if(it)
    key.copy(it.item()->key);
}

SWC_CAN_INLINE
bool Mutable::get(const DB::Cell::Key& key, Condition::Comp comp,
                  DB::Cell::Key& res) const {
  Condition::Comp chk;
  for(auto it = GetConstIterator(_narrow(key)); it; ++it) {
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
bool Mutable::scan_after(const DB::Cell::Key& after,
                         const DB::Cell::Key& to, Mutable& cells) const {
  if(!_size)
    return false;

  const Cell* cell;
  for(auto it=GetConstIterator(_narrow(after)); it; ++it) {
    cell = it.item();
    if(!to.empty()
        && DB::KeySeq::compare(key_seq, to, cell->key) == Condition::GT)
      return false;
    if(cell->has_expired(ttl) || (!after.empty()
        && DB::KeySeq::compare(key_seq, after, cell->key) != Condition::GT))
      continue;

    cells.add_raw(*cell);
  }
  return true;
}

SWC_CAN_INLINE
void Mutable::expand(Interval& interval) const {
  expand_begin(interval);
  if(_size > 1)
    expand_end(interval);
}

SWC_CAN_INLINE
void Mutable::expand_begin(Interval& interval) const {
  interval.expand_begin(front());
}

SWC_CAN_INLINE
void Mutable::expand_end(Interval& interval) const {
  interval.expand_end(back());
}

SWC_CAN_INLINE
size_t Mutable::_narrow(const Specs::Interval& specs) const {
  return specs.offset_key.empty()
    ? (specs.range_begin.empty() ? 0 : _narrow(specs.range_begin))
    : _narrow(specs.offset_key);
}

/*
template<typename IteratorT>
SWC_CAN_INLINE
size_t Mutable::narrow(const Specs::Interval& specs) const {
  return specs.offset_key.empty()
    ? (specs.range_begin.empty() ? 0 : narrow<IteratorT>(specs.range_begin))
    : narrow<IteratorT>(specs.offset_key);
}
*/


SWC_CAN_INLINE
void Mutable::_add(Cell* cell) noexcept {
  _bytes += cell->encoded_length();
  ++_size;
}

SWC_CAN_INLINE
void Mutable::_remove(Cell* cell) noexcept {
  _bytes -= cell->encoded_length();
  --_size;
}


SWC_CAN_INLINE
Cell* Mutable::_insert(Mutable::Iterator& it, const Cell& cell) {
  Cell* add = new Cell(cell);
  _add(add);
  it ? it.insert(add) : _container.push_back(add);
  return add;
}

SWC_CAN_INLINE
void Mutable::_remove(Mutable::Iterator& it) {
  _remove(it.item());
  delete it.item();
  it.remove();
}

SWC_CAN_INLINE
void Mutable::_remove(Mutable::Iterator& it, size_t number, bool wdel) {
  if(wdel) {
    Iterator it_del(it);
    for(auto c = number; c && it_del; ++it_del,--c) {
      _remove(it_del.item());
      delete it_del.item();
    }
  }
  it.remove(number);
}

SWC_CAN_INLINE
void Mutable::_remove_overhead(Mutable::Iterator& it,
                               const DB::Cell::Key& key,
                               uint32_t revs) {
  while(it && key.equal(it.item()->key)) {
    if(it.item()->flag == INSERT && ++revs > max_revs)
      _remove(it);
    else
      ++it;
  }
}



}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Mutable.cc"
#endif

#endif // swcdb_db_Cells_Mutable_h
