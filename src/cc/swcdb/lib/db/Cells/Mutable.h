/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Cells_Mutable_h
#define swcdb_lib_db_Cells_Mutable_h

#include <mutex>
#include <functional>

#include "Cell.h"
#include "SpecsInterval.h"
#include "Interval.h"

namespace SWC { namespace DB { namespace Cells {


class Mutable {

  static const uint32_t narrow_sz = 20;

  public:

  typedef std::shared_ptr<Mutable>          Ptr;
  typedef std::function<bool(const Cell&)>  Selector_t;
  
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
                  : m_cells(0), m_cap(cap==0?1:cap), m_size(0), m_size_bytes(0), 
                    m_max_revs(max_revs), m_ttl(ttl), m_type(type){
    _allocate();
  }

  virtual ~Mutable() {
    //std::cout << " ~Mutable " << (size_t)this << " 1\n";
    _free();
    //std::cout << " ~Mutable " << (size_t)this << " 2\n";
  }

  void ensure(uint32_t sz) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return _ensure(sz);
  }

  void free(){
    std::lock_guard<std::mutex> lock(m_mutex);
    _free();
  }

  void get(int32_t idx, Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);
    cell.copy(**(m_cells+(idx < 0? m_size+idx: idx)));
  }
   
  bool get(const DB::Cell::Key& key, Condition::Comp comp, Cell& cell) {
    Cell* ptr;
    Condition::Comp chk;

    std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t offset = _narrow(key, 0);

    for(;offset < m_size; offset++){
      ptr = *(m_cells + offset);
      if((chk = key.compare(ptr->key, 0)) == Condition::GT 
        || (comp == Condition::GE && chk == Condition::EQ)){
        cell.copy(*ptr);
        return true;
      }
    }
    return false;
  }

  bool get(const Specs::Key& key, Cell& cell) {
    Cell* ptr;
    std::lock_guard<std::mutex> lock(m_mutex);

    for(uint32_t offset = 0;offset < m_size; offset++){
      ptr = *(m_cells + offset);
      if(key.is_matching(ptr->key)) {
        cell.copy(*ptr);
        return true;
      }
    }
    return false;
  }

  uint32_t size(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_size;
  }

  uint32_t size_bytes(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_size_bytes;
  }

  void push_back(const Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);
    _push_back(cell);
  }
  
