/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsColumn_h
#define swcdb_db_cells_SpecsColumn_h


#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace DB { namespace Specs {


class Column final : public std::vector<Interval::Ptr> {
  public:

  typedef std::vector<Interval::Ptr> Intervals;
  typedef std::shared_ptr<Column>    Ptr;

  SWC_CAN_INLINE
  static Ptr make_ptr(cid_t cid=0, uint32_t reserve=0) {
    return Ptr(new Column(cid, reserve));
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(cid_t cid, const Intervals& intervals) {
    return Ptr(new Column(cid, intervals));
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(const uint8_t** bufp, size_t* remainp) {
    return Ptr(new Column(bufp, remainp));
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(const Column& other) {
    return Ptr(new Column(other));
  }

  SWC_CAN_INLINE
  static Ptr make_ptr(Ptr other) {
    return Ptr(new Column(*other.get()));
  }


  SWC_CAN_INLINE
  explicit Column(cid_t cid=0, uint32_t _reserve=0)
                  : cid(cid) {
    reserve(_reserve);
  }

  SWC_CAN_INLINE
  explicit Column(cid_t cid, const Intervals& intervals)
                  : Intervals(intervals), cid(cid) {
  }

  SWC_CAN_INLINE
  explicit Column(const uint8_t** bufp, size_t* remainp) {
    decode(bufp, remainp);
  }

  SWC_CAN_INLINE
  explicit Column(const Column& other) : Intervals() {
    copy(other);
  }

  void copy(const Column &other);

  //~Column() { }

  Interval::Ptr& add(Types::Column col_type);

  bool equal(const Column &other) const noexcept;

  size_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  std::string to_string() const;

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
