/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_params_Open_h
#define swc_lib_fs_Broker_Protocol_params_Open_h

#include "swcdb/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class OpenReq : public Serializable {
  public:

  OpenReq() {}

  OpenReq(const std::string& fname, uint32_t flags, int32_t bufsz)
          : fname(fname), flags(flags), bufsz(bufsz) {}

  std::string fname;
  uint32_t    flags;
  int32_t     bufsz;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
    return 8 + Serialization::encoded_length_vstr(fname);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, flags);
    Serialization::encode_i32(bufp, bufsz);
    Serialization::encode_vstr(bufp, fname);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    flags = Serialization::decode_i32(bufp, remainp);
    bufsz = (int32_t)Serialization::decode_i32(bufp, remainp);
    fname.clear();
    fname.append(Serialization::decode_vstr(bufp, remainp));
  }

};




class OpenRsp : public Serializable {
  public:
  
  OpenRsp() {}

  OpenRsp(int32_t fd) : fd(fd) {}

  int32_t fd;

  private:

  uint8_t encoding_version() const override {
    return 1;
  }

  size_t encoded_length_internal() const override {
    return 4;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, fd);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
	                     size_t *remainp) override {
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
  }
  
};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Open_h
