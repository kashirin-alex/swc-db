/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_ReadAll_h
#define swc_fs_Broker_Protocol_params_ReadAll_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class ReadAllReq : public Serializable {
  public:

  ReadAllReq() {}

  ReadAllReq(const std::string& name)
            : name(name) {}

  std::string name;

  private:

  size_t internal_encoded_length() const {
      return Serialization::encoded_length_vstr(name);
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_vstr(bufp, name);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    name = Serialization::decode_vstr(bufp, remainp);
  }
  
};



}}}}

#endif // swc_fs_Broker_Protocol_params_ReadAll_h
