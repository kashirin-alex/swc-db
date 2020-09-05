
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_manager_Protocol_mngr_params_ColumnUpdate_h
#define swc_manager_Protocol_mngr_params_ColumnUpdate_h

#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class ColumnUpdate : public Serializable {
  public:

  ColumnUpdate() {}

  ColumnUpdate(ColumnMng::Function function, 
               const DB::Schema::Ptr& schema, int err) 
              : function(function), schema(schema), err(err) {
  }

  void print(std::ostream& out) {
    out << "ColumnUpdate(func=" << int(function);
    schema->print(out << ' ');
    Error::print(out << ' ', err);
    out << ')';
  }
  
  ColumnMng::Function   function;
  DB::Schema::Ptr       schema;
  int                   err;

  private:

  size_t internal_encoded_length() const {
    return 1
          + schema->encoded_length() 
          + Serialization::encoded_length_vi32(err);
  }
    
  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)function);
    schema->encode(bufp);
    Serialization::encode_vi32(bufp, err);
  }
    
  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    function = (ColumnMng::Function)Serialization::decode_i8(bufp, remainp);
    schema = std::make_shared<DB::Schema>(bufp, remainp);
    err = Serialization::decode_vi32(bufp, remainp);
  }

};
  

}}}}

#endif // swc_manager_Protocol_mngr_params_ColumnUpdate_h
