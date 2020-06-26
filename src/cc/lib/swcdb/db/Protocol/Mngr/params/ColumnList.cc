
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {

ColumnListReq::ColumnListReq() { }

ColumnListReq::~ColumnListReq() { }

size_t ColumnListReq::internal_encoded_length() const {
  return 0;
}
  
void ColumnListReq::internal_encode(uint8_t** bufp) const {
}
  
void ColumnListReq::internal_decode(const uint8_t** bufp, size_t* remainp) {
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
  for(auto i=0; i<sz; ++i) 
    schemas[i].reset(new DB::Schema(bufp, remainp));
}


}}}}
