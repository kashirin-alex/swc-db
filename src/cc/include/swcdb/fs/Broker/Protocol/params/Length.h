/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_params_Length_h
#define swc_lib_fs_Broker_Protocol_params_Length_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class LengthReq : public Serializable {
  public:

  LengthReq() {}

  LengthReq(const std::string& fname) : fname(fname) {}

  std::string fname;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
    return Serialization::encoded_length_vstr(fname);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_vstr(bufp, fname);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
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

  uint8_t encoding_version() const override {
    return 1;
  }

  size_t encoded_length_internal() const override {
    return 8;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i64(bufp, length);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
	                     size_t *remainp) override {
    length = Serialization::decode_i64(bufp, remainp);
  }
  
};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Length_h
