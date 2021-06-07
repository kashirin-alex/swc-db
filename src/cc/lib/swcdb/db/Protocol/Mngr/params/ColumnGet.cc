/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {



size_t ColumnGetReq::internal_encoded_length() const {
  return 1 +
        (flag == Flag::SCHEMA_BY_ID
        ? Serialization::encoded_length_vi64(cid)
        : Serialization::encoded_length_bytes(name.size()));
}

void ColumnGetReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, flag);

  if(flag == Flag::SCHEMA_BY_ID)
    Serialization::encode_vi64(bufp, cid);
  else
    Serialization::encode_bytes(bufp, name.c_str(), name.size());
}

void ColumnGetReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flag = Flag(Serialization::decode_i8(bufp, remainp));

  if(flag == Flag::SCHEMA_BY_ID)
    cid = Serialization::decode_vi64(bufp, remainp);
  else
    name = Serialization::decode_bytes_string(bufp, remainp);
}





size_t ColumnGetRsp::internal_encoded_length() const {
  size_t sz = 1;
  if(flag == ColumnGetReq::Flag::ID_BY_NAME)
    sz += Serialization::encoded_length_vi64(schema->cid);
  else
    sz += schema->encoded_length();
  return sz;
}

void ColumnGetRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, flag);
  if(flag == ColumnGetReq::Flag::ID_BY_NAME)
    Serialization::encode_vi64(bufp, schema->cid);
  else
    schema->encode(bufp);
}

void ColumnGetRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  flag = ColumnGetReq::Flag(Serialization::decode_i8(bufp, remainp));
   if(flag == ColumnGetReq::Flag::ID_BY_NAME)
    cid = Serialization::decode_vi64(bufp, remainp);
  else
    schema = std::make_shared<DB::Schema>(bufp, remainp);
}


}}}}}
