/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsValue_h
#define swcdb_db_cells_SpecsValue_h


#include "swcdb/core/Comparators.h"


namespace SWC { namespace DB { namespace Specs {

class Value {
  public:

  explicit Value(bool own=true);

  explicit Value(const char* data_n, Condition::Comp comp_n,
                 bool owner=false);

  explicit Value(const char* data_n, const uint32_t size_n, 
                 Condition::Comp comp_n, bool owner=false);

  explicit Value(const uint8_t* data_n, const uint32_t size_n, 
                 Condition::Comp comp_n, bool owner=false);

  explicit Value(int64_t count, Condition::Comp comp_n);

  explicit Value(const Value &other);

  void set_counter(int64_t count, Condition::Comp comp_n);

  void set(const char* data_n, Condition::Comp comp_n, bool owner=true);
  
  void set(const std::string& data_n, Condition::Comp comp_n);

  void copy(const Value &other);
  
  void set(const uint8_t* data_n, const uint32_t size_n, 
           Condition::Comp comp_n, bool owner=false);

  ~Value();

  void free();

  bool empty() const;

  bool equal(const Value &other) const;

  size_t encoded_length() const;
  
  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  bool is_matching(const uint8_t *other_data, const uint32_t other_size) const;
  
  bool is_matching(int64_t other) const;

  std::string to_string() const;

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty=true) const;

  bool            own;
  uint8_t*        data;
  uint32_t        size;    
  Condition::Comp comp;

};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsValue.cc"
#endif 

#endif // swcdb_db_cells_SpecsValue_h
