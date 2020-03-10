
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_params_ColumnMng_h
#define swc_db_protocol_mngr_params_ColumnMng_h


#include "swcdb/core/Serializable.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {
  

class ColumnMng : public Serializable {
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

  ColumnMng();

  ColumnMng(Function function, DB::Schema::Ptr schema);

  virtual ~ColumnMng();

  Function        function;
  DB::Schema::Ptr schema;

  private:

  uint8_t encoding_version() const;
    
  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

  };


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/ColumnMng.cc"
#endif 

#endif // swc_db_protocol_mngr_params_ColumnMng_h
