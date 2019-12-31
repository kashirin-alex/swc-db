/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Cells_Vector_h
#define swcdb_lib_db_Cells_Vector_h

#include "swcdb/db/Cells/Cell.h"

namespace SWC { namespace DB { namespace Cells {


class Vector  {
  public:

  typedef std::shared_ptr<Vector>          Ptr;
  
  inline static Ptr make(const uint32_t cap=0) {
    return std::make_shared<Vector>(cap);
  }

  Vector(const uint32_t cap=0) 
        : cells(std::vector<Cell*>(cap)) {
  }

  virtual ~Vector() {
    free();
  }

  void free() {
    for(auto & cell : cells)
      if(cell)
        delete cell;
    cells.clear();
  }

  void add(Vector& other) {
    for(auto cell : other.cells)
      cells.push_back(cell);
  }

  void add(const Cell& cell) {
    cells.push_back(new Cell(cell));
  }  
  
  const size_t add(const uint8_t* ptr, size_t remain) {
    size_t count = 0;
    Cell* cell;
    while(remain) {
      cells.push_back(new Cell(&ptr, &remain, true));
      count++;
    }
    return count;
  }

  std::vector<Cell*>  cells;
};


}}}
#endif