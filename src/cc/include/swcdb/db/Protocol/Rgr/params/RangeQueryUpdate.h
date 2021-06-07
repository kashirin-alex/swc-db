/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeQueryUpdate_h
#define swcdb_db_protocol_rgr_params_RangeQueryUpdate_h

#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Types/Identifiers.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {

class RangeQueryUpdateReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeQueryUpdateReq() noexcept { }

  SWC_CAN_INLINE
  RangeQueryUpdateReq(cid_t cid, rid_t rid) noexcept : cid(cid), rid(rid) { }

  //~RangeQueryUpdateReq() { }

  void print(std::ostream& out) const;

  cid_t           cid;
  rid_t           rid;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RangeQueryUpdateRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeQueryUpdateRsp(int err = Error::OK) noexcept
                      : err(err), cells_added(0) {
  }

  RangeQueryUpdateRsp(int err, const uint8_t* ptr, size_t remain) noexcept;

  //~RangeQueryUpdateRsp() { }

  void print(std::ostream& out) const;

  int32_t       err;
  uint32_t      cells_added;
  DB::Cell::Key range_prev_end;
  DB::Cell::Key range_end;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.cc"
#endif

#endif // swcdb_db_protocol_rgr_params_RangeQueryUpdate_h
