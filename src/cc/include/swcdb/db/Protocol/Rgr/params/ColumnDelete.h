/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_ColumnDelete_h
#define swcdb_db_protocol_rgr_params_ColumnDelete_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


class ColumnDelete : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnDelete(cid_t a_cid = DB::Schema::NO_CID) noexcept : cid(a_cid) { }

  //~ColumnDelete() { }

  cid_t  cid;

  protected:

  size_t internal_encoded_length() const override {
    return Serialization::encoded_length_vi64(cid);
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_vi64(bufp, cid);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    cid = Serialization::decode_vi64(bufp, remainp);
  }


};


}}}}}

#endif // swcdb_db_protocol_rgr_params_ColumnDelete_h
