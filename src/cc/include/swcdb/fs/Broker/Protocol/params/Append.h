/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_params_Append_h
#define swc_lib_fs_Broker_Protocol_params_Append_h

#include "swcdb/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class AppendReq : public Serializable {
  public:

  AppendReq() : fd(-1), flags(0) {}

  AppendReq(int32_t fd, uint8_t flags)
            : fd(fd), flags(flags) {}

  int32_t fd ;
  uint8_t flags;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 5;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, fd);
    Serialization::encode_i8(bufp, flags);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
    flags = Serialization::decode_i8(bufp, remainp);
  }
  
};




class AppendRsp : public Serializable {
  public:

  AppendRsp() {}

  AppendRsp(uint64_t offset, uint32_t amount)
           : offset(offset), amount(amount) {}

  uint64_t offset {};
  uint32_t amount {};

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 12;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i64(bufp, offset);
    Serialization::encode_i32(bufp, amount);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    offset = Serialization::decode_i64(bufp, remainp);
    amount = Serialization::decode_i32(bufp, remainp);
  }

};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Append_h
