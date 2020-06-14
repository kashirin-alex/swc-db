
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


ColumnGetReq::ColumnGetReq() {}

ColumnGetReq::ColumnGetReq(ColumnGetReq::Flag flag, const std::string& name)
                          : flag(flag), name(name) {}

ColumnGetReq::ColumnGetReq(ColumnGetReq::Flag flag, cid_t cid)
                          : flag(flag), cid(cid) {}

ColumnGetReq::~ColumnGetReq() { }

uint8_t ColumnGetReq::encoding_version() const {
  return 1;
}

size_t ColumnGetReq::encoded_length_internal() const {
  return 1 +  
        (flag == Flag::SCHEMA_BY_ID 
        ? Serialization::encoded_length_vi64(cid)
        : Serialization::encoded_length_vstr(name.length()));
}

void ColumnGetReq::encode_internal(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)flag);

  if(flag == Flag::SCHEMA_BY_ID)
    Serialization::encode_vi64(bufp, cid);
  else
    Serialization::encode_vstr(bufp, name.data(), name.length());
}

void ColumnGetReq::decode_internal(uint8_t version, const uint8_t **bufp,
                                   size_t *remainp) {
  flag = (Flag)Serialization::decode_i8(bufp, remainp);

  if(flag == Flag::SCHEMA_BY_ID)
    cid = Serialization::decode_vi64(bufp, remainp);
  else
    name = Serialization::decode_vstr(bufp, remainp);
}



ColumnGetRsp::ColumnGetRsp() {}

ColumnGetRsp::ColumnGetRsp(ColumnGetReq::Flag flag, DB::Schema::Ptr schema)
                          : flag(flag), schema(schema) {     
}

ColumnGetRsp::~ColumnGetRsp() { }

uint8_t ColumnGetRsp::encoding_version() const {
  return 1;
}

size_t ColumnGetRsp::encoded_length_internal() const {
  return 1 + 
        (flag == ColumnGetReq::Flag::ID_BY_NAME
            ? Serialization::encoded_length_vi64(schema->cid)
        : schema->encoded_length());
}

void ColumnGetRsp::encode_internal(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)flag);
  if(flag == ColumnGetReq::Flag::ID_BY_NAME)
    Serialization::encode_vi64(bufp, schema->cid);
  else
    schema->encode(bufp);
}

void ColumnGetRsp::decode_internal(uint8_t version, const uint8_t **bufp, 
                                   size_t *remainp) {
  flag = (ColumnGetReq::Flag)Serialization::decode_i8(bufp, remainp);
   if(flag == ColumnGetReq::Flag::ID_BY_NAME)
    cid = Serialization::decode_vi64(bufp, remainp);
  else
    schema = std::make_shared<DB::Schema>(bufp, remainp);
}


}}}}
