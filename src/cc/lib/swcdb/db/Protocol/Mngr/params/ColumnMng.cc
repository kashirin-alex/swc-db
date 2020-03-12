
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {

ColumnMng::ColumnMng() {}

ColumnMng::ColumnMng(ColumnMng::Function function,  DB::Schema::Ptr schema)
                    : function(function), schema(schema) {     
}

ColumnMng::~ColumnMng() { }

uint8_t ColumnMng::encoding_version() const {
  return 1;
}

size_t ColumnMng::encoded_length_internal() const {
  return 1 + schema->encoded_length();
}

void ColumnMng::encode_internal(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)function);
  schema->encode(bufp);
}

void ColumnMng::decode_internal(uint8_t version, const uint8_t **bufp, 
                                size_t *remainp) {
  function = (Function)Serialization::decode_i8(bufp, remainp);
  schema = std::make_shared<DB::Schema>(bufp, remainp);
}


}}}}
