
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_mngr_params_ColumnUpdate_h
#define swcdb_manager_Protocol_mngr_params_ColumnUpdate_h

#include "swcdb/core/comm/Serializable.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class ColumnUpdate : public Serializable {
  public:

  ColumnUpdate() {}

  ColumnUpdate(ColumnMng::Function function, 
               const DB::Schema::Ptr& schema, int err) 
              : function(function), schema(schema), err(err) {
  }

  ColumnUpdate(ColumnMng::Function function, 
               const std::vector<cid_t>& columns)
              : function(function), columns(columns), err(Error::OK) {
  }

  void print(std::ostream& out) {
    out << "ColumnUpdate(func=" << int(function);

    if(function == ColumnMng::Function::INTERNAL_EXPECT) {
      out << " columns=[";
      for(cid_t cid : columns)
        out << cid << ',';
      out << ']';
  
    } else {
      schema->print(out << ' ');
      Error::print(out << ' ', err);
    }
    out << ')';
  }
  
  ColumnMng::Function function;
  std::vector<cid_t>  columns;
  DB::Schema::Ptr     schema;
  int                 err;

  private:

  size_t internal_encoded_length() const {
    size_t sz = 1;

    if(function == ColumnMng::Function::INTERNAL_EXPECT) {
      sz += Serialization::encoded_length_vi64(columns.size());
      for(const cid_t cid : columns)
        sz += Serialization::encoded_length_vi64(cid);

    } else {
      sz += schema->encoded_length() 
          + Serialization::encoded_length_vi32(err);
    }
    return sz;
  }
    
  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)function);

    if(function == ColumnMng::Function::INTERNAL_EXPECT) {
      Serialization::encode_vi64(bufp, columns.size());
      for(const cid_t cid : columns)
        Serialization::encode_vi64(bufp, cid);

    } else {
      schema->encode(bufp);
      Serialization::encode_vi32(bufp, err);
    }
  }
    
  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    function = (ColumnMng::Function)Serialization::decode_i8(bufp, remainp);
    
    if(function == ColumnMng::Function::INTERNAL_EXPECT) {
      columns.clear();
      columns.resize(Serialization::decode_vi64(bufp, remainp));
      for(auto it = columns.begin(); it < columns.end(); ++it)
        *it = Serialization::decode_vi64(bufp, remainp);
  
    } else {
      schema = std::make_shared<DB::Schema>(bufp, remainp);
      err = Serialization::decode_vi32(bufp, remainp);
    }
  }

};
  

}}}}}

#endif // swcdb_manager_Protocol_mngr_params_ColumnUpdate_h
