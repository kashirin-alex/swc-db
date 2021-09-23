/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_ColumnCompact_h
#define swcdb_db_protocol_rgr_params_ColumnCompact_h

#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Types/Identifiers.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


class ColumnCompactReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnCompactReq(cid_t a_cid=0) noexcept : cid(a_cid) { }

  //~ColumnCompactReq() { }

  cid_t cid;

  std::string to_string() const;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class ColumnCompactRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnCompactRsp(int a_err = Error::OK) noexcept : err(a_err) { }

  ColumnCompactRsp(int err, const uint8_t* ptr, size_t remain) noexcept;

  //~ColumnCompactRsp() { }

  int err;

  std::string to_string() const;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.cc"
#endif

#endif // swcdb_db_protocol_rgr_params_ColumnCompact_h
