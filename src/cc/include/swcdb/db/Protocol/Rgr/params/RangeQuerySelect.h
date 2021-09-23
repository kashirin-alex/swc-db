/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeQuerySelect_h
#define swcdb_db_protocol_rgr_params_RangeQuerySelect_h

#include "swcdb/core/Buffer.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


class RangeQuerySelectReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeQuerySelectReq() noexcept { }

  SWC_CAN_INLINE
  RangeQuerySelectReq(cid_t a_cid, rid_t a_rid,
                      const DB::Specs::Interval& a_interval)
                      : cid(a_cid), rid(a_rid), interval(a_interval) {
  }

  SWC_CAN_INLINE
  ~RangeQuerySelectReq() noexcept { }

  void print(std::ostream& out) const;

  cid_t                cid;
  rid_t                rid;
  DB::Specs::Interval  interval;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RangeQuerySelectReqRef final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeQuerySelectReqRef(cid_t a_cid, rid_t a_rid,
                         const DB::Specs::Interval& a_interval) noexcept
                         : cid(a_cid), rid(a_rid), interval(a_interval) {
  }

  //~RangeQuerySelectReqRef() { }

  void print(std::ostream& out) const;

  cid_t                      cid;
  rid_t                      rid;
  const DB::Specs::Interval& interval;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  // not-optional/available
  void internal_decode(const uint8_t**, size_t*) override { }

};



class RangeQuerySelectRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeQuerySelectRsp(int a_err = Error::OK, bool a_reached_limit=false,
                      uint64_t a_offset=0) noexcept
                      : err(a_err), reached_limit(a_reached_limit),
                        offset(a_offset) {
  }

  SWC_CAN_INLINE
  RangeQuerySelectRsp(int a_err, StaticBuffer& a_data) noexcept
                      : err(a_err), reached_limit(false),
                        offset(0), data(a_data) {
  }

  RangeQuerySelectRsp(int err, const uint8_t *ptr, size_t remain,
                      StaticBuffer& data) noexcept;

  SWC_CAN_INLINE
  ~RangeQuerySelectRsp() noexcept { }

  void print(std::ostream& out) const;

  int32_t         err;
  bool            reached_limit;
  uint64_t        offset;
  StaticBuffer    data;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.cc"
#endif

#endif // swcdb_db_protocol_rgr_params_RangeQuerySelect_h
