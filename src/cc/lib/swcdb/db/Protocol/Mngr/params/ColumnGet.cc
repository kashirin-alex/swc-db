
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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

size_t ColumnGetReq::internal_encoded_length() const {
  return 1 +  
        (flag == Flag::SCHEMA_BY_ID 
        ? Serialization::encoded_length_vi64(cid)
        : Serialization::encoded_length_bytes(name.size()));
}

void ColumnGetReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)flag);

  if(flag == Flag::SCHEMA_BY_ID)
    Serialization::encode_vi64(bufp, cid);
  else
    Serialization::encode_bytes(bufp, name.c_str(), name.size());
}

void ColumnGetReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flag = (Flag)Serialization::decode_i8(bufp, remainp);

  if(flag == Flag::SCHEMA_BY_ID)
    cid = Serialization::decode_vi64(bufp, remainp);
  else
    name = Serialization::decode_bytes_string(bufp, remainp);
}



ColumnGetRsp::ColumnGetRsp() {}

ColumnGetRsp::ColumnGetRsp(ColumnGetReq::Flag flag, 
                           const DB::Schema::Ptr& schema)
                          : flag(flag), schema(schema) {     
}

ColumnGetRsp::~ColumnGetRsp() { }


size_t ColumnGetRsp::internal_encoded_length() const {
  return 1 + 
        (flag == ColumnGetReq::Flag::ID_BY_NAME
            ? Serialization::encoded_length_vi64(schema->cid)
        : schema->encoded_length());
}

void ColumnGetRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)flag);
  if(flag == ColumnGetReq::Flag::ID_BY_NAME)
    Serialization::encode_vi64(bufp, schema->cid);
  else
    schema->encode(bufp);
}

void ColumnGetRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flag = (ColumnGetReq::Flag)Serialization::decode_i8(bufp, remainp);
   if(flag == ColumnGetReq::Flag::ID_BY_NAME)
    cid = Serialization::decode_vi64(bufp, remainp);
  else
    schema = std::make_shared<DB::Schema>(bufp, remainp);
}


}}}}
