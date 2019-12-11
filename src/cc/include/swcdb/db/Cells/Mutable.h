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
  typedef std::function<bool(const Cell&)>  Selector_t;
  
  uint32_t          size;
  uint32_t          size_bytes;
  
  inline static Ptr make(const uint32_t cap=1, 
                         const uint32_t max_revs=1, 
                         const uint64_t ttl=0, 
                         const Types::Column type=Types::Column::PLAIN) {
    return std::make_shared<Mutable>(cap, max_revs, ttl, type);
  }

  explicit Mutable(const uint32_t cap=1, 
                   const uint32_t max_revs=1, 
                   const uint64_t ttl=0, 
                   const Types::Column type=Types::Column::PLAIN)
                  : m_cells(0), m_cap(cap==0?1:cap), size(0), size_bytes(0), 
                    m_max_revs(max_revs), m_ttl(ttl), m_type(type){
    _allocate();
  }

  ~Mutable() {
    free();
  }

  void ensure(uint32_t sz) {
    if(m_cap < size + sz){
      m_cap += sz;
      _allocate();
    }
  }

  void free() {
    if(m_cells) {
      if(size) do { 
        size--;
        delete *(m_cells+size);
        //(*(m_cells+size))->~Cell();
        //std::free(*(m_cells+size));
      } while(size);

      delete [] m_cells;
      //std::free(m_cells);

      m_cap = 0;
      m_cells = 0;
      size_bytes = 0;
    }
  }

  void get(int32_t idx, Cell& cell) const {
    cell.copy(**(m_cells+(idx < 0? size+idx: idx)));
  }

  void get(int32_t idx, DB::Cell::Key& key) const {
    key.copy((*(m_cells+(idx < 0? size+idx: idx)))->key);
  }

  void get(int32_t idx, DB::Cell::Key& key, int64_t& ts) const {
    Cell* cell = *(m_cells+(idx < 0? size+idx: idx));
    key.copy(cell->key);
    ts = cell->timestamp;
  }
   
  bool get(const DB::Cell::Key& key, Condition::Comp comp, Cell& cell) const {
    Cell* ptr;
    Condition::Comp chk;

    for(uint32_t offset = _narrow(key, 0); offset < size; offset++) {
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

    for(uint32_t offset = 0; offset < size; offset++){
      ptr = *(m_cells + offset);
      if(key.is_matching(ptr->key)) {
        cell.copy(*ptr);
        return true;
      }
    }
    return false;
  }

  void push_back(const Cell& cell) {
    ensure(1);
    *(m_cells + size) = new Cell(cell);
    //new(*(m_cells + size) = (Cell*)std::malloc(sizeof(Cell))) Cell(cell);
    size++;
    size_bytes += cell.encoded_length();
  }

  void push_back_nocpy(Cell* cell) {
    ensure(1);
    *(m_cells + size) = cell;
    //new(*(m_cells + size) = (Cell*)std::malloc(sizeof(Cell))) Cell(cell);
    size++;
    size_bytes += cell->encoded_length();
  }
  
  void insert(uint32_t offset, const Cell& cell) {
    _move_fwd(offset, 1);
    *(m_cells + offset) = new Cell(cell);
    //new(*(m_cells + offset) = (Cell*)std::malloc(sizeof(Cell))) Cell(cell);
    size++;
    size_bytes += cell.encoded_length();
  }

  void add(const Cell& e_cell) {
    if(e_cell.has_expired(m_ttl))
      return;

    Cell* cell;
    Cell cell_cpy;
    Condition::Comp cond;
    bool removing = false;
    bool updating = false;
    uint32_t revs = 0;
    int64_t revision_exist;
    int64_t revision_new;
    
    for(uint32_t offset = _narrow(e_cell.key, e_cell.on_fraction);
        offset < size; offset++) {
      cell = *(m_cells + offset);

      if(cell->has_expired(m_ttl)) {
         _move_bwd(offset--, 1);
        continue;
      }

      cond = cell->key.compare(
        e_cell.key, 
        (removing || updating) ?  e_cell.on_fraction : cell->on_fraction
      );
      
      if(cond == Condition::GT)
        continue;

      if(cond == Condition::LT){
        if(removing || updating)
          return;
        insert(offset, e_cell);
        return;
      }

      // (cond == Condition::EQ)

      revision_exist = cell->get_revision();
      revision_new   = e_cell.get_revision();
    
      if(removing) {
        if(e_cell.is_removing(revision_exist)) {
          _move_bwd(offset--, 1);
        }
        continue;
          
      } else if(cell->removal()) {
        if(cell->is_removing(revision_new)) {
          return;
        }
        continue;
      }

      if(e_cell.removal()) {
        removing = true;
        if(e_cell.is_removing(revision_exist))
          cell->copy(e_cell);
        else 
          insert(offset, e_cell);
        continue;
      }
      
      switch(m_type) {
        case Types::Column::COUNTER_I64: {
            
          if(!updating && e_cell.on_fraction) {
            updating = true;
            insert(offset++, e_cell);
            continue;
          }
          if(cell->on_fraction) {
            if(!(cell_cpy.control & HAVE_REVISION) 
              || cell_cpy.get_revision() < revision_exist)
              cell_cpy.copy(*cell);
            continue;
          } // on_fraction agrregated evaluation

          OP op1;
          int64_t v1 = cell->get_value(&op1);
          OP op2;
          int64_t v2 = e_cell.get_value(&op2);

          if(op1 == OP::EQUAL) {
            if(revision_exist > revision_new) 
              return;
            continue;
          } 
          if(op2 == OP::EQUAL) {
            insert(offset++, e_cell);
            v1 = 0;
          }

          if(revision_exist == revision_new) {
            if(updating)
              continue;
            return;
          }

          if((v2 += (op1 == OP::MINUS? -v1 : v1)) > 0) {
            op2 = OP::PLUS;
          } else {
            op2 = OP::MINUS;
            v2 *= -1;
          }
          cell->set_value(op2, v2);

          if(revision_exist < revision_new)
            revision_exist = revision_new;
          if(cell->timestamp < e_cell.timestamp)
            cell->timestamp = e_cell.timestamp;
            
          if(updating)
            continue;
          return;
        }

        default: { // PLAIN
          if(revision_exist == revision_new) {
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
      }
    }
    
    if(removing || updating)
      return;
    push_back(e_cell);
  }

  void pop(int32_t idx, Cell& cell) {
    uint32_t offset = idx < 0? size+idx: idx;
    cell.copy(**(m_cells+offset));
    
    _move_bwd(offset, 1);
  }

  void scan(const DB::Cells::Interval& interval, 
                  DB::Cells::Mutable& cells) const {
    Cell* cell;

    for(uint32_t offset = _narrow(interval.key_begin, 0);
        offset < size; offset++){
      cell = *(m_cells + offset);

      if(cell->has_expired(m_ttl) || !interval.is_in_begin(cell->key))
        continue;
      if(!interval.is_in_end(cell->key))
        break; 

      cells.add(*cell);
    }
  }

  void scan(const Specs::Interval& specs, Mutable::Ptr cells, 
            size_t& cell_offset, const std::function<bool()>& reached_limits, 
            size_t& skips, const Selector_t& selector=0) const {
    Cell* cell;

    uint32_t offset = 0; //(narrower over specs.key_start)
    
    for(; offset < size; offset++){
      cell = *(m_cells + offset);

      if(cell->flag == NONE) {
        // temp checkup
        std::cerr << "FLAG::NONE, offset=" << offset 
                  << " size=" << size << " " 
                  << cell->to_string() << "\n";
        exit(1);
      }
      
      if(cell->has_expired(m_ttl) || cell->on_fraction 
        || (cell->flag != INSERT && !specs.flags.return_deletes)) {
        skips++;
        continue;
      }

      if((!selector && specs.is_matching(*cell, m_type))
          || (selector && selector(*cell))) {
        if(cell_offset){
          cell_offset--;
          skips++;  
          continue;
        }
        cells->add(*cell);
        if(reached_limits())
          break;
      } else 
        skips++;
    }
  }

  void scan(const Specs::Interval& specs, DynamicBuffer& result, 
            size_t& count, size_t& skips) const {
    Cell* cell;
    uint32_t offset = 0; //specs.flags.offset; //(narrower over specs.key_start)
    uint cell_offset = specs.flags.offset;
    for(; offset < size; offset++){
      cell = *(m_cells + offset);

      if(cell->has_expired(m_ttl) || cell->on_fraction 
        || (cell->flag != INSERT && !specs.flags.return_deletes))
        continue;

      if(specs.is_matching(*cell, m_type)) {
        if(cell_offset){
          cell_offset--;
          skips++;  
          continue;
        }
        
        cell->write(result);
        if(++count == specs.flags.limit) 
          // specs.flags.limit_by && specs.flags.max_versions
          break;
      } else 
        skips++;
    }
  }

  void write(DynamicBuffer& cells) {
    Cell* cell;
    cells.ensure(size_bytes);

    for(uint32_t offset = 0; offset < size; offset++) {
      cell = *(m_cells + offset);
      
      if(cell->has_expired(m_ttl))
        continue;
        
      cell->write(cells);
    }
  }

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold) {
    Cell* cell;
    Cell* first = nullptr;
    Cell* last = nullptr;

    cells.ensure(size_bytes < threshold? size_bytes: threshold);
    uint32_t offset = 0;
    for(;offset < size && (!threshold || threshold > cells.fill()); offset++) {
      cell = *(m_cells + offset);
      
      if(cell->has_expired(m_ttl))
        continue;
      
      cell->write(cells);
      cell_count++;

      if(!first)
        first = cell;
      else 
        last = cell;
      intval.expand(cell->timestamp);
      //intval.expand(*cell);
    }
    if(first) {
      intval.expand(*first);
      if(last)
        intval.expand(*last);
    }
    
    if(offset) {
      if(size == offset)
        free();
      else 
        _move_bwd(0, offset);
    }
  }
  
  bool write_and_free(const DB::Cell::Key& key_start, 
                      const DB::Cell::Key& key_finish,
                      DynamicBuffer& cells, uint32_t threshold) {
    Cell* cell;
    uint32_t count = 0;
    int32_t offset_applied = -1;
    Condition::Comp cond;
    
    cells.ensure(size_bytes < threshold? size_bytes: threshold);
    
    uint32_t offset = _narrow(key_start, 0);

    for(; offset < size; offset++) {
      cell = *(m_cells + offset);

      cond = cell->key.compare(key_start, 0);
      if(cond == Condition::GT) 
        continue;
      cond = cell->key.compare(key_finish, 0);
      if(cond == Condition::GT)
        break;

      count++;
      if(cell->has_expired(m_ttl))
        continue;

      if(offset_applied == -1)
        offset_applied = offset;

      cell->write(cells);
      if(threshold < cells.fill())
        break;
    }
    
    if(count) {
      if(size == count) {
        free();
        return false;
      }
      _move_bwd(offset_applied, offset-offset_applied);
      return true;
    }
    return false;
  }
  
  void move_from_key_offset(uint32_t from, Mutable& cells) {
    Cell* cell;
    uint32_t rest = 0;
    Cell* from_cell = *(m_cells + from);

    for(uint32_t offset = _narrow(from_cell->key, 0);
        offset < size; offset++) {
      cell = *(m_cells + offset);
      
      if(!rest) {
        if(cell->key.compare(from_cell->key, 0) == Condition::GT)
          continue;
        rest = offset;
      }

      *(m_cells + offset) = 0;
      
      if(cell->has_expired(m_ttl)){
        _remove(cell);
        continue;
      }
      
      cells.push_back_nocpy(cell);
    }
    size = rest;
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

  void add_to(Ptr cells) {
    for(uint32_t offset = 0; offset < size; offset++)
      cells->add(**(m_cells + offset));
  }

  void expand(DB::Cells::Interval& interval) {
    interval.expand(**(m_cells)); // !on_fraction
    if(size)
      interval.expand(**(m_cells + size-1));
  }

  const std::string to_string(bool with_cells=false) const {
    std::string s("CellsMutable(cap=");
    s.append(std::to_string(m_cap));
    s.append(" size=");
    s.append(std::to_string(size));
    s.append(" bytes=");
    s.append(std::to_string(size_bytes));
    s.append(" type=");
    s.append(Types::to_string(m_type));
    s.append(" max_revs=");
    s.append(std::to_string(m_max_revs));
    s.append(" ttl=");
    s.append(std::to_string(m_ttl));
    if(with_cells) {
      s.append(" cells=[\n");
      for(uint32_t offset=0; offset < size; offset++) {
        s.append((*(m_cells + offset))->to_string(m_type));
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
      //void* cells_new = std::realloc(m_cells, (m_cap += size)*_cell_sz));
      //if(cells_new) 
      //  m_cells = (Cell**)cells_new;
      
      Cell** cells_old = m_cells;
      m_cells = new Cell*[m_cap += size];     

      memcpy(m_cells, cells_old, size*_cell_sz);
      //for(uint32_t n=size;n--;) 
      //  *(m_cells+n) = *(cells_old+n);
     
      delete [] cells_old;
    }

    memset(m_cells+size, 0, (m_cap-size)*_cell_sz);
    //for(uint32_t n=m_cap-size; n--;) 
    //  *(m_cells+size+n) = nullptr;
  }

  void _move_fwd(uint32_t offset, int32_t by) {
    ensure(by);
    Cell** ptr = m_cells+offset;
    memmove(ptr+by, ptr, (size-offset)*_cell_sz);
    //Cell** end = m_cells+size+by-1;
    //for(uint32_t n = size-offset;n--;)
      //*end-- = *(end-by); 
  }

  void _move_bwd(uint32_t offset, int32_t by) {
    Cell** offset_ptr = m_cells+offset;
    Cell** offset_end_ptr = offset_ptr+by;
    Cell** ptr = offset_ptr;
    do {
      _remove(*ptr);
    } while(++ptr < offset_end_ptr);
   
    size -= by;
    if(offset < size)
      memmove(offset_ptr, offset_end_ptr, (size-offset)*_cell_sz);
    memset(m_cells+size, 0, by*_cell_sz);
  }

  void _remove(Cell* cell) {
    size_bytes -= cell->encoded_length();
    delete cell;
    //(*ptr)->~Cell();
    //std::free(*ptr);
  }

  uint32_t _narrow(const DB::Cell::Key& key, uint32_t on_fraction=0) const {
    uint32_t offset = 0;

    if(size < narrow_sz)
      return offset;

    uint32_t narrows = 0;
    uint32_t sz = size/2;
    offset = sz; 
    Condition::Comp cond;

    for(;;) {
      cond = (*(m_cells + offset))->key.compare(key, on_fraction); 
      narrows++;

      if(cond == Condition::GT){
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
    Cell* cell;
    for(; offset < size; offset++) {

      cell = *(m_cells + offset);
      
      if(cell->flag != INSERT)
        continue;

      if(cell->has_expired(m_ttl)) {
         _move_bwd(offset--, 1);
        continue;
      }
    
      if(cell->key.compare(key, 0) != Condition::EQ)
        return;

      if(++revs > m_max_revs)
        _move_bwd(offset--, 1);
    }
  }

  Cell**            m_cells;
  uint32_t          m_cap;
  uint32_t          m_max_revs;
  uint64_t          m_ttl;
  Types::Column     m_type;

  static const uint8_t     _cell_sz = sizeof(Cell*);
};


}}}
#endif