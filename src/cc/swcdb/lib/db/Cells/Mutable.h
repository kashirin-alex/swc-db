/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Cells_Mutable_h
#define swcdb_lib_db_Cells_Mutable_h

#include <mutex>

#include "swcdb/lib/db/Cells/Cell.h"
#include "SpecsInterval.h"

namespace SWC { namespace DB { namespace Cells {


class Mutable {
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
                  : m_cells(0), m_cap(cap==0?1:cap), m_size(0), 
                    m_max_revs(max_revs), m_ttl(ttl), m_type(type) {
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

  Cell* front() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return *m_cells;
  }
  
  Cell* back() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return *(m_cells+m_size-1);
  }

  void add(Cell& e_cell){ 
  
    bool removing = false;
    bool updating = false;
    uint32_t revs = 0;
    add(e_cell, removing, updating, revs);
  }

  void add(Cell& e_cell, bool& removing, bool& updating, uint32_t& revs){ 
    //std::cout << "add, " << e_cell.to_string() << "\n";
    
    if(e_cell.has_expired(m_ttl))
      return;

    uint32_t size;
    Cell* cell;
    {
      //std::lock_guard<std::mutex> lock(m_mutex);
      size = m_size;
    }

    for(uint32_t offset = 0; offset < size; offset++) {
      //std::lock_guard<std::mutex> lock(m_mutex);

      if(size != m_size)  {
        offset = 0;
        size = m_size;
        continue;
      }

      cell = *(m_cells + offset);
      
      if(cell->has_expired(m_ttl)) {
         _move_bwd(offset--, 1);
        size = m_size;
        continue;
      }

      Condition::Comp cond = cell->key.compare(
        e_cell.key, 
        (removing || updating) ?  e_cell.on_fraction : cell->on_fraction
      );
      
      if(cond == Condition::EQ) {

        if(removing) {
          if(e_cell.is_removing(cell->revision)){
            _move_bwd(offset--, 1);
            size = m_size;
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
          size = m_size;
          continue;
        
        } else {

          if(m_type == Types::Column::COUNTER_I64) {
            
            if(!updating && e_cell.on_fraction) {
              updating = true;
              _insert(offset++, e_cell);
              size = m_size;
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

    push_back(e_cell);
  }
  
  void push_back(const Cell& cell) {
    //std::lock_guard<std::mutex> lock(m_mutex);
    _allocate_if_needed(1);
    *(m_cells + m_size) = new Cell(cell);
    m_size++;
  }
  
  void insert(uint32_t offset, const Cell& cell) {
    //std::lock_guard<std::mutex> lock(m_mutex);
    _insert(offset, cell);
  }

  uint32_t size(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_size;
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

    } else {
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
    for(uint32_t n=0;n<by; n++)
      if(*(m_cells+offset+n))
        delete *(m_cells+offset+n);

    memmove(m_cells+offset, m_cells+offset+by, (m_size-offset-by)*sizeof(Cell*));
    m_size-=by;
    memset(m_cells+m_size, 0, (m_cap-m_size)*sizeof(Cell*));
  }
  
  inline void _insert(uint32_t offset, const Cell& cell) {
    _move_fwd(offset, 1);
    *(m_cells + offset) = new Cell(cell);
    m_size++;
  }

  inline void _free() {
    //std::cout << " _free, m_cells " << (size_t)*m_cells << " " << _to_string() << "\n";
    if(m_cells != 0) {
      while(m_size) 
        delete *(m_cells+(m_size--));
      delete [] m_cells;
      m_cap = 0;
      m_cells = 0;
    }
    //std::cout << " _free 2\n";
  }

  std::mutex        m_mutex;
  Cell**            m_cells;
  uint32_t          m_cap;
  uint32_t          m_size;
  uint32_t          m_max_revs;
  uint64_t          m_ttl;
  Types::Column     m_type;
};


class Mutables {
  public:

  typedef std::shared_ptr<Mutables>  Ptr;
  
  inline static Ptr make(const uint32_t index_max,
                         const uint32_t cap=1, 
                         const uint32_t max_revs=1, 
                         const uint64_t ttl=0, 
                         const Types::Column type=Types::Column::PLAIN) {
    return std::make_shared<Mutables>(index_max, cap, max_revs, ttl, type);
  }

  explicit Mutables(const uint32_t index_max,
                    const uint32_t cap=1, 
                    const uint32_t max_revs=1, 
                    const uint64_t ttl=0, 
                    const Types::Column type=Types::Column::PLAIN)
                  : m_index_max(index_max),
                    m_cap(cap==0?1:cap), m_max_revs(max_revs), 
                    m_ttl(ttl), m_type(type) {
    m_index.push_back(new Mutable(m_index_max, m_max_revs, m_ttl, m_type));
  }

  virtual ~Mutables() { 
    std::cout << " ~Mutables 1 sz=" << m_index.size() << "\n";
    for(auto idx : m_index)
      delete idx;
    std::cout << " ~Mutables 2 \n";
  }

  void add(Cell& cell) {
    uint32_t size;
    Mutable* index;
    Cell* idx_cell_begin;
    Cell* idx_cell_end;

    bool removing = false;
    bool updating = false;
    uint32_t revs = 0;
    bool cell_set = false;
    uint32_t idx_sz;
    uint32_t n=0;

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      size = m_index.size();
    }
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if(n < size) {        
          if(n && size != m_index.size()){
            size = m_index.size();
            n = 0;
            continue;
          }
          index = m_index.at(n++);
          idx_cell_begin = index->front();
          idx_cell_end = index->back();

        } else {
          index = m_index.back();
          idx_cell_begin = nullptr;
        }
        
      }

      if(idx_cell_begin == nullptr) {
        if(!removing && !updating) {
          uint32_t sz = index->size();
          index->add(cell, removing, updating, revs);
          if(m_index_max == index->size() && sz != index->size()) 
            alter(n);
        }
        break;
      }
       
      if(idx_cell_end->key.compare(cell.key) == Condition::GT)
        continue;

      index->add(cell, removing, updating, revs);
      if(!removing && !updating) {
        uint32_t sz = index->size();
        if(m_index_max == index->size() && sz != index->size()) 
          alter(n);
        break;
      }
    }

  }

  void alter(uint32_t n) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_index.insert(
      m_index.begin()+n, new Mutable(m_index_max, m_max_revs, m_ttl, m_type));
  }

  void push_back(const Cell& cell){

  }

  const size_t size() {
    size_t sz = 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto idx : m_index)
      sz += idx->size();
    return sz;
  }

  private:

  std::mutex            m_mutex;
  std::vector<Mutable*> m_index;
  
  uint32_t              m_index_max;
  uint32_t              m_cap;
  uint32_t              m_max_revs;
  uint64_t              m_ttl;
  Types::Column         m_type;
};


}}}
#endif