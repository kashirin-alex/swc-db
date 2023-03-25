/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_RgrMngId_h
#define swcdb_db_protocol_mngr_params_RgrMngId_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class RgrMngId final : public Serializable {
  public:

  enum Flag : uint8_t {
    MNGR_ASSIGNED     = 1,
    MNGR_NOT_ACTIVE   = 2,
    MNGR_REASSIGN     = 3,
    MNGR_REREQ        = 4,
    MNGR_ACK          = 5,
    RGR_REQ           = 6, // >= params with host endpoints
    RGR_ACK           = 7,
    RGR_DISAGREE      = 8,
    RGR_SHUTTINGDOWN  = 9
    };

  SWC_CAN_INLINE
  RgrMngId() noexcept 
          : endpoints(), rgrid(), flag(),
            fs(FS::Type::UNKNOWN) {
  }

  SWC_CAN_INLINE
  RgrMngId(rgrid_t a_rgrid, Flag a_flag, const EndPoints& a_endpoints)
          : endpoints(a_endpoints),
            rgrid(a_rgrid), flag(a_flag),
            fs(FS::Type::UNKNOWN) {
  }

  SWC_CAN_INLINE
  ~RgrMngId() noexcept {}

  void print(std::ostream& out) const {
    out << "RgrMngId(id=" << rgrid;
    Comm::print(out << ' ', endpoints);
    out << " flag=" << int(flag);
    if(flag == Flag::MNGR_ASSIGNED)
      out << " fs=" << FS::to_string(fs);
    out << ')';
  }

  EndPoints       endpoints;
  rgrid_t         rgrid;
  Flag            flag;
  FS::Type        fs;

  private:

  size_t internal_encoded_length() const override {
    size_t len = 1;
    if(flag != Flag::MNGR_NOT_ACTIVE && flag != Flag::MNGR_ACK)
      len += Serialization::encoded_length_vi64(rgrid);

    if(flag >= Flag::RGR_REQ)
      len +=  Serialization::encoded_length(endpoints);

    if(flag == Flag::MNGR_ASSIGNED)
      ++len; // fs-type
    return len;
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_i8(bufp, flag);
    if(flag != Flag::MNGR_NOT_ACTIVE && flag != Flag::MNGR_ACK)
      Serialization::encode_vi64(bufp, rgrid);

    if(flag >= Flag::RGR_REQ)
      Serialization::encode(bufp, endpoints);

    if(flag == Flag::MNGR_ASSIGNED)
      Serialization::encode_i8(bufp, fs);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    flag = Flag(Serialization::decode_i8(bufp, remainp));
    if(flag != Flag::MNGR_NOT_ACTIVE && flag != Flag::MNGR_ACK)
      rgrid = Serialization::decode_vi64(bufp, remainp);

    if(flag >= Flag::RGR_REQ)
      Serialization::decode(bufp, remainp, endpoints);

    if(flag == Flag::MNGR_ASSIGNED)
      fs = FS::Type(Serialization::decode_i8(bufp, remainp));
  }

};


}}}}}

#endif // swcdb_db_protocol_params_RgrMngId_h
