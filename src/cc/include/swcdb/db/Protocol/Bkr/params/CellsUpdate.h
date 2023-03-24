/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_params_CellsUpdate_h
#define swcdb_db_protocol_bkr_params_CellsUpdate_h

#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Types/Identifiers.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Params {

class CellsUpdateReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  CellsUpdateReq() noexcept: cid() { }

  SWC_CAN_INLINE
  CellsUpdateReq(cid_t a_cid) noexcept : cid(a_cid) { }

  //~CellsUpdateReq() { }

  void print(std::ostream& out) const;

  cid_t           cid;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class CellsUpdateRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  CellsUpdateRsp(int a_err = Error::OK) noexcept : err(a_err) { }

  CellsUpdateRsp(int err, const uint8_t* ptr, size_t remain) noexcept;

  //~CellsUpdateRsp() { }

  void print(std::ostream& out) const;

  int32_t       err;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/params/CellsUpdate.cc"
#endif

#endif // swcdb_db_protocol_bkr_params_CellsUpdate_h
