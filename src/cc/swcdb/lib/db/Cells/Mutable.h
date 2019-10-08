/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Cells_Mutable_h
#define swcdb_lib_db_Cells_Mutable_h

#include <mutex>

#include "swcdb/lib/db/Cells/Cell.h"
#include "SpecsInterval.h"
#include "Interval.h"

namespace SWC { namespace DB { namespace Cells {


class Mutable {
  static const uint32_t narrow_sz = 20;
  public:

  typedef std::shared_ptr<Mutable>  Ptr;
  
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

  void allocate_if_needed(uint32_t sz) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return _allocate_if_needed(sz);
  }

  void get(int32_t idx, Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);
    cell.copy(**(m_cells+(idx < 0? m_size+idx: idx)));
  }

  Cell* front() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return *m_cells;
  }
  
  Cell* back() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return *(m_cells+m_size-1);
  }

  void add(Cell& e_cell) { 
    //std::cout << "add, " << e_cell.to_string() << "\n";
    
    if(e_cell.has_expired(m_ttl))
      return;

    Cell* cell;
    Condition::Comp cond;
    bool removing = false;
    bool updating = false;
    uint32_t revs = 0;
    uint32_t offset = 0; 
    
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t narrows = 0;

    if(m_size > narrow_sz) {
      uint32_t sz = m_size/2;
      offset = sz; 

      for(;;) {
        cond = (*(m_cells + offset))->key.compare(e_cell.key, e_cell.on_fraction); 
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
    }
    
    //if(m_size < offset) {
    //  std::cout << " size=" << m_size << " offset=" << offset << " narrows=" << narrows << "\n";
    //  exit(1);
    //}
    
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
      
      if(cond == Condition::EQ) {

        if(removing) {
          if(e_cell.is_removing(cell->revision)){
            _move_bwd(offset--, 1);
          }
          continue;
          
        } else if(cell->removal()) {
          if(cell->is_removing(e_cell.revision))
            return;
          continue;
        }

        if(e_cell.removal()) {
          removing = true;
          _insert(offset++, e_cell);
          continue;
        
        } else {

          if(m_type == Types::Column::COUNTER_I64) {
            
            if(!updating && e_cell.on_fraction) {
              updating = true;
              _insert(offset++, e_cell);
              continue;
            }

            OP op1;
            int64_t v1 = cell->get_value(&op1);
            OP op2;
            int64_t v2 = e_cell.get_value(&op2);

            if(op1 == OP::EQUAL && op2 == OP::EQUAL 
              && cell->revision > e_cell.revision)
              return;

            v2 = op2 == OP::EQUAL? v2 :(v1+(op2 == OP::MINUS?-v2:+v2));
            if(cell->on_fraction) {
              //std::cout << "cell->on_fraction\n";
              e_cell.set_value(OP::EQUAL, v2);
              if(e_cell.revision < cell->revision)
                e_cell.revision = cell->revision;
              if(e_cell.timestamp < cell->timestamp)
                e_cell.timestamp = cell->timestamp;
              continue;
            }

            cell->set_value(OP::EQUAL, v2);
            if(cell->revision < e_cell.revision)
              cell->revision = e_cell.revision;
            if(cell->timestamp < e_cell.timestamp)
              cell->timestamp = e_cell.timestamp;
            
            //std::cout << cell->key.to_string() << " == " << cell->get_value(&op2) << "\n";
            if(updating)
              continue;
            return;

          } else {

            if(cell->revision == e_cell.revision) {
              cell->copy(e_cell);
              return;
            }

            if(m_max_revs == revs)
              return;

            if(e_cell.control & TS_DESC 
              ? e_cell.timestamp < cell->timestamp
              : e_cell.timestamp > cell->timestamp) {
              if(m_max_revs == ++revs) {
                //std::cout << "revs=" << revs << "\n";
                //std::cout << "old " << cell->to_string() << "\n";
                //std::cout << "new " << e_cell.to_string() << "\n";
                return;
              }
              continue;
            }
          }

        } 

      } else if(removing || updating)
        return;

      if(cond == Condition::GT)
        continue;

      _insert(offset, e_cell);
      return;
    }

    _push_back(e_cell);
  }
  
  void push_back(Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);
    _push_back(cell);
  }
  
  void insert(uint32_t offset, Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);
    _insert(offset, cell);
  }

  uint32_t size(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_size;
  }

  uint32_t size_bytes(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_size_bytes;
  }

  void free(){
    std::lock_guard<std::mutex> lock(m_mutex);
    _free();
  }

  bool scan(const Specs::Interval& specs, Mutable::Ptr cells){
    //
    return false;
  }

  bool scan(const Specs::Interval& specs, DynamicBufferPtr result){
    //
    return false;
  }
  
  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval::Ptr& intval, uint32_t threshold) {
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
      intval->expand(*cell);
    }
    
    if(offset > 0) {
      if(m_size == offset)
        _free();
      else 
        _move_bwd(0, offset);
    }
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return _to_string();
  }
  
  private:

  inline const std::string _to_string() {
    std::string s("CellsMutable(cap=");
    s.append(std::to_string(m_cap));
    s.append(" size=");
    s.append(std::to_string(m_size));
    s.append(" bytes=");
    s.append(std::to_string(m_size_bytes));
    s.append(" max_revs=");
    s.append(std::to_string(m_max_revs));
    s.append(" ttl=");
    s.append(std::to_string(m_ttl));
    s.append(")");
    return s;
  }

  inline void _allocate_if_needed(uint32_t sz) {
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
      //m_cells = (Cell**)std::malloc(m_cap*sizeof(Cell*));

    } else {
      //void* cells_new = std::realloc(m_cells, (m_cap += m_size)*sizeof(Cell*));
      //if(cells_new) 
      //  m_cells = (Cell**)cells_new;
      
      Cell** cells_old = m_cells;
      m_cells = new Cell*[m_cap += m_size];

      memcpy(m_cells, cells_old, m_size*sizeof(Cell*));
      //for(uint32_t n=m_size;n--;) 
      //  *(m_cells+n) = *(cells_old+n);
     
      delete [] cells_old;
    }

    memset(m_cells+m_size, 0, (m_cap-m_size)*sizeof(Cell*));
    //for(uint32_t n=m_cap-m_size; n--;) 
    //  *(m_cells+m_size+n) = nullptr;
  }

  inline void _move_fwd(uint32_t offset, int32_t by) {
    _allocate_if_needed(by);
    memmove(m_cells+offset+by, m_cells+offset, (m_size-offset)*sizeof(Cell*));
    //Cell** end = m_cells+m_size+by-1;
    //for(uint32_t n = m_size-offset;n--;)
      //*end-- = *(end-by); 
  }

  inline void _move_bwd(uint32_t offset, int32_t by) {
    for(uint32_t n=0;n<by; n++){
      if(*(m_cells+offset+n)){
        m_size_bytes -= (*(m_cells+offset+n))->encoded_length();
        delete *(m_cells+offset+n);
        //(*(m_cells+offset+n))->~Cell();
        //std::free(*(m_cells+offset+n));
      }
    }

    memmove(m_cells+offset, m_cells+offset+by, (m_size-offset-by)*sizeof(Cell*));
    m_size-=by;
    memset(m_cells+m_size, 0, (m_cap-m_size)*sizeof(Cell*));
  }
  
  inline void _insert(uint32_t offset, Cell& cell) {
    _move_fwd(offset, 1);
    *(m_cells + offset) = new Cell(cell);
    //new(*(m_cells + offset) = (Cell*)std::malloc(sizeof(Cell))) Cell(cell);
    m_size++;
    m_size_bytes += cell.encoded_length();
  }

  inline void _push_back(Cell& cell) {
    _allocate_if_needed(1);
    *(m_cells + m_size) = new Cell(cell);
    //new(*(m_cells + m_size) = (Cell*)std::malloc(sizeof(Cell))) Cell(cell);
    m_size++;
    m_size_bytes += cell.encoded_length();
  }

  inline void _free() {
    if(m_cells != 0) {
      do { 
        m_size--;
        delete *(m_cells+m_size);
        //(*(m_cells+m_size))->~Cell();
        //std::free(*(m_cells+m_size));
      } while(m_size);

      delete [] m_cells;
      //std::free(m_cells);

      m_cap = 0;
      m_cells = 0;
      m_size_bytes = 0;
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
};


}}}
#endif