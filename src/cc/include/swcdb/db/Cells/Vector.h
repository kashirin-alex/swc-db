/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Cells_Vector_h
#define swcdb_lib_db_Cells_Vector_h

#include "swcdb/db/Cells/Cell.h"

namespace SWC { namespace DB { namespace Cells {


class Vector : public std::vector<Cell*> {
  public:

  typedef std::shared_ptr<Vector> Ptr;

  using std::vector<Cell*>::vector;

  
  inline static Ptr make(const uint32_t cap=0) {
    return std::make_shared<Vector>(cap);
  }

  Vector(const uint32_t cap=0) 
        : std::vector<Cell*>(cap), 
          bytes(0) {
  }

  virtual ~Vector() {
    free();
  }

  void free() {
    for(auto & cell : *this)
      if(cell)
        delete cell;
    clear();
    bytes = 0;
  }

  void take(Vector& other) {
    bytes += other.bytes;
    insert(end(), other.begin(), other.end());
    
    other.clear();
    other.bytes = 0;
  }

  void add(const Cell& cell) {
    push_back(new Cell(cell));
    bytes += cell.encoded_length();
  }
  
  const size_t add(const uint8_t* ptr, size_t remain) {
    bytes += remain;
    size_t count = 0;
    Cell* cell;
    while(remain) {
      push_back(new Cell(&ptr, &remain, true));
      ++count;
    }
    return count;
  }

  const size_t size_bytes() const {
    return bytes;
  }

  const std::string to_string(bool with_cells=false) const {
    std::string s("Cells(size=");
    s.append(std::to_string(size()));
    s.append(" bytes=");
    s.append(std::to_string(bytes));
    //s.append(" type=");
    //s.append(Types::to_string(type));
    //s.append(" max_revs=");
    //s.append(std::to_string(m_max_revs));
    //s.append(" ttl=");
    //s.append(std::to_string(m_ttl));
    if(with_cells) {
      s.append(" cells=[\n");
      for(auto cell : *this) {
        //s.append(cell->to_string(type));
        s.append("\n");
      }
      s.append("]");
    }
    s.append(")");
    return s;
  }

  size_t  bytes;
};


}}}
#endif