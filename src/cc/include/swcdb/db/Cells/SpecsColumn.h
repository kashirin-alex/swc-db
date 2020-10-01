/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsColumn_h
#define swcdb_db_cells_SpecsColumn_h

#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace DB { namespace Specs {


class Column final : public Comm::Serializable {
  public:
  
  typedef std::vector<Interval::Ptr> Intervals;
  typedef std::shared_ptr<Column> Ptr;
  
  static Ptr make_ptr(cid_t cid=0, uint32_t reserve=0);

  static Ptr make_ptr(cid_t cid, const Intervals& intervals);

  static Ptr make_ptr(const uint8_t** bufp, size_t* remainp);

  static Ptr make_ptr(const Column& other);

  static Ptr make_ptr(Ptr other);

  explicit Column(cid_t cid=0, uint32_t reserve=0);

  explicit Column(cid_t cid, const Intervals& intervals);

  explicit Column(const uint8_t** bufp, size_t* remainp);

  explicit Column(const Column& other);

  void copy(const Column &other);

  ~Column();

  void free();

  bool equal(const Column &other);

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);

  std::string to_string() const;

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty=false, 
               std::string offset = "") const;
  
  cid_t     cid;
  Intervals intervals;
};

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsColumn.cc"
#endif 

#endif // swcdb_db_cells_SpecsColumn_h
