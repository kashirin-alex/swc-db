/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeUnoad_h
#define swcdb_db_protocol_rgr_params_RangeUnoad_h


#include "swcdb/db/Protocol/Common/params/ColRangeId.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


class RangeUnload final : public Common::Params::ColRangeId {
  public:

  enum Flag : uint8_t {
    NONE         = 0x00,
    CHECK_EMPTY  = 0x01
  };

  SWC_CAN_INLINE
  RangeUnload() noexcept : flags(NONE) { }

  SWC_CAN_INLINE
  RangeUnload(cid_t cid, rid_t rid, uint8_t flags=NONE) noexcept
             : Common::Params::ColRangeId(cid, rid), flags(flags) {
  }

  //~RangeUnload() { }

  uint8_t  flags;

  private:

  size_t internal_encoded_length() const override {
    return Common::Params::ColRangeId::internal_encoded_length() + 1;
  }

  void internal_encode(uint8_t** bufp) const override {
    Common::Params::ColRangeId::internal_encode(bufp);
    Serialization::encode_i8(bufp, flags);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    Common::Params::ColRangeId::internal_decode(bufp, remainp);
    flags = Serialization::decode_i8(bufp, remainp);
  }

};


class RangeUnloadRsp final : public Serializable {
  public:

  enum Flag : uint8_t {
    NONE         = 0x00,
    EMPTY        = 0x01
  };

  SWC_CAN_INLINE
  RangeUnloadRsp(int err) noexcept : err(err), flags(NONE) { }

  //~RangeUnloadRsp() { }

  void set_empty() {
    flags |= EMPTY;
  }

  int       err;
  uint8_t   flags;

  private:

  size_t internal_encoded_length() const override {
    return Serialization::encoded_length_vi32(err) + (err ? 0 : 1);
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_vi32(bufp, err);
    if(!err)
      Serialization::encode_i8(bufp, flags);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    if(!(err = Serialization::decode_vi32(bufp, remainp)))
      flags = Serialization::decode_i8(bufp, remainp);
  }

};

}}}}}

#endif // swcdb_db_protocol_rgr_params_RangeIsLoaded_h
