/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeIsLoaded_h
#define swcdb_db_protocol_rgr_params_RangeIsLoaded_h

#include "swcdb/db/Protocol/Common/params/ColRangeId.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {

class RangeIsLoaded : public Common::Params::ColRangeId {
  public:

  RangeIsLoaded() {}
  RangeIsLoaded(cid_t cid, rid_t rid)
                : Common::Params::ColRangeId(cid, rid) { }
  virtual ~RangeIsLoaded() {}

};


class RangeIsLoadedRsp : public Serializable {
  public:

  enum Flags : uint8_t {
    NONE      = 0x00,
    CAN_MERGE = 0x01
  };

  RangeIsLoadedRsp(int err) : err(err), flags(NONE) { }

  virtual ~RangeIsLoadedRsp() {}

  void can_be_merged() {
    flags |= CAN_MERGE;
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
