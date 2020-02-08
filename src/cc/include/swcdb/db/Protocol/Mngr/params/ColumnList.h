
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_params_ColumnList_h
#define swc_db_protocol_mngr_params_ColumnList_h

#include "swcdb/core/Serializable.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class ColumnListReq  : public Serializable {
  public:

  ColumnListReq() {}

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    return 0;
  }
    
  void encode_internal(uint8_t **bufp) const {
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
  }

};



class ColumnListRsp  : public Serializable {
  public:

  ColumnListRsp() {}

  std::vector<DB::Schema::Ptr> schemas;

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    size_t sz = Serialization::encoded_length_vi64(schemas.size());
    for (auto schema : schemas)
      sz += schema->encoded_length();
    return sz;
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, schemas.size());
    for(auto schema : schemas)
      schema->encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    size_t sz = Serialization::decode_vi64(bufp, remainp);
    schemas.clear();
    schemas.resize(sz);
    for(auto i=0;i<sz;++i) 
      schemas[i].reset(new DB::Schema(bufp, remainp));
  }

};

}}}}

#endif // swc_db_protocol_params_ColumnListRsp_h
