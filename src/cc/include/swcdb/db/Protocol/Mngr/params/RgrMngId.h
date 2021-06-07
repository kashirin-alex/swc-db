/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_RgrMngId_h
#define swcdb_db_protocol_mngr_params_RgrMngId_h

#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class RgrMngId final : public Common::Params::HostEndPoints {
  public:

  enum Flag : uint8_t {
    MNGR_ASSIGNED   = 1,
    MNGR_NOT_ACTIVE = 2,
    MNGR_REASSIGN   = 3,
    MNGR_REREQ      = 4,
    MNGR_ACK        = 5,
    RS_REQ          = 6, // >= params with host endpoints
      RS_ACK          = 7,
    RS_DISAGREE     = 8,
    RS_SHUTTINGDOWN = 9
    };

  SWC_CAN_INLINE
  RgrMngId() noexcept {}

  SWC_CAN_INLINE
  RgrMngId(rgrid_t rgrid, Flag flag, const EndPoints& endpoints)
          : Common::Params::HostEndPoints(endpoints),
            rgrid(rgrid), flag(flag) {
  }

  //~RgrMngId() {}

  rgrid_t         rgrid;
  Flag            flag;
  FS::Type        fs;

  private:

  size_t internal_encoded_length() const override {
    size_t len = 1;
    if(flag != Flag::MNGR_NOT_ACTIVE && flag != Flag::MNGR_ACK)
      len += Serialization::encoded_length_vi64(rgrid);

    if(flag >= Flag::RS_REQ)
      len +=  Common::Params::HostEndPoints::internal_encoded_length();

    if(flag == Flag::MNGR_ASSIGNED)
      ++len; // fs-type
    return len;
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_i8(bufp, flag);
    if(flag != Flag::MNGR_NOT_ACTIVE && flag != Flag::MNGR_ACK)
      Serialization::encode_vi64(bufp, rgrid);

    if(flag >= Flag::RS_REQ)
      Common::Params::HostEndPoints::internal_encode(bufp);

    if(flag == Flag::MNGR_ASSIGNED)
      Serialization::encode_i8(bufp, fs);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    flag = Flag(Serialization::decode_i8(bufp, remainp));
    if(flag != Flag::MNGR_NOT_ACTIVE && flag != Flag::MNGR_ACK)
      rgrid = Serialization::decode_vi64(bufp, remainp);

    if(flag >= Flag::RS_REQ)
      Common::Params::HostEndPoints::internal_decode(bufp, remainp);

    if(flag == Flag::MNGR_ASSIGNED)
      fs = FS::Type(Serialization::decode_i8(bufp, remainp));
  }

};


}}}}}

#endif // swcdb_db_protocol_params_RgrMngId_h
