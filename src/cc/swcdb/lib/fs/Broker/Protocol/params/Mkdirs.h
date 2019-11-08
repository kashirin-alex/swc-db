/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_params_Mkdirs_h
#define swc_lib_fs_Broker_Protocol_params_Mkdirs_h

#include "swcdb/lib/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class MkdirsReq : public Serializable {
  public:

  MkdirsReq() {}

  MkdirsReq(const std::string& dirname) : dirname(dirname) {}

  std::string dirname;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
  return Serialization::encoded_length_vstr(dirname);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_vstr(bufp, dirname);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    dirname.clear();
    dirname.append(Serialization::decode_vstr(bufp, remainp));
  }

};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Mkdirs_h
