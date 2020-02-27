/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Cells_Vector_h
#define swcdb_lib_db_Cells_Vector_h

#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/Interval.h"

namespace SWC { namespace DB { namespace Cells {


class Vector : private std::vector<Cell*> {
  
  using std::vector<Cell*>::pop_back;
  public:

  typedef std::shared_ptr<Vector> Ptr;

  using std::vector<Cell*>::vector;
  using std::vector<Cell*>::empty;
  using std::vector<Cell*>::reserve;
  using std::vector<Cell*>::size;
  using std::vector<Cell*>::back;
  using std::vector<Cell*>::front;
  using std::vector<Cell*>::begin;
  using std::vector<Cell*>::end;
  using std::vector<Cell*>::operator[];

  explicit Vector(const uint32_t max_revs=1, const uint32_t ttl_sec=0, 
                  const Types::Column type=Types::Column::PLAIN)
                  : bytes(0), type(type), max_revs(max_revs), 
                    ttl(ttl_sec*1000000000) {
  }

  explicit Vector(Vector& other)
                  : bytes(other.bytes), type(other.type), 
                    max_revs(other.max_revs), ttl(other.ttl) {
    if(!other.empty()) {
      assign(other.begin(), other.end());
      other.clear();
      other.bytes = 0;
    }
  }

  Vector& operator=(const Vector& other) = delete;

  virtual ~Vector() {
    free();
  }

  void free() {
    for(auto cell : *this)
      if(cell)
        delete cell;
    clear();
    bytes = 0;
  }

  void reset(const uint32_t revs=1, const uint32_t ttl_sec=0, 
             const Types::Column typ=Types::Column::PLAIN) {
    free();
    configure(revs, ttl_sec, typ);
  }

  void configure(const uint32_t revs=1, 
                 const uint32_t ttl_sec=0, 
                 const Types::Column typ=Types::Column::PLAIN) {
    type = typ;
    max_revs = revs;
    ttl = ttl_sec*1000000000;
  }

  void take(Vector& other) {
    bytes += other.bytes;
    insert(end(), other.begin(), other.end());
    
    other.clear();
    other.bytes = 0;
  }

  void add(const Cell& cell, bool no_value=false) {
    Cell* adding;
    push_back(adding = new Cell(cell, no_value));
    bytes += adding->encoded_length();
  }
  
  const size_t add(const uint8_t* ptr, size_t remain) {
    size_t count = 0;
    bytes += remain;
    while(remain) {
      push_back(new Cell(&ptr, &remain, true));
      ++count;
    }
    return count;
  }

  const size_t size_bytes() const {
    return bytes;
  }

  void pop_back(Cell*& cell) {
    cell = back();
    pop_back();
    bytes -= cell->encoded_length();
  }

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold, 
                      uint32_t max_cells) {
    if(empty())
      return;
    Cell* cell;
    Cell* first = nullptr;
    Cell* last = nullptr;
    size_t count = 0;
    cells.ensure(bytes < threshold? bytes: threshold);
    auto it = begin();
    for(; it < end() && ((!threshold || threshold > cells.fill()) && 
          (!max_cells || max_cells > count) ); ++it) {
      cell = *it;
      
      if(cell->has_expired(ttl))
        continue;
      
      cell->write(cells);
      ++count;
      bytes -= cell->encoded_length();

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
    
    if(it == end())
      free();
    else
      erase(begin(), it);

    cell_count += count;
  }

  void write(DynamicBuffer& cells) const {
    cells.ensure(bytes);
    for(auto cell : *this) {
      if(cell->has_expired(ttl))
        continue;
      cell->write(cells);
    }
  }

  const std::string to_string(bool with_cells=false) const {
    std::string s("Cells(size=");
    s.append(std::to_string(size()));
    s.append(" bytes=");
    s.append(std::to_string(bytes));
    s.append(" type=");
    s.append(Types::to_string(type));
    s.append(" max_revs=");
    s.append(std::to_string(max_revs));
    s.append(" ttl=");
    s.append(std::to_string(ttl));
    if(with_cells) {
      s.append(" cells=[\n");
      for(auto cell : *this) {
        s.append(cell->to_string(type));
        s.append("\n");
      }
      s.append("]");
    }
    s.append(")");
    return s;
  }

  size_t            bytes;

  Types::Column     type;
  uint32_t          max_revs;
  uint64_t          ttl;
};


}}}
#endif