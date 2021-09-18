/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_ColumnsUnload_h
#define swcdb_db_protocol_rgr_params_ColumnsUnload_h

#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


class ColumnsUnloadReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnsUnloadReq(cid_t a_cid_begin = DB::Schema::NO_CID,
                   cid_t a_cid_end = DB::Schema::NO_CID) noexcept
                  : cid_begin(a_cid_begin), cid_end(a_cid_end) {
  }

  //~ColumnsUnloadReq() { }

  cid_t cid_begin;
  cid_t cid_end;

  protected:

  size_t internal_encoded_length() const override {
    return Serialization::encoded_length_vi64(cid_begin)
         + Serialization::encoded_length_vi64(cid_end);
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_vi64(bufp, cid_begin);
    Serialization::encode_vi64(bufp, cid_end);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    cid_begin = Serialization::decode_vi64(bufp, remainp);
    cid_end = Serialization::decode_vi64(bufp, remainp);
  }

};





class ColumnsUnloadRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnsUnloadRsp(int a_err = Error::OK) noexcept : err(a_err) { }

  SWC_CAN_INLINE
  ~ColumnsUnloadRsp() noexcept { }

  int                               err;
  std::unordered_map<cid_t, rids_t> columns;

  private:

  size_t internal_encoded_length() const override {
    size_t sz = Serialization::encoded_length_vi32(err);
    if(err)
      return sz;
    sz += Serialization::encoded_length_vi64(columns.size());
    for(auto it = columns.cbegin(); it != columns.cend(); ++it) {
      sz += Serialization::encoded_length_vi64(it->first);
      sz += Serialization::encoded_length_vi64(it->second.size());
      for(auto& r : it->second)
        sz += Serialization::encoded_length_vi64(r);
    }
    return sz;
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_vi32(bufp, err);
    if(err)
      return;
    Serialization::encode_vi64(bufp, columns.size());
    for(auto it = columns.cbegin(); it != columns.cend(); ++it) {
      Serialization::encode_vi64(bufp, it->first);
      Serialization::encode_vi64(bufp, it->second.size());
      for(auto& r : it->second)
        Serialization::encode_vi64(bufp, r);
    }
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    err = Serialization::decode_vi32(bufp, remainp);
    if(err)
      return;
    cid_t cid;
    for(size_t cids=Serialization::decode_vi64(bufp, remainp); cids; --cids) {
      cid = Serialization::decode_vi64(bufp, remainp);
      auto& rids = columns[cid];
      rids.resize(Serialization::decode_vi64(bufp, remainp));
      for(auto& rid : rids)
        rid = Serialization::decode_vi64(bufp, remainp);
    }
  }

};


}}}}}

#endif // swcdb_db_protocol_rgr_params_ColumnsUnload_h
