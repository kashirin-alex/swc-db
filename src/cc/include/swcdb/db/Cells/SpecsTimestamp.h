/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsTimestamp_h
#define swcdb_db_cells_SpecsTimestamp_h

#include <string>
#include "swcdb/core/Comparators.h"


namespace SWC { namespace DB { namespace Specs {

     
class Timestamp {
  public:

  explicit Timestamp();

  explicit Timestamp(int64_t timestamp, Condition::Comp comp);
  
  explicit Timestamp(const Timestamp &other);

  void copy(const Timestamp &other);

  void set(int64_t timestamp, Condition::Comp comperator);

  void free();

  virtual ~Timestamp();

  const bool empty() const;

  const bool equal(const Timestamp &other) const;

  const size_t encoded_length() const;

  void encode(uint8_t **bufp) const;

  void decode(const uint8_t **bufp, size_t *remainp);

  const bool is_matching(int64_t other) const;

  const std::string to_string() const;

  void display(std::ostream& out) const;

  int64_t          value; 
  Condition::Comp  comp;
  bool             was_set;
  
};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsTimestamp.cc"
#endif 

#endif