/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_common_params_ColRangeId_h
#define swcdb_db_protocol_common_params_ColRangeId_h

#include "swcdb/core/comm/Serializable.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Common { namespace Params {


class ColRangeId : public Serializable {
  public:

  ColRangeId(cid_t cid = 0, rid_t rid = 0)
            : cid(cid), rid(rid){
  }

  virtual ~ColRangeId() {}

  cid_t  cid;
  rid_t  rid;


  protected:

  size_t internal_encoded_length() const override {
    return Serialization::encoded_length_vi64(cid)
         + Serialization::encoded_length_vi64(rid);
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
  }

};


}}}}}

#endif // swcdb_db_protocol_common_params_ColRangeId_h
