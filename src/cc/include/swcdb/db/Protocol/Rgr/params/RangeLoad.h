/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeLoad_h
#define swcdb_db_protocol_rgr_params_RangeLoad_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


class RangeLoad final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeLoad() noexcept : cid(0), rid(0) { }

  SWC_CAN_INLINE
  RangeLoad(cid_t cid, rid_t rid, const DB::Schema::Ptr& schema) noexcept
            : cid(cid), rid(rid), schema(schema) {
  }

  //~RangeLoad() { }

  cid_t           cid;
  rid_t           rid;
  DB::Schema::Ptr schema;

  private:

  size_t internal_encoded_length() const override {
    return  Serialization::encoded_length_vi64(cid)
          + Serialization::encoded_length_vi64(rid)
          + schema->encoded_length();
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    schema->encode(bufp);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    schema.reset(new DB::Schema(bufp, remainp));
  }

};



class RangeLoaded final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeLoaded(const DB::Types::KeySeq key_seq) noexcept
              : intval(false), interval(key_seq) {
  }

  //~RangeLoaded() { }

  bool                intval;
  DB::Cells::Interval interval;

  private:

  size_t internal_encoded_length() const override {
    return 1 + (intval ? interval.encoded_length() : 0);
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_bool(bufp, intval);
    if(intval)
      interval.encode(bufp);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    if((intval = Serialization::decode_bool(bufp, remainp)))
      interval.decode(bufp, remainp, false);
  }

};

}}}}}

#endif // swcdb_db_protocol_rgr_params_RangeLoad_h
