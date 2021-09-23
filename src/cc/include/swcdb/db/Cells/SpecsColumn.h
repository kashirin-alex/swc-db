/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsColumn_h
#define swcdb_db_cells_SpecsColumn_h


#include "swcdb/db/Cells/SpecsInterval.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace DB { namespace Specs {


class Column final : public Core::Vector<Interval::Ptr> {
  public:

  typedef Core::Vector<Interval::Ptr> Intervals;

  SWC_CAN_INLINE
  explicit Column(cid_t a_cid=Schema::NO_CID) noexcept : cid(a_cid) { }

  SWC_CAN_INLINE
  explicit Column(cid_t a_cid, uint32_t _reserve) : cid(a_cid) {
    reserve(_reserve);
  }

  SWC_CAN_INLINE
  explicit Column(cid_t a_cid, const Intervals& intervals)
                  : Intervals(intervals), cid(a_cid) {
  }

  SWC_CAN_INLINE
  explicit Column(cid_t a_cid, Intervals&& intervals) noexcept
                  : Intervals(std::move(intervals)), cid(a_cid) {
  }

  SWC_CAN_INLINE
  explicit Column(const uint8_t** bufp, size_t* remainp) {
    decode(bufp, remainp);
  }

  SWC_CAN_INLINE
  explicit Column(const Column& other) : Intervals() {
    copy(other);
  }

  SWC_CAN_INLINE
  explicit Column(Column&& other) noexcept
          : Intervals(std::move(other)), cid(other.cid) {
  }

  ~Column() noexcept { }

  SWC_CAN_INLINE
  Column& operator=(Column&& other) noexcept {
    Intervals::operator=(std::move(other));
    cid = other.cid;
    return *this;
  }

  SWC_CAN_INLINE
  Column& operator=(const Column& other) {
    copy(other);
    return *this;
  }

  void copy(const Column &other);

  Interval::Ptr& add(Types::Column col_type);

  bool SWC_PURE_FUNC equal(const Column &other) const noexcept;

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

  void display(std::ostream& out, bool pretty=false,
               std::string offset = "") const;

  cid_t     cid;
};

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsColumn.cc"
#endif

#endif // swcdb_db_cells_SpecsColumn_h
