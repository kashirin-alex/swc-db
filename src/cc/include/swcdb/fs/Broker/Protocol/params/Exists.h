/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_params_Exists_h
#define swc_fs_Broker_Protocol_params_Exists_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class ExistsReq : public Serializable {
  public:

  ExistsReq() {}

  ExistsReq(const std::string& fname) : fname(fname) {}

  std::string fname;

  private:

  size_t internal_encoded_length() const {
    return Serialization::encoded_length_bytes(fname.size());
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_bytes(bufp, fname.c_str(), fname.size());
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    fname.clear();
    fname.append(Serialization::decode_bytes_string(bufp, remainp));
  }

};




class ExistsRsp : public Serializable {
  public:
  
  ExistsRsp() {}

  ExistsRsp(bool exists) : exists(exists) {}

  bool exists;

  private:

  size_t internal_encoded_length() const {
    return 1;
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_bool(bufp, exists);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    exists = Serialization::decode_bool(bufp, remainp);
  }

};

}}}}

#endif // swc_fs_Broker_Protocol_params_Exists_h
