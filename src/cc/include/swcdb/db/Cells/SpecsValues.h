/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsValues_h
#define swcdb_db_cells_SpecsValues_h


#include "swcdb/db/Cells/SpecsValue.h"


namespace SWC { namespace DB { namespace Specs {


class Values : private std::vector<Value> {
  public:
  
  typedef std::vector<Value> Vec;
  
  using Vec::empty;
  using Vec::size;
  using Vec::begin;
  using Vec::end;
  using Vec::front;
  using Vec::back;
  using Vec::operator[];

  Types::Column col_type;

  Values(Types::Column col_type = Types::Column::UNKNOWN) 
        : col_type(col_type) { 
  }

  Values(const Values& other) = delete;

  Values(Values&& other) = delete;

  void copy(const Values& other);

  void free();

  Value& add();

  Value& add(Value&& other);

  size_t size_of_internal() const;

  bool equal(const Values& other) const;
  
  bool is_matching(const Cells::Cell& cell) const;

  size_t encoded_length() const;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty, 
               const std::string& offset) const;

};


}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsValues.cc"
#endif 


#endif // swcdb_db_cells_SpecsValues_h
