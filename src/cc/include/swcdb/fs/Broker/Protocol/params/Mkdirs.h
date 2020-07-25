/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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

  size_t internal_encoded_length() const {
  return Serialization::encoded_length_bytes(dirname.size());
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_bytes(bufp, dirname.c_str(), dirname.size());
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    dirname.clear();
    dirname.append(Serialization::decode_bytes_string(bufp, remainp));
  }

};

}}}}

#endif // swc_fs_Broker_Protocol_params_Mkdirs_h
