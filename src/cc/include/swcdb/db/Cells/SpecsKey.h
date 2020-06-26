/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsKey_h
#define swcdb_db_cells_SpecsKey_h

#include <vector>
#include <string>
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Cells/CellKey.h"

namespace SWC { namespace DB { namespace Specs {
      

struct Fraction final : public std::string {

  Condition::Comp comp;

  bool operator==(const Fraction &other) const;
  
  uint32_t encoded_length() const;
  
  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);
};


class Key final : public std::vector<Fraction> {
  public:

  using std::vector<Fraction>::insert;

  typedef std::shared_ptr<Key> Ptr;

  explicit Key();

  explicit Key(const DB::Cell::Key &cell_key, Condition::Comp comp);

  ~Key();

  void free();

  void copy(const Key &other);

  bool equal(const Key &other) const;

  void set(const DB::Cell::Key &cell_key, Condition::Comp comp);

  void set(int32_t idx, Condition::Comp comp);

  void add(const char* buf, uint32_t len, Condition::Comp comp);

  void add(const std::string& fraction, Condition::Comp comp);

  void add(const std::string_view& fraction, Condition::Comp comp);

  void add(const char* fraction, Condition::Comp comp);

  void add(const uint8_t* fraction, uint32_t len, Condition::Comp comp);
  

  void insert(uint32_t idx, const char* buf, uint32_t len, 
              Condition::Comp comp);

  void insert(uint32_t idx, const std::string& fraction, 
              Condition::Comp comp);

  void insert(uint32_t idx, const std::string_view& fraction, 
              Condition::Comp comp);

  void insert(uint32_t idx, const uint8_t* fraction, uint32_t len,
              Condition::Comp comp);

  void insert(uint32_t idx, const char* fraction, Condition::Comp comp);

  std::string_view get(const uint32_t idx, Condition::Comp& comp) const;

  std::string_view get(const uint32_t idx) const;

  void get(DB::Cell::Key& key) const;

  void remove(uint32_t idx, bool recursive=false);
  
  //bool is_matching(const DB::Cell::Key &other) const;

  uint32_t encoded_length() const;
  
  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp, bool owner = true);

  std::string to_string() const;

  void display(std::ostream& out, bool pretty=true) const;

};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsKey.cc"
#endif 

#endif