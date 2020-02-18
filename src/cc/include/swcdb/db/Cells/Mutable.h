/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Cells_Mutable_h
#define swcdb_lib_db_Cells_Mutable_h

#include <functional>

#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/SpecsInterval.h"
#include "swcdb/db/Cells/Interval.h"

namespace SWC { namespace DB { namespace Cells {


class Mutable final {

  static const uint32_t narrow_sz = 20;

  public:

  typedef std::shared_ptr<Mutable>          Ptr;
  typedef std::function<bool(const Cell&, bool&)>  Selector_t;
  
  static const uint8_t  _cell_sz = sizeof(Cell*);
  
  inline static Ptr make(const uint32_t cap=0, 
                         const uint32_t max_revs=1, 
                         const uint32_t ttl=0, 
                         const Types::Column type=Types::Column::PLAIN) {
    return std::make_shared<Mutable>(cap, max_revs, ttl, type);
  }

  Types::Column     type;
  
  explicit Mutable(const uint32_t cap=0, 
                   const uint32_t max_revs=1, 
                   const uint32_t ttl=0, 
                   const Types::Column type=Types::Column::PLAIN)
                  : type(type), 
                    m_cells(0), m_cap(cap), m_size(0), m_size_bytes(0), 
                    m_max_revs(max_revs), m_ttl(ttl*1000000000) {
    if(m_cap)
      _allocate();
  }

  explicit Mutable(Mutable& other)
                  : type(other.type),
                    m_cells(other.m_cells), m_cap(other.m_cap), 
                    m_size(other.m_size), m_size_bytes(other.m_size_bytes), 
                    m_max_revs(other.m_max_revs), m_ttl(other.m_ttl) {
    if(other.m_cells) {
      other.m_cells = 0;
      other.m_cap = 0;
      other.m_size = 0;
      other.m_size_bytes = 0;
    }
  }

  void reset(const uint32_t cap=0, const uint32_t max_revs=1, 
             const uint32_t ttl=0, 
             const Types::Column typ=Types::Column::PLAIN) {
    free();
    m_cap = cap;
    configure(max_revs, ttl, typ);
    if(m_cap)
      _allocate();
  }

  Mutable& operator=(const Mutable& other) = delete;

  ~Mutable() {
    free();
  }

  void configure(const uint32_t max_revs=1, 
                 const uint32_t ttl=0, 
                 const Types::Column typ=Types::Column::PLAIN) {
    m_max_revs = max_revs;
    m_ttl = ttl*1000000000;
    type = typ;
  }

  void ensure(uint32_t sz) {
    if(m_cap < m_size + sz){
      m_cap += sz;
      _allocate();
    }
  }

  void free() {
    if(m_cells) {
      if(m_size) do { 
        --m_size;
        delete *(m_cells+m_size);
        //(*(m_cells+m_size))->~Cell();
        //std::free(*(m_cells+m_size));
      } while(m_size);

      delete [] m_cells;
      //std::free(m_cells);

      m_cells = 0;
      m_cap = 0;
      m_size_bytes = 0;
    }
  }
  
  const uint32_t size() const {
    return m_size;
  }
  
  const uint32_t size_bytes() const {
    return m_size_bytes;
  }
  
  void get(int32_t idx, Cell& cell) const {
    cell.copy(**(m_cells+(idx < 0? m_size+idx: idx)));
  }

  void get(int32_t idx, DB::Cell::Key& key) const {
    key.copy((*(m_cells+(idx < 0? m_size+idx: idx)))->key);
  }

  void get(int32_t idx, DB::Cell::Key& key, int64_t& ts) const {
    Cell* cell = *(m_cells+(idx < 0? m_size+idx: idx));
    key.copy(cell->key);
    ts = cell->timestamp;
  }
   
  const bool get(const DB::Cell::Key& key, Condition::Comp comp, 
                 DB::Cell::Key& res) const {
    Cell* ptr;
    Condition::Comp chk;

    for(uint32_t offset = _narrow(key); offset < m_size; ++offset) {
      ptr = *(m_cells + offset);
      if((chk = key.compare(ptr->key, 0)) == Condition::GT 
        || (comp == Condition::GE && chk == Condition::EQ)){
        res.copy(ptr->key);
        return true;
      }
    }
    return false;
  }
   
  const bool get(const DB::Cell::Key& key, Condition::Comp comp, 
                 Cell& cell) const {
    Cell* ptr;
    Condition::Comp chk;

    for(uint32_t offset = _narrow(key); offset < m_size; ++offset) {
      ptr = *(m_cells + offset);
      if((chk = key.compare(ptr->key, 0)) == Condition::GT 
        || (comp == Condition::GE && chk == Condition::EQ)){
        cell.copy(*ptr);
        return true;
      }
    }
    return false;
  }

  bool get(const Specs::Key& key, Cell& cell) const {
    Cell* ptr;

    for(uint32_t offset = 0; offset < m_size; ++offset) {
      ptr = *(m_cells + offset);
      if(key.is_matching(ptr->key)) {
        cell.copy(*ptr);
        return true;
      }
    }
    return false;
  }

  const Condition::Comp compare(int32_t idx, const DB::Cell::Key& key) const {
    return (*(m_cells+(idx < 0? m_size+idx: idx)))->key.compare(key);
  }

  const bool has_one_key() const {
    return (*m_cells)->key.compare(
      (*(m_cells+m_size-1))->key) == Condition::EQ;
  }

  void push_back(const Cell& cell, bool no_value=false) {
    ensure(1);
    *(m_cells + m_size) = new Cell(cell, no_value);
    ++m_size;
    m_size_bytes += cell.encoded_length();
  }

  void push_back_nocpy(Cell* cell) {
    ensure(1);
    *(m_cells + m_size) = cell;
    ++m_size;
    m_size_bytes += cell->encoded_length();
  }
  
  void insert(uint32_t offset, const Cell& cell) {
    _move_fwd(offset, 1);
    *(m_cells + offset) = new Cell(cell);
    //new(*(m_cells + offset) = (Cell*)std::malloc(sizeof(Cell))) Cell(cell);
    ++m_size;
    m_size_bytes += cell.encoded_length();
  }

  void add(const Cell& e_cell) {
    if(e_cell.has_expired(m_ttl))
      return;

    uint32_t offset = _narrow(e_cell.key);

    if(e_cell.removal()) {
      add_remove(e_cell, offset);
      return;
    }

    if(Types::is_counter(type))
      add_counter(e_cell, offset);
    else
      add_plain(e_cell, offset);
  }

  Cell* get_next(uint32_t offset) {
    Cell* cell;
    while(m_size > offset) {
      cell = *(m_cells + offset);
      if(cell->has_expired(m_ttl)) {
         _move_bwd(offset, 1);
        continue;
      }
      return cell;
    }
    return nullptr;
  }

  void add_remove(const Cell& e_cell, uint32_t offset) {
    Condition::Comp cond;
    int64_t revision_new = e_cell.get_revision();

    for(Cell* cell; cell=get_next(offset); ++offset) {

      cond = cell->key.compare(e_cell.key, 0);
      if(cond == Condition::GT)
        continue;

      if(cond == Condition::LT) {
        insert(offset, e_cell);
        return;
      }

      if(cell->removal() && cell->is_removing(revision_new))
        return;
      
      if(e_cell.is_removing(cell->get_revision()))
        _move_bwd(offset--, 1);
    }
    
    push_back(e_cell);
  }

  void add_plain(const Cell& e_cell, uint32_t offset) {
    Condition::Comp cond;
    int64_t revision_new = e_cell.get_revision();
    uint32_t revs = 0;

    for(Cell* cell; cell=get_next(offset); ++offset) {

      cond = cell->key.compare(e_cell.key, 0);
      if(cond == Condition::GT)
        continue;

      if(cond == Condition::LT) {
        insert(offset, e_cell);
        return;
      }

      if(cell->removal()) {
        if(cell->is_removing(revision_new))
          return;
        continue;
      }

      if(revision_new != AUTO_ASSIGN && cell->get_revision() == revision_new) {
        cell->copy(e_cell);
        return;
      }
      
      ++revs;
      if(e_cell.control & TS_DESC 
          ? e_cell.timestamp < cell->timestamp
          : e_cell.timestamp > cell->timestamp) {
        if(m_max_revs == revs)
          return;
        continue;
      }
      
      if(m_max_revs == revs) {
        cell->copy(e_cell);
      } else {
        insert(offset, e_cell);
        _remove_overhead(++offset, e_cell.key, revs);
      } 
      return;
    }

    push_back(e_cell);
  }

  void add_counter(const Cell& e_cell, uint32_t offset) {
    Condition::Comp cond;

    int64_t revision = e_cell.get_revision();
    uint32_t add_offset = m_size;
    Cell* cell;
    for(; cell=get_next(offset); ++offset) {

      cond = cell->key.compare(e_cell.key, 0);
      if(cond == Condition::GT)
        continue;

      if(cond == Condition::LT) {
        add_offset = offset;
        goto add_counter;
      }

      if(cell->removal()) {
        if(cell->is_removing(revision))
          return;
        continue;
      }

      uint8_t op_1;
      int64_t eq_rev_1;
      int64_t value_1 = cell->get_counter(op_1, eq_rev_1);
      if(op_1 & OP_EQUAL) {
        if(!(op_1 & HAVE_REVISION))
          eq_rev_1 = cell->get_revision();
        if(eq_rev_1 > revision)
          return;
      }
      
      if(e_cell.get_counter_op() & OP_EQUAL)
        cell->copy(e_cell);
      else {
        value_1 += e_cell.get_counter();
        cell->set_counter(op_1, value_1, type, eq_rev_1);
        if(cell->timestamp < e_cell.timestamp) {
          cell->timestamp = e_cell.timestamp;
          //cell->revision = e_cell.revision;
          cell->control = e_cell.control;
        }
      }
      return;
    }

    add_counter:
      insert(add_offset, e_cell);
      if(type != Types::Column::COUNTER_I64) {
        cell = *(m_cells+add_offset);
        uint8_t op_1;
        int64_t eq_rev_1;
        int64_t value_1 = cell->get_counter(op_1, eq_rev_1);
        cell->set_counter(op_1, value_1, type, eq_rev_1);
      }
  }

  void pop(int32_t idx, Cell& cell) {
    uint32_t offset = idx < 0? m_size+idx: idx;
    cell.copy(**(m_cells+offset));
    
    _move_bwd(offset, 1);
  }

  void scan(Interval& interval, Mutable& cells) const {
    if(!m_size)
      return;
    Cell* cell;

    for(uint32_t offset = _narrow(interval.key_begin);
        offset < m_size; ++offset){
      cell = *(m_cells + offset);

      if(cell->has_expired(m_ttl) || (!interval.key_begin.empty() 
          && interval.key_begin.compare(cell->key) == Condition::LT))
        continue;
      if(!interval.key_end.empty() 
          && interval.key_end.compare(cell->key) == Condition::GT)
        break; 

      cells.add(*cell);
      
      //cell->key.align(interval.aligned_min, interval.aligned_max);
      //intval.expand(cell->timestamp);
    }
  }

  void scan(const Specs::Interval& specs, Mutable& cells, 
            size_t& cell_offset, const std::function<bool()>& reached_limits, 
            size_t& skips, const Selector_t& selector) const {
    if(!m_size)
      return;
    if(m_max_revs == 1) 
      scan_version_single(
        specs, cells, cell_offset, reached_limits, skips, selector);
    else
      scan_version_multi(
        specs, cells, cell_offset, reached_limits, skips, selector);
  }

  void scan_version_single(const Specs::Interval& specs, Mutable& cells, 
                           size_t& cell_offset, 
                           const std::function<bool()>& reached_limits, 
                           size_t& skips, const Selector_t& selector) const {
    bool stop = false;
    bool only_deletes = specs.flags.is_only_deletes();
    bool only_keys = specs.flags.is_only_keys();

    uint32_t offset = specs.offset_key.empty()? 0 : _narrow(specs.offset_key);
                                               // ?specs.key_start
    for(Cell* cell; !stop && offset < m_size; ++offset){
      cell = *(m_cells + offset);

      if(!cell->has_expired(m_ttl) &&
         (only_deletes ? cell->flag != INSERT : cell->flag == INSERT) &&
         selector(*cell, stop)) {
        
        if(cell_offset) {
          --cell_offset;
          ++skips;  
          continue;
        }

        cells.push_back(*cell, only_keys);
        if(reached_limits())
          break;
      } else 
        ++skips;
    }
  }

  void scan_version_multi(const Specs::Interval& specs, Mutable& cells, 
                          size_t& cell_offset, 
                          const std::function<bool()>& reached_limits, 
                          size_t& skips, const Selector_t& selector) const {
    bool stop = false;
    bool only_deletes = specs.flags.is_only_deletes();
    bool only_keys = specs.flags.is_only_keys();
    
    bool chk_align;
    uint32_t rev;
    uint32_t offset;
    if((chk_align = !specs.offset_key.empty())) {
      rev = cells.m_max_revs;
      offset = _narrow(specs.offset_key);// ?specs.key_start
    } else {
      rev = 0;
      offset = 0;
    }
    
    for(Cell* cell; !stop && offset < m_size; ++offset) {
      cell = *(m_cells + offset);

      if((only_deletes ? cell->flag == INSERT : cell->flag != INSERT) || 
         cell->has_expired(m_ttl)) {
        ++skips;
        continue;
      }

      if(chk_align) switch(specs.offset_key.compare(cell->key)) {
        case Condition::LT: {
          ++skips;
          continue;
        }
        case Condition::EQ: {
          if(!rev ||
             !specs.is_matching(cell->timestamp, cell->control & TS_DESC)) {
            if(rev)
              --rev;
            //if(cell_offset && selector(*cell, stop))
            //  --cell_offset;
            ++skips;
            continue;
          }
        }
        default:
          chk_align = false;
          break;
      }

      if(!selector(*cell, stop)) {
        ++skips;
        continue;
      }
      if(cells.size() && cells.compare(-1, cell->key) == Condition::EQ) {
        if(!rev) {
          ++skips;
          continue;
        }
      } else {
        rev = cells.m_max_revs;
      }

      if(cell_offset) {
        --cell_offset;
        ++skips;
        continue;
      }

      cells.push_back(*cell, only_keys);
      if(reached_limits())
        break;
      --rev;
    }
  }
  

  void scan_test_use(const Specs::Interval& specs, DynamicBuffer& result, 
            size_t& count, size_t& skips) const {
    Cell* cell;
    uint32_t offset = 0;
    uint cell_offset = specs.flags.offset;
    bool only_deletes = specs.flags.is_only_deletes();

    for(; offset < m_size; ++offset) {
      cell = *(m_cells + offset);

      if(!cell->has_expired(m_ttl) && 
         (only_deletes ? cell->flag != INSERT : cell->flag == INSERT) &&
         specs.is_matching(*cell, type)) {

        if(cell_offset) {
          --cell_offset;
          ++skips;
          continue;
        }
        
        cell->write(result);
        if(++count == specs.flags.limit) 
          // specs.flags.limit_by && specs.flags.max_versions
          break;
      } else 
        ++skips;
    }
  }

  void write(DynamicBuffer& cells) const {
    Cell* cell;
    cells.ensure(m_size_bytes);

    for(uint32_t offset = 0; offset < m_size; ++offset) {
      cell = *(m_cells + offset);
      
      if(cell->has_expired(m_ttl))
        continue;
        
      cell->write(cells);
    }
  }

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold, 
                      uint32_t max_cells) {
    if(!m_size)
      return;
    Cell* cell;
    Cell* first = nullptr;
    Cell* last = nullptr;
    size_t count = 0;
    cells.ensure(m_size_bytes < threshold? m_size_bytes: threshold);
    uint32_t offset = 0;
    for(;offset < m_size 
          && ((!threshold || threshold > cells.fill()) 
              && (!max_cells || max_cells > count) ); 
        ++offset) {
      cell = *(m_cells + offset);
      
      if(cell->has_expired(m_ttl))
        continue;
      
      cell->write(cells);
      ++count;

      if(!first)
        first = cell;
      else 
        last = cell;

      intval.expand(cell->timestamp);
      cell->key.align(intval.aligned_min, intval.aligned_max);
    }
    if(first) {
      intval.expand_begin(*first);
      intval.expand_end(*(last ? last : first));
    }
    
    if(m_size == offset)
      free();
    else 
      _move_bwd(0, offset);

    cell_count += count;
  }
  
  bool write_and_free(const DB::Cell::Key& key_start, 
                      const DB::Cell::Key& key_finish,
                      DynamicBuffer& cells, uint32_t threshold) {
    if(!m_size)
      return false;
    uint32_t count = 0;
    int32_t offset_applied = -1;
    
    cells.ensure(m_size_bytes < threshold? m_size_bytes: threshold);
    
    uint32_t offset = _narrow(key_start);
    for(Cell* cell; offset < m_size; ++offset) {
      cell = *(m_cells + offset);

      if(!key_start.empty() && 
          key_start.compare(cell->key, 0) == Condition::LT) 
        continue;
      if(!key_finish.empty() && 
          key_finish.compare(cell->key, 0) == Condition::GT)
        break;

      ++count;
      if(offset_applied == -1)
        offset_applied = offset;

      if(cell->has_expired(m_ttl))
        continue;

      cell->write(cells);
      if(threshold < cells.fill())
        break;
    }
    
    if(count) {
      if(m_size == count) {
        free();
        return false;
      }
      _move_bwd(offset_applied, count);
      return true;
    }
    return false;
  }
  
  void split(uint32_t from, Mutable& cells, 
             Interval& intval_1st, Interval& intval_2nd, 
             bool loaded) {
    Cell* cell;
    uint32_t rest = 0;
    Cell* from_cell = *(m_cells + from);

    //intval_1st.aligned_min.free();
    //intval_1st.aligned_max.free();

    for(uint32_t offset = 0; offset < m_size; ++offset) {
      cell = *(m_cells + offset);
      
      if(!rest) {
        if(cell->key.compare(from_cell->key, 0) == Condition::GT) {
          //cell->key.align(intval_1st.aligned_min, intval_1st.aligned_max);
          continue;
        }
        rest = offset;
        intval_2nd.expand_begin(*cell);
      }

      *(m_cells + offset) = 0;
      if(!loaded || cell->has_expired(m_ttl)){
        _remove(cell);
        continue;
      }
      
      cells.push_back_nocpy(cell);
      //cell->key.align(intval_2nd.aligned_min, intval_2nd.aligned_max);
    }
    m_size = rest;

    intval_2nd.set_key_end(intval_1st.key_end);      
    intval_1st.key_end.free();
    expand_end(intval_1st);
  }

  void add(const DynamicBuffer& cells) {
    Cell cell;

    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    while(remain) {
      cell.read(&ptr, &remain);
      add(cell);
    }
  }

  void add(const DynamicBuffer& cells, const DB::Cell::Key& from_key) {
    Cell cell;

    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    while(remain) {
      cell.read(&ptr, &remain);
      if(from_key.compare(cell.key) == Condition::GT)
        add(cell);
    }
  }

  void expand(Interval& interval) const {
    expand_begin(interval);
    if(m_size > 1)
      expand_end(interval);
  }

  void expand_begin(Interval& interval) const {
    interval.expand_begin(**(m_cells));
  }

  void expand_end(Interval& interval) const {
    interval.expand_end(**(m_cells + m_size-1));
  }
  

  const std::string to_string(bool with_cells=false) const {
    std::string s("CellsMutable(cap=");
    s.append(std::to_string(m_cap));
    s.append(" size=");
    s.append(std::to_string(m_size));
    s.append(" ptr=");
    s.append(std::to_string((size_t)this));
    s.append(" cells-ptr=");
    s.append(std::to_string((size_t)m_cells));
    s.append(" bytes=");
    s.append(std::to_string(m_size_bytes));
    s.append(" type=");
    s.append(Types::to_string(type));
    s.append(" max_revs=");
    s.append(std::to_string(m_max_revs));
    s.append(" ttl=");
    s.append(std::to_string(m_ttl));
    if(with_cells) {
      s.append(" cells=[\n");
      for(uint32_t offset=0; offset < m_size; ++offset) {
        s.append((*(m_cells + offset))->to_string(type));
        s.append("\n");
      }
      s.append("]");
    }
    s.append(")");
    return s;
  }
  
  private:


  void _allocate() {
    if(!m_cells) {      
      m_cells = new Cell*[m_cap];
      //m_cells = (Cell**)std::malloc(m_cap*_cell_sz);

    } else {
      //void* cells_new = std::realloc(m_cells, (m_cap += m_size)*_cell_sz));
      //if(cells_new) 
      //  m_cells = (Cell**)cells_new;
      
      Cell** cells_old = m_cells;
      m_cells = new Cell*[m_cap += m_size];     
      memcpy(m_cells, cells_old, m_size*_cell_sz);
      //for(uint32_t n=m_size;--n;) 
      //  *(m_cells+n) = *(cells_old+n);
      delete [] cells_old;
    }

    memset(m_cells+m_size, 0, (m_cap-m_size)*_cell_sz);
    //for(uint32_t n=m_cap-m_size; --n;) 
    //  *(m_cells+m_size+n) = nullptr;
  }

  void _move_fwd(uint32_t offset, int32_t by) {
    ensure(by);
    Cell** ptr = m_cells+offset;
    memmove(ptr+by, ptr, (m_size-offset)*_cell_sz);
    //Cell** end = m_cells+m_size+by-1;
    //for(uint32_t n = m_size-offset;--n;)
      //*end-- = *(end-by); 
  }

  void _move_bwd(uint32_t offset, int32_t by) {
    Cell** offset_ptr = m_cells+offset;
    Cell** offset_end_ptr = offset_ptr+by;
    Cell** ptr = offset_ptr;
    do {
      _remove(*ptr);
    } while(++ptr < offset_end_ptr);
   
    if((m_size -= by) > offset)
      memmove(offset_ptr, offset_end_ptr, (m_size-offset)*_cell_sz);
    memset(m_cells+m_size, 0, by*_cell_sz);
  }

  void _remove(Cell* cell) {
    m_size_bytes -= cell->encoded_length();
    delete cell;
    //(*ptr)->~Cell();
    //std::free(*ptr);
  }

  uint32_t _narrow(const DB::Cell::Key& key) const {
    uint32_t offset = 0;

    if(m_size < narrow_sz || key.empty())
      return offset;

    uint32_t sz = m_size/2;
    offset = sz; 

    for(;;) {
      if((*(m_cells + offset))->key.compare(key, 0) == Condition::GT) {
        if(sz < narrow_sz)
          break;
        offset += sz /= 2; 
        continue;
      }
      if((sz /= 2) == 0)
        ++sz;  

      if(offset < sz) {
        offset = 0;
        break;
      }
      offset -= sz;
    }
    return offset;
  }

  void _remove_overhead(uint32_t offset, const DB::Cell::Key& key, 
                        uint32_t revs) {
    
    for(Cell* cell; cell=get_next(offset); ++offset) {
      if(cell->key.compare(key, 0) != Condition::EQ)
        return;

      if(cell->flag != INSERT)
        continue;

      if(++revs > m_max_revs)
        _move_bwd(offset--, 1);
    }
  }

  Cell**            m_cells;
  uint32_t          m_cap;
  uint32_t          m_size;
  uint32_t          m_size_bytes;
  uint32_t          m_max_revs;
  uint64_t          m_ttl;

};


}}}
#endif