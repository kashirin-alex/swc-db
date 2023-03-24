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
  RangeQueryUpdateReq() noexcept: cid(), rid() { }

  SWC_CAN_INLINE
  RangeQueryUpdateReq(cid_t a_cid, rid_t a_rid) noexcept
                      : cid(a_cid), rid(a_rid) {
  }

  //~RangeQueryUpdateReq() { }

  void print(std::ostream& out) const;

  cid_t           cid;
  rid_t           rid;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RangeQueryUpdateRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeQueryUpdateRsp(int a_err = Error::OK) noexcept
                      : err(a_err), cells_added(0),
                        range_prev_end(), range_end() {
  }

  RangeQueryUpdateRsp(int err, const uint8_t* ptr, size_t remain) noexcept;

  SWC_CAN_INLINE
  ~RangeQueryUpdateRsp() noexcept { }

  void print(std::ostream& out) const;

  int32_t       err;
  uint32_t      cells_added;
  DB::Cell::Key range_prev_end;
  DB::Cell::Key range_end;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.cc"
#endif

#endif // swcdb_db_protocol_rgr_params_RangeQueryUpdate_h
