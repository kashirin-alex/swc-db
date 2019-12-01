/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_params_Read_h
#define swc_lib_fs_Broker_Protocol_params_Read_h

#include "swcdb/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class ReadReq : public Serializable {
  public:

  ReadReq() {}

  ReadReq(int32_t fd, uint32_t amount)
          : fd(fd), amount(amount) {}

  int32_t   fd {};
  uint32_t  amount;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 8;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, fd);
    Serialization::encode_i32(bufp, amount);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
    amount = Serialization::decode_i32(bufp, remainp);
  }
  
};



class ReadRsp : public Serializable {
  public:

  ReadRsp() {}

  ReadRsp(uint64_t offset) 
          : offset(offset) {}

  uint64_t offset;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 8;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i64(bufp, offset);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    offset = Serialization::decode_i64(bufp, remainp);
  }
  

};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Read_h
