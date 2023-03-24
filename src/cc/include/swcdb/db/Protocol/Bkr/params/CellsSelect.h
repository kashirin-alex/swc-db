/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_params_CellsSelect_h
#define swcdb_db_protocol_bkr_params_CellsSelect_h

#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Cells/SpecsScan.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Params {

class CellsSelectReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  CellsSelectReq() noexcept: cid(), interval() { }

  SWC_CAN_INLINE
  CellsSelectReq(cid_t a_cid, const DB::Specs::Interval& a_interval)
                : cid(a_cid), interval(a_interval) {
  }

  ~CellsSelectReq() noexcept { }

  void print(std::ostream& out) const;

  cid_t                cid;
  DB::Specs::Interval  interval;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class CellsSelectReqRef final : public Serializable {
  public:

  SWC_CAN_INLINE
  CellsSelectReqRef(cid_t a_cid,
                    const DB::Specs::Interval& a_interval) noexcept
                    : cid(a_cid), interval(a_interval) {
  }

  //~CellsSelectReqRef() { }

  void print(std::ostream& out) const;

  cid_t                       cid;
  const DB::Specs::Interval&  interval;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  // not-available/option
  void internal_decode(const uint8_t**, size_t*) override { }

};



class CellsSelectRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  CellsSelectRsp(int a_err = Error::OK, bool a_more=false,
                 uint64_t a_offset=0) noexcept
                : err(a_err), more(a_more), offset(a_offset), data() {
  }

  CellsSelectRsp(int err, const uint8_t* ptr, size_t remain,
                 StaticBuffer& data) noexcept;

  ~CellsSelectRsp() noexcept { }

  void print(std::ostream& out) const;

  int32_t         err;
  bool            more;
  uint64_t        offset;
  StaticBuffer    data;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/params/CellsSelect.cc"
#endif

#endif // swcdb_db_protocol_bkr_params_CellsSelect_h
