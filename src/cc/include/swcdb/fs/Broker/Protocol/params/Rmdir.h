/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_params_Rmdir_h
#define swc_fs_Broker_Protocol_params_Rmdir_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class RmdirReq : public Serializable {
  public:

  RmdirReq() {}

  RmdirReq(const std::string& dname) : dname(dname) {}

  std::string dname;

  private:

  size_t internal_encoded_length() const {
  return Serialization::encoded_length_vstr(dname);
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_vstr(bufp, dname);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    dname.clear();
    dname.append(Serialization::decode_vstr(bufp, remainp));
  }

};

}}}}

#endif // swc_fs_Broker_Protocol_params_Rmdir_h
