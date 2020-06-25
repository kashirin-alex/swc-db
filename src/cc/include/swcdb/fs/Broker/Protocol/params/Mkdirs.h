/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Mkdirs_h
#define swc_fs_Broker_Protocol_params_Mkdirs_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class MkdirsReq : public Serializable {
  public:

  MkdirsReq() {}

  MkdirsReq(const std::string& dirname) : dirname(dirname) {}

  std::string dirname;

  private:

  size_t encoded_length_internal() const {
  return Serialization::encoded_length_vstr(dirname);
  }

  void encode_internal(uint8_t** bufp) const {
    Serialization::encode_vstr(bufp, dirname);
  }

  void decode_internal(const uint8_t** bufp, size_t* remainp) {
    dirname.clear();
    dirname.append(Serialization::decode_vstr(bufp, remainp));
  }

};

}}}}

#endif // swc_fs_Broker_Protocol_params_Mkdirs_h
