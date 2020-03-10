/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsScan_h
#define swcdb_db_cells_SpecsScan_h

#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/SpecsColumn.h"


namespace SWC { namespace DB { namespace Specs {


class Scan : public Serializable {
  public:

  typedef std::vector<Column::Ptr> Columns;

  explicit Scan(uint32_t reserve=0);

  explicit Scan(Columns& columns);
       
  explicit Scan(const uint8_t **bufp, size_t *remainp);

  void copy(const Scan &other);
  
  virtual ~Scan();

  void free();

  bool equal(const Scan &other);

  uint8_t encoding_version() const;
  
  size_t encoded_length_internal() const;

  void encode_internal(uint8_t **bufp) const;

  void decode_internal(uint8_t version, const uint8_t **bufp,
	                	   size_t *remainp);
  
  const std::string to_string();
  
  void display(std::ostream& out, bool pretty=false, 
               std::string offset = "") const;

  Columns   columns;
  Flags     flags;

};

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsScan.cc"
#endif 

#endif