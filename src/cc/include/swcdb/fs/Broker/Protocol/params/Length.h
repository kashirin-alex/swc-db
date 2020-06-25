/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Length_h
#define swc_fs_Broker_Protocol_params_Length_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class LengthReq : public Serializable {
  public:

  LengthReq() {}

  LengthReq(const std::string& fname) : fname(fname) {}

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




class LengthRsp : public Serializable {
  public:
  
  LengthRsp() {}

  LengthRsp(size_t length) : length(length) {}

  size_t length;

  private:

  size_t encoded_length_internal() const {
    return 8;
  }

  void encode_internal(uint8_t** bufp) const {
    Serialization::encode_i64(bufp, length);
  }

  void decode_internal(const uint8_t** bufp, size_t* remainp) {
    length = Serialization::decode_i64(bufp, remainp);
  }
  
};

}}}}

#endif // swc_fs_Broker_Protocol_params_Length_h
