/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_Vector_h
#define swcdb_db_Cells_Vector_h

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

  explicit Vector(const uint32_t max_revs=1, const uint64_t ttl_ns=0, 
                  const Types::Column type=Types::Column::PLAIN);

  explicit Vector(Vector& other);

  Vector& operator=(const Vector& other) = delete;

  virtual ~Vector();

  void free();

  void reset(const uint32_t revs=1, const uint64_t ttl_ns=0, 
             const Types::Column typ=Types::Column::PLAIN);

  void configure(const uint32_t revs=1, const uint64_t ttl_ns=0,
                 const Types::Column typ=Types::Column::PLAIN);

  void take(Vector& other);

  void add(const Cell& cell, bool no_value=false);
  
  const size_t add(const uint8_t* ptr, size_t remain);

  const size_t size_bytes() const;

  Cell* takeout_begin(size_t idx);

  Cell* takeout_end(size_t idx);

  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold, 
                      uint32_t max_cells);

  void write(DynamicBuffer& cells) const;

  const std::string to_string(bool with_cells=false) const;

  size_t            bytes;

  Types::Column     type;
  uint32_t          max_revs;
  uint64_t          ttl;
};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Vector.cc"
#endif 

#endif // swcdb_db_Cells_Vector_h