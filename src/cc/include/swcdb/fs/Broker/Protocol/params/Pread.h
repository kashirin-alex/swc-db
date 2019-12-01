/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_params_Pread_h
#define swc_lib_fs_Broker_Protocol_params_Pread_h

#include "swcdb/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class PreadReq : public Serializable {
  public:

  PreadReq() {}

  PreadReq(int32_t fd, uint64_t offset, uint32_t amount)
          : fd(fd), offset(offset), amount(amount) {}

  int32_t   fd;
  uint64_t  offset;
  uint32_t  amount;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 16;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, fd);
    Serialization::encode_i64(bufp, offset);
    Serialization::encode_i32(bufp, amount);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
    offset = Serialization::decode_i64(bufp, remainp);
    amount = Serialization::decode_i32(bufp, remainp);
  }
  
};


}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Pread_h