  void insert(uint32_t offset, const Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);
    _insert(offset, cell);
  }

  void add(const Cell& cell) { 
    //std::cout << "add, " << e_cell.to_string() << "\n";
    
    if(cell.has_expired(m_ttl))
      return;

    std::lock_guard<std::mutex> lock(m_mutex);
    _add(cell);
  }

  void scan(const DB::Cells::Interval& interval, 
                  DB::Cells::Mutable& cells){
    Cell* cell;
    std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t offset = _narrow(interval.key_begin, 0);

    for(;offset < m_size; offset++){
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
            size_t& skips, const Selector_t selector=0){
    Cell* cell;
    uint32_t offset = 0; //(narrower over specs.key_start)
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for(;offset < m_size; offset++){
      cell = *(m_cells + offset);

      //std::cout << " " << cell->to_string() << "\n";
      if(cell->has_expired(m_ttl) || cell->on_fraction 
        || (cell->flag != INSERT && !specs.flags.return_deletes))
        continue;

      if((!selector && specs.is_matching(*cell, m_type))
          || (selector && selector(*cell))) {
        //std::cout << " " << cell->to_string() << "\n";
        //std::cout << " " << specs.to_string() << "\n";
        if(cell_offset != 0){
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
            size_t& count, size_t& skips){
    Cell* cell;
    uint32_t offset = 0; //specs.flags.offset; //(narrower over specs.key_start)
    uint cell_offset = specs.flags.offset;
    std::lock_guard<std::mutex> lock(m_mutex);

    for(;offset < m_size; offset++){
      cell = *(m_cells + offset);
      if(cell->has_expired(m_ttl) || cell->on_fraction 
        || (cell->flag != INSERT && !specs.flags.return_deletes))
        continue;

      if(specs.is_matching(*cell, m_type)) {
        if(cell_offset != 0){
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
    cells.ensure(m_size_bytes);
    uint32_t offset = 0;

    std::lock_guard<std::mutex> lock(m_mutex);
    while(offset < m_size) {
      cell = *(m_cells + offset++);
      if(cell->has_expired(m_ttl))
        continue;
        
      cell->write(cells);
    }
  }

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold) {
    Cell* cell;
    uint32_t offset = 0;

    std::lock_guard<std::mutex> lock(m_mutex);
  
    cells.ensure(m_size_bytes < threshold? m_size_bytes: threshold);

    while(offset < m_size && threshold > cells.fill()) {
      cell = *(m_cells + offset++);
      
      if(cell->has_expired(m_ttl))
        continue;
        
      cell->write(cells);
      cell_count++;
      intval.expand(*cell);
    }
    
    if(offset > 0) {
      if(m_size == offset)
        _free();
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
    
    std::lock_guard<std::mutex> lock(m_mutex);
    cells.ensure(m_size_bytes < threshold? m_size_bytes: threshold);

    uint32_t offset = _narrow(key_start, 0);
    
    for(;offset < m_size;offset++) {
      cell = *(m_cells + offset);

      if(cell->has_expired(m_ttl))
        continue;

      cond = cell->key.compare(key_start, 0);
      if(cond == Condition::GT) 
        continue;
      cond = cell->key.compare(key_finish, 0);
      if(cond == Condition::GT)
        break;

      count++;
      if(offset_applied == -1)
        offset_applied = offset;

      cell->write(cells);
      if(threshold < cells.fill())
        break;
    }
    
    if(count > 0) {
      if(m_size == count) {
        _free();
        return false;
      }
      _move_bwd(offset_applied, offset);
      return true;
    }
    return false;
  }
  
  void add(const DynamicBuffer& cells) {
    Cell cell;
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint8_t* ptr = cells.base;
    size_t remain = cells.fill();
    while(remain) {
      cell.read(&ptr, &remain);
      _add(cell);
    }
  }

  void add_to(Ptr cells) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for(uint32_t offset = 0;offset < m_size; offset++)
      cells->add(**(m_cells + offset));
  }
  

  const std::string to_string(bool with_cells=false)  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return _to_string(with_cells);
  }
  
  private:

  inline const std::string _to_string(bool with_cells=false) const {
    std::string s("CellsMutable(cap=");
    s.append(std::to_string(m_cap));
    s.append(" size=");
    s.append(std::to_string(m_size));
    s.append(" bytes=");
    s.append(std::to_string(m_size_bytes));
    s.append(" type=");
    s.append(Types::to_string(m_type));
    s.append(" max_revs=");
    s.append(std::to_string(m_max_revs));
    s.append(" ttl=");
    s.append(std::to_string(m_ttl));
    if(with_cells) {
      s.append(" cells=[");
      for(uint32_t offset=0;offset < m_size; offset++) {
        s.append((*(m_cells + offset))->to_string(m_type));
        s.append("\n");
      }
      s.append("]");
    }
    s.append(")");
    return s;
  }

  inline void _ensure(uint32_t sz) {
    if(m_cap < m_size + sz){
      m_cap += sz;
      //auto ts = Time::now_ns();
      _allocate();
      //std::cout << " _allocate took=" << Time::now_ns()-ts << " " << _to_string() << "\n";
    }
  }

  void _allocate() {
    if(m_cells == 0) {      
      m_cells = new Cell*[m_cap];
      //m_cells = (Cell**)std::malloc(m_cap*_cell_sz);

    } else {
      //void* cells_new = std::realloc(m_cells, (m_cap += m_size)*_cell_sz));
      //if(cells_new) 
      //  m_cells = (Cell**)cells_new;
      
      Cell** cells_old = m_cells;
      m_cells = new Cell*[m_cap += m_size];

      memcpy(m_cells, cells_old, m_size*_cell_sz);
      //for(uint32_t n=m_size;n--;) 
      //  *(m_cells+n) = *(cells_old+n);
     
      delete [] cells_old;
    }

    memset(m_cells+m_size, 0, (m_cap-m_size)*_cell_sz);
    //for(uint32_t n=m_cap-m_size; n--;) 
    //  *(m_cells+m_size+n) = nullptr;
  }

  void _move_fwd(uint32_t offset, int32_t by) {
    _ensure(by);
    Cell** ptr = m_cells+offset;
    memmove(ptr+by, ptr, (m_size-offset)*_cell_sz);
    //Cell** end = m_cells+m_size+by-1;
    //for(uint32_t n = m_size-offset;n--;)
      //*end-- = *(end-by); 
  }

  void _move_bwd(uint32_t offset, int32_t by) {
    Cell** offset_ptr = m_cells+offset;
    Cell** offset_end_ptr = offset_ptr+by;
    Cell** ptr = offset_ptr;
    do {
      m_size_bytes -= (*ptr)->encoded_length();
      delete *ptr;
      //(*ptr)->~Cell();
      //std::free(*ptr);
    } while(++ptr < offset_end_ptr);

    m_size -= by;
    memmove(offset_ptr, offset_end_ptr, (m_size-offset)*_cell_sz);
    memset(m_cells+m_size, 0, by*_cell_sz);
  }
  
  void _insert(uint32_t offset, const Cell& cell) {
    _move_fwd(offset, 1);
    *(m_cells + offset) = new Cell(cell);
    //new(*(m_cells + offset) = (Cell*)std::malloc(sizeof(Cell))) Cell(cell);
    m_size++;
    m_size_bytes += cell.encoded_length();
  }

  void _push_back(const Cell& cell) {
    _ensure(1);
    *(m_cells + m_size) = new Cell(cell);
    //new(*(m_cells + m_size) = (Cell*)std::malloc(sizeof(Cell))) Cell(cell);
    m_size++;
    m_size_bytes += cell.encoded_length();
  }

  void _add(const Cell& e_cell) {

    Cell* cell;
    Cell cell_cpy;
    Condition::Comp cond;
    bool removing = false;
    bool updating = false;
    uint32_t revs = 0;
    
    
    uint32_t offset = _narrow(e_cell.key, e_cell.on_fraction);
    //std::cout << "NARROWED add(offset)=" << offset << "\n";

    for(;offset < m_size; offset++) {

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
        _insert(offset, e_cell);
        return;
      }

      // (cond == Condition::EQ)

      if(removing) {
        if(e_cell.is_removing(cell->revision)) {
          //std::cout << " e_cell is_removing: " << cell->to_string() << "\n";
          _move_bwd(offset--, 1);
        }
        continue;
          
      } else if(cell->removal()) {
        if(cell->is_removing(e_cell.revision)) {
          //std::cout << " cell is_removing: " << e_cell.to_string() << "\n";
          return;
        }
        continue;
      }

      //std::cout << "new " << e_cell.to_string() << "\n"; 
      //std::cout << "old " << cell->to_string() << "\n";  
      //std::cout << "cond " << Condition::to_string(cond) << "\n";  
      if(e_cell.removal()) {
        removing = true;
        if(e_cell.is_removing(cell->revision))
          cell->copy(e_cell);
        else 
          _insert(offset, e_cell);
        continue;
      }
      
      switch(m_type) {
        case Types::Column::COUNTER_I64: {
            
          if(!updating && e_cell.on_fraction) {
            updating = true;
            _insert(offset++, e_cell);
            continue;
          }
          if(cell->on_fraction) {
            if(!(cell_cpy.control & HAVE_REVISION) 
              || cell_cpy.revision < cell->revision)
              cell_cpy.copy(*cell);
            continue;
          } // on_fraction agrregated evaluation

          OP op1;
          int64_t v1 = cell->get_value(&op1);
          OP op2;
          int64_t v2 = e_cell.get_value(&op2);

          if(op1 == OP::EQUAL) {
            if(cell->revision > e_cell.revision) 
              return;
            continue;
          } 
          if(op2 == OP::EQUAL) {
            _insert(offset++, e_cell);
            v1 = 0;
          }

          if(cell->revision == e_cell.revision) {
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

          if(cell->revision < e_cell.revision)
            cell->revision = e_cell.revision;
          if(cell->timestamp < e_cell.timestamp)
            cell->timestamp = e_cell.timestamp;
            
          if(updating)
            continue;
          return;
        }

        default: { // PLAIN
          if(cell->revision == e_cell.revision) {
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
            _insert(offset, e_cell);
            _remove_overhead(++offset, e_cell.key, revs);
          }
          return;
        }
      }
    }
    
    if(removing || updating)
      return;
    _push_back(e_cell);
  }

  void _free() {
    if(m_cells != 0) {
      if(m_size > 0) {
        do { 
          m_size--;
          delete *(m_cells+m_size);
          //(*(m_cells+m_size))->~Cell();
          //std::free(*(m_cells+m_size));
        } while(m_size);
      }

      delete [] m_cells;
      //std::free(m_cells);

      m_cap = 0;
      m_cells = 0;
      m_size_bytes = 0;
    }
  }


  uint32_t _narrow(const DB::Cell::Key& key, uint32_t on_fraction=0){
    uint32_t offset = 0;

    if(m_size < narrow_sz)
      return offset;

    uint32_t narrows = 0;
    uint32_t sz = m_size/2;
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
  
    //if(narrows > narrow_sz*2)
    //  std::cout << " size=" << m_size << " offset=" << offset << " narrows=" << narrows << "\n";
    //if(m_size < offset) {
    //  std::cout << " size=" << m_size << " offset=" << offset << " narrows=" << narrows << "\n";
    //  exit(1);
    //}
    return offset;
  }

  void _remove_overhead(uint32_t offset, const DB::Cell::Key& key, uint32_t revs) {
    Cell* cell;
    for(;offset < m_size; offset++) {

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

  std::mutex        m_mutex;
  Cell**            m_cells;
  uint32_t          m_cap;
  uint32_t          m_size;
  uint32_t          m_size_bytes;
  uint32_t          m_max_revs;
  uint64_t          m_ttl;
  Types::Column     m_type;

  static const uint8_t     _cell_sz = sizeof(Cell*);
};


}}}
#endif