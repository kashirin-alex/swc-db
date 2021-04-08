/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {

ColumnListReq::ColumnListReq() { }

ColumnListReq::~ColumnListReq() { }

size_t ColumnListReq::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi32(patterns.size());
  for(auto& pattern : patterns)
    sz += 1 + Serialization::encoded_length_bytes(pattern.value.size());
  return sz;
}

void ColumnListReq::internal_encode(uint8_t** bufp) const {;
  Serialization::encode_vi32(bufp, patterns.size());

  for(auto& pattern : patterns) {
    Serialization::encode_i8(bufp, pattern.comp);
    Serialization::encode_bytes(
      bufp, pattern.value.c_str(), pattern.value.size());
  }
}

void ColumnListReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  uint32_t sz = Serialization::decode_vi32(bufp, remainp);
  if(!sz)
    return;
  patterns.resize(sz);
  for(auto& pattern : patterns) {
    pattern.comp = Condition::Comp(Serialization::decode_i8(bufp, remainp));
    pattern.value = Serialization::decode_bytes_string(bufp, remainp);
  }
}


ColumnListRsp::ColumnListRsp() { }

ColumnListRsp::~ColumnListRsp() { }


size_t ColumnListRsp::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi64(schemas.size());
  for (auto schema : schemas)
    sz += schema->encoded_length();
  return sz;
}

void ColumnListRsp::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, schemas.size());
  for(auto& schema : schemas)
    schema->encode(bufp);
}

void ColumnListRsp::internal_decode(const uint8_t** bufp, size_t* remainp) {
  size_t sz = Serialization::decode_vi64(bufp, remainp);
  schemas.clear();
  schemas.resize(sz);
  for(size_t i=0; i<sz; ++i)
    schemas[i].reset(new DB::Schema(bufp, remainp));
}


}}}}}
