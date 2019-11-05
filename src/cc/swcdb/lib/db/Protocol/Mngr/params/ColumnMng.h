
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_params_ColumnMng_h
#define swc_db_protocol_mngr_params_ColumnMng_h

#include "swcdb/lib/core/Serializable.h"
#include "swcdb/lib/db/Columns/Schema.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {
  

class ColumnMng  : public Serializable {
  public:

    enum Function { // corelation-sequence required
      INTERNAL_LOAD_ALL     = 0,

      INTERNAL_LOAD         = 1,
      INTERNAL_ACK_LOAD     = 2,

      CREATE                = 3,
      INTERNAL_ACK_CREATE   = 4,

      DELETE                = 5,
      INTERNAL_ACK_DELETE   = 6,

      MODIFY                = 7,
      INTERNAL_ACK_MODIFY   = 8,
    };

    ColumnMng() {}

    ColumnMng(Function function,  DB::SchemaPtr schema)
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


}}}}

#endif // swc_db_protocol_mngr_params_ColumnMng_h
