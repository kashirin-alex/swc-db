/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeLocate_h
#define swcdb_db_protocol_rgr_params_RangeLocate_h


#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Types/Identifiers.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {

class RangeLocateReq final : public Serializable {
  public:

  static const uint8_t CURRENT_RANGE  = 0x01;
  static const uint8_t NEXT_RANGE     = 0x02;
  static const uint8_t COMMIT         = 0x04;
  static const uint8_t RANGE_END_REST = 0x08;
  static const uint8_t KEY_EQUAL      = 0x10;
  static const uint8_t HAVE_REVISION  = 0x20;

  SWC_CAN_INLINE
  RangeLocateReq(cid_t a_cid=0, rid_t a_rid=0) noexcept
                : cid(a_cid), rid(a_rid), flags(0) {
  }

  SWC_CAN_INLINE
  ~RangeLocateReq() { }

  void print(std::ostream& out) const;

  cid_t          cid;
  rid_t          rid;
  DB::Cell::Key  range_begin, range_end, range_offset;
  uint8_t        flags;
  int64_t        revision;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RangeLocateRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeLocateRsp(int a_err = Error::OK) noexcept
                : err(a_err), cid(0), rid(0) {
  }

  RangeLocateRsp(int err, const uint8_t* ptr, size_t remain) noexcept;

  SWC_CAN_INLINE
  ~RangeLocateRsp() { }

  void print(std::ostream& out) const;

  int             err;
  cid_t           cid;
  rid_t           rid;
  DB::Cell::Key   range_end;
  DB::Cell::Key   range_begin;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeLocate.cc"
#endif

#endif // swcdb_db_protocol_rgr_params_RangeLocate_h
