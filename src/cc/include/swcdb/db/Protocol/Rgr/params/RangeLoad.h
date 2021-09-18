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
  RangeLoad() noexcept : rid(0) { }

  SWC_CAN_INLINE
  RangeLoad(const DB::Schema::Ptr& schema, rid_t a_rid) noexcept
            : schema_primitives(*schema.get()), rid(a_rid) {
  }

  //~RangeLoad() { }

  DB::SchemaPrimitives schema_primitives;
  rid_t                rid;

  private:

  size_t internal_encoded_length() const override {
    return schema_primitives.encoded_length() +
           Serialization::encoded_length_vi64(rid);
  }

  void internal_encode(uint8_t** bufp) const override {
    schema_primitives.encode(bufp);
    Serialization::encode_vi64(bufp, rid);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    schema_primitives.decode(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
  }

};



class RangeLoaded final : public Serializable {
  public:

  SWC_CAN_INLINE
  RangeLoaded(const DB::Types::KeySeq key_seq, int64_t a_revision=0) noexcept
              : intval(false), interval(key_seq), revision(a_revision) {
  }

  SWC_CAN_INLINE
  ~RangeLoaded() noexcept { }

  bool                intval;
  DB::Cells::Interval interval;
  int64_t             revision;

  private:

  size_t internal_encoded_length() const override {
    return 1 + (intval ? interval.encoded_length() : 0) +
           Serialization::encoded_length_vi64(revision);
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_bool(bufp, intval);
    if(intval)
      interval.encode(bufp);
    Serialization::encode_vi64(bufp, revision);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    if((intval = Serialization::decode_bool(bufp, remainp)))
      interval.decode(bufp, remainp, false);
    revision = Serialization::decode_vi64(bufp, remainp);
  }

};

}}}}}

#endif // swcdb_db_protocol_rgr_params_RangeLoad_h
