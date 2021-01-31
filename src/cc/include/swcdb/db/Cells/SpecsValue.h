/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsValue_h
#define swcdb_db_cells_SpecsValue_h


#include "swcdb/core/Comparators.h"
#include "swcdb/db/Cells/Cell.h"


namespace SWC { namespace DB { namespace Specs {

class Value {
  public:

  explicit Value(bool own=true) noexcept;

  explicit Value(const char* data_n, Condition::Comp comp_n,
                 bool owner=false);

  explicit Value(const char* data_n, const uint32_t size_n,
                 Condition::Comp comp_n, bool owner=false);

  explicit Value(const uint8_t* data_n, const uint32_t size_n,
                 Condition::Comp comp_n, bool owner=false);

  explicit Value(int64_t count, Condition::Comp comp_n);

  explicit Value(const Value &other);

  explicit Value(Value&& other) noexcept;

  void set_counter(int64_t count, Condition::Comp comp_n);

  void set(const char* data_n, Condition::Comp comp_n, bool owner=true);

  void set(const std::string& data_n, Condition::Comp comp_n);

  void copy(const Value &other);

  void set(const char* data_n, uint32_t size_n,
           Condition::Comp comp_n, bool owner=true);

  void set(const uint8_t* data_n, const uint32_t size_n,
           Condition::Comp comp_n, bool owner=false);

  ~Value();

  void _free();

  void free();

  bool empty() const noexcept;

  bool equal(const Value &other) const noexcept;

  size_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  bool is_matching(Types::Column col_type, const Cells::Cell& cell) const;

  bool is_matching_plain(const Cells::Cell& cell) const;

  bool is_matching_serial(const Cells::Cell& cell) const;

  bool is_matching_counter(const Cells::Cell& cell) const;

  std::string to_string(Types::Column col_type) const;

  void print(Types::Column col_type, std::ostream& out) const;

  void display(Types::Column col_type, std::ostream& out,
               bool pretty=true) const;

  bool            own;
  Condition::Comp comp;

  uint8_t*        data;
  uint32_t        size;

  struct TypeMatcher {
    virtual ~TypeMatcher() { }
  };
  private:
  mutable TypeMatcher*  matcher;
};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsValue.cc"
#endif

#endif // swcdb_db_cells_SpecsValue_h
