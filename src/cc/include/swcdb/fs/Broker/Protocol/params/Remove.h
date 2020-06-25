/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Remove_h
#define swc_fs_Broker_Protocol_params_Remove_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class RemoveReq : public Serializable {
  public:

  RemoveReq() {}

  RemoveReq(const std::string& fname) : fname(fname) {}

  std::string fname;

  private:

  size_t encoded_length_internal() const {
  return Serialization::encoded_length_vstr(fname);
  }

  void encode_internal(uint8_t** bufp) const {
    Serialization::encode_vstr(bufp, fname);
  }

  void decode_internal(const uint8_t** bufp, size_t* remainp) {
    fname.clear();
    fname.append(Serialization::decode_vstr(bufp, remainp));
  }

};

}}}}

#endif // swc_fs_Broker_Protocol_params_Remove_h
