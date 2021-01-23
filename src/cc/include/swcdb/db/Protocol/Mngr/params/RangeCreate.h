
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_RangeCreate_h
#define swcdb_db_protocol_mngr_params_RangeCreate_h

#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class RangeCreateReq : public Serializable {
  public:

  RangeCreateReq(cid_t cid=0, rgrid_t rgrid=0)
                 : cid(cid), rgrid(rgrid) {
  }

  virtual ~RangeCreateReq(){ }

  cid_t       cid;
  rgrid_t     rgrid;

  std::string to_string() const {
    std::string s("RangeCreateReq(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rgrid=");
    s.append(std::to_string(rgrid));
    return s;
  }

  private:

  size_t internal_encoded_length() const override {
    return Serialization::encoded_length_vi64(cid)
      + Serialization::encoded_length_vi64(rgrid);
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rgrid);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    cid = Serialization::decode_vi64(bufp, remainp);
    rgrid = Serialization::decode_vi64(bufp, remainp);
  }

};



class RangeCreateRsp : public Serializable {
  public:

  RangeCreateRsp(int err = Error::OK): err(err), rid(0) { }

  virtual ~RangeCreateRsp() {}

  int           err;
  rid_t         rid;

  std::string to_string() const {
    std::string s("RangeCreateRsp(");
    s.append("err=");
    s.append(std::to_string(err));
    if(!err) {
      s.append(" rid=");
      s.append(std::to_string(rid));
    } else {
      s.append("(");
      s.append(Error::get_text(err));
      s.append(")");
    }
    s.append(")");
    return s;
  }

  private:

  size_t internal_encoded_length() const override {
    return Serialization::encoded_length_vi32(err)
          + (err ? 0 : Serialization::encoded_length_vi64(rid));
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_vi32(bufp, err);
    if(!err)
      Serialization::encode_vi64(bufp, rid);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    err = Serialization::decode_vi32(bufp, remainp);
    if(!err)
      rid = Serialization::decode_vi64(bufp, remainp);
  }

};


}}}}}

#endif // swcdb_db_protocol_mngr_params_RangeCreate_h
