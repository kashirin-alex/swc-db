/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_cells_CellKeyVec_h
#define swcdb_db_cells_CellKeyVec_h


#include <vector>
#include <string>
#include "swcdb/core/Comparators.h"


namespace SWC { namespace DB { namespace Cell {

class KeyVec : public std::vector<std::string> {
  public:

  using std::vector<std::string>::vector;

  void free();

  KeyVec operator=(const KeyVec &other) = delete;
  
  void copy(const KeyVec &other);

  const bool equal(const KeyVec &other) const;

  void add(const char* fraction, const uint32_t len);

  void add(const std::string& fraction);

  void add(const char* fraction);

  void add(const uint8_t* fraction, const uint32_t len);

  void insert(const uint32_t idx, const char* fraction, const uint32_t len);

  void insert(const uint32_t idx, const std::string& fraction);

  void insert(const uint32_t idx, const char* fraction);

  void insert(const uint32_t idx, const uint8_t* fraction, const uint32_t len);

  void set(const uint32_t idx, const char* fraction, const uint32_t len);

  void set(const uint32_t idx, const uint8_t* fraction, const uint32_t len);

  void set(const uint32_t idx, const std::string& fraction);

  void remove(const uint32_t idx);

  const std::string get(const uint32_t idx) const;

  void get(const uint32_t idx, std::string& fraction) const;

  const bool align(const KeyVec& other, Condition::Comp comp);

  const uint32_t encoded_length() const;
  
  void encode(uint8_t **bufp) const;

  void decode(const uint8_t **bufp, size_t* remainp);

  const std::string to_string() const;

};

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/CellKeyVec.cc"
#endif 

#endif // swcdb_db_Cells_CellKeyVec_h