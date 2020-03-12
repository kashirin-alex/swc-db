/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsFlags_h
#define swcdb_db_cells_SpecsFlags_h

#include <string>


namespace SWC { namespace DB { namespace Specs {


class Flags {
  public:

  static const uint8_t NONE           = 0x00;
  static const uint8_t LIMIT_BY_KEYS  = 0x01;
  static const uint8_t OFFSET_BY_KEYS = 0x04;
  static const uint8_t ONLY_KEYS      = 0x08;
  static const uint8_t ONLY_DELETES   = 0x10;

  explicit Flags();

  explicit Flags(const Flags &other);

  void copy(const Flags &other);

  virtual ~Flags();

  const bool is_only_keys() const;

  const bool is_only_deletes() const;

  void set_only_keys();

  void set_only_deletes();

  const bool equal(const Flags &other) const;

  const size_t encoded_length() const;

  void encode(uint8_t **bufp) const;
  
  void decode(const uint8_t **bufp, size_t *remainp);
  
  const std::string to_string() const;
  
  
  void display(std::ostream& out) const;

  uint64_t 	limit, offset;
  uint32_t  max_versions;
  uint8_t 	options;
  bool      was_set;

};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsFlags.cc"
#endif 

#endif