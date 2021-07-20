/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {




size_t ColumnListReq::internal_encoded_length() const {
  size_t sz = Serialization::encoded_length_vi32(patterns.names.size());
  sz += patterns.names.size(); // i8-pattern.comp
  for(auto& pattern : patterns.names)
    sz += Serialization::encoded_length_bytes(pattern.size());
  ++sz;
  if(patterns.tags.comp != Condition::NONE || !patterns.tags.empty()) {
    sz += Serialization::encoded_length_vi32(patterns.tags.size());
    sz += patterns.tags.size(); // i8-tag.comp
    for(auto& tag : patterns.tags)
      sz += Serialization::encoded_length_bytes(tag.size());
  }
  return sz;
}

void ColumnListReq::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, patterns.names.size());
  for(auto& pattern : patterns.names) {
    Serialization::encode_i8(bufp, pattern.comp);
    Serialization::encode_bytes(bufp, pattern.c_str(), pattern.size());
  }

  Serialization::encode_i8(
    bufp,
    patterns.tags.comp == Condition::NONE && !patterns.tags.empty()
      ? Condition::EQ
      : patterns.tags.comp
  );
  if(patterns.tags.comp != Condition::NONE || !patterns.tags.empty()) {
    Serialization::encode_vi32(bufp, patterns.tags.size());
    for(auto& tag : patterns.tags) {
      Serialization::encode_i8(bufp, tag.comp);
      Serialization::encode_bytes(bufp, tag.c_str(), tag.size());
    }
  }
}

void ColumnListReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
  if(uint32_t sz = Serialization::decode_vi32(bufp, remainp)) {
    patterns.names.reserve(sz);
    for(; sz; --sz) {
      auto comp = Condition::Comp(Serialization::decode_i8(bufp, remainp));
      patterns.names.emplace_back(
        comp, Serialization::decode_bytes_string(bufp, remainp));
    }
  }

  patterns.tags.comp = Condition::Comp(Serialization::decode_i8(bufp, remainp));
  if(patterns.tags.comp != Condition::NONE) {
    if(uint32_t sz = Serialization::decode_vi32(bufp, remainp)) {
      patterns.tags.reserve(sz);
      for(; sz; --sz) {
        auto comp = Condition::Comp(Serialization::decode_i8(bufp, remainp));
        patterns.tags.emplace_back(
          comp, Serialization::decode_bytes_string(bufp, remainp));
      }
    }
  }
}





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
  if(size_t sz = Serialization::decode_vi64(bufp, remainp)) {
    schemas.reserve(sz);
    for(; sz; --sz)
      schemas.emplace_back(new DB::Schema(bufp, remainp));
  }
}


}}}}}
