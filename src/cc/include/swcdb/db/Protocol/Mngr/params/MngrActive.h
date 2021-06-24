/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_protocol_mngr_params_MngrActive_h
#define swcdb_protocol_mngr_params_MngrActive_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/client/service/mngr/Groups.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class MngrActiveReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  MngrActiveReq(uint8_t role=DB::Types::MngrRole::COLUMNS, cid_t cid=0)
                noexcept : role(role), cid(cid) {
  }

  //~MngrActiveReq() { }

  uint8_t   role;
  cid_t     cid;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class MngrActiveRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  MngrActiveRsp() noexcept { }

  SWC_CAN_INLINE
  MngrActiveRsp(const EndPoints& endpoints)
                : endpoints(endpoints) {
  }

  //~MngrActiveRsp() { }

  EndPoints endpoints;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/MngrActive.cc"
#endif

#endif // swcdb_protocol_mngr_params_MngrActive_h
