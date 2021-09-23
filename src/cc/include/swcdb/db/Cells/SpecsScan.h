/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsScan_h
#define swcdb_db_cells_SpecsScan_h


#include "swcdb/db/Cells/SpecsColumn.h"


namespace SWC { namespace DB {


//! The SWC-DB Specifications C++ namespace 'SWC::DB::Specs'
namespace Specs {


class Scan final {
  public:

  typedef Core::Vector<Column> Columns;

  SWC_CAN_INLINE
  explicit Scan() noexcept { }

  SWC_CAN_INLINE
  explicit Scan(uint32_t reserve) {
    columns.reserve(reserve);
  }

  SWC_CAN_INLINE
  explicit Scan(const Scan& other)
                : columns(other.columns), flags(other.flags) {
  }

  SWC_CAN_INLINE
  explicit Scan(Scan&& other) noexcept
                : columns(std::move(other.columns)), flags(other.flags) {
  }

  SWC_CAN_INLINE
  explicit Scan(const Columns& a_columns) : columns(a_columns) { }

  SWC_CAN_INLINE
  explicit Scan(Columns&& a_columns) noexcept
                : columns(std::move(a_columns)) { }

  SWC_CAN_INLINE
  explicit Scan(const uint8_t** bufp, size_t* remainp) {
    decode(bufp, remainp);
  }

  ~Scan() noexcept { }

  SWC_CAN_INLINE
  Scan& operator=(Scan&& other) noexcept {
    columns = std::move(other.columns);
    flags.copy(other.flags);
    return *this;
  }

  SWC_CAN_INLINE
  Scan& operator=(const Scan& other) {
    columns = other.columns;
    flags.copy(other.flags);
    return *this;
  }

  SWC_CAN_INLINE
  void free() noexcept {
    columns.clear();
  }

  bool SWC_PURE_FUNC equal(const Scan &other) const noexcept;

  size_t SWC_PURE_FUNC encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  SWC_CAN_INLINE
  std::string to_string() const {
    std::string s;
    {
      std::stringstream ss;
      print(ss);
      s = ss.str();
    }
    return s;
  }

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty=true,
               std::string offset = "") const;

  Columns   columns;
  Flags     flags;

};

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsScan.cc"
#endif

#endif // swcdb_db_cells_SpecsScan_h
