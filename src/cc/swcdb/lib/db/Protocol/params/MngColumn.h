
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngColumn_h
#define swc_db_protocol_params_MngColumn_h

#include "swcdb/lib/core/Serializable.h"
#include "swcdb/lib/db/Columns/Schema.h"

namespace SWC {
namespace Protocol {
namespace Params {


  class MngColumn  : public Serializable {
  public:

    enum Function {
      CREATE  = 1,
      MODIFY  = 2,
      DELETE  = 3,
      INTERNAL_LOAD = 4,
      INTERNAL_ACK_LOAD = 5,
      INTERNAL_ACK_CREATE = 6,
      INTERNAL_ACK_DELETE = 7
    };

    MngColumn() {}

    MngColumn(Function function,  DB::SchemaPtr schema)
              : function(function), schema(schema) {     
    }

    Function        function;
    DB::SchemaPtr   schema;

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      return 1 + schema->encoded_length();
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_i8(bufp, (uint8_t)function);
      schema->encode(bufp);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      function = (Function)Serialization::decode_i8(bufp, remainp);
      schema = std::make_shared<DB::Schema>(bufp, remainp);
    }

  };


}}}

#endif // swc_db_protocol_params_MngColumn_h
