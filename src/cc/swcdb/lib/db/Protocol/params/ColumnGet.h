
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_ColumnGet_h
#define swc_db_protocol_params_ColumnGet_h

#include "swcdb/lib/core/Serializable.h"
#include "swcdb/lib/db/Columns/Schema.h"

namespace SWC {
namespace Protocol {
namespace Params {


class ColumnGetReq  : public Serializable {
  public:

  enum Flag {
    SCHEMA_BY_ID    = 0x0,
    SCHEMA_BY_NAME  = 0x1,
    ID_BY_NAME      = 0x2
  };

  ColumnGetReq() {}

  ColumnGetReq(Flag flag,  std::string& name)
              : flag(flag), name(name) {}

  ColumnGetReq(Flag flag,  int64_t cid)
              : flag(flag), cid(cid) {}

  Flag        flag;
  std::string name;
  int64_t     cid {};

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      return 1 +  
            (flag == Flag::SCHEMA_BY_ID 
            ? Serialization::encoded_length_vi64(cid)
            : Serialization::encoded_length_vstr(name.length()));
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_i8(bufp, (uint8_t)flag);

      if(flag == Flag::SCHEMA_BY_ID)
        Serialization::encode_vi64(bufp, cid);
      else
        Serialization::encode_vstr(bufp, name.data(), name.length());
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      flag = (Flag)Serialization::decode_i8(bufp, remainp);

      if(flag == Flag::SCHEMA_BY_ID)
        cid = Serialization::decode_vi64(bufp, remainp);
      else
        name = Serialization::decode_vstr(bufp, remainp);
    }

};



class ColumnGetRsp  : public Serializable {
  public:

  ColumnGetRsp() {}

  ColumnGetRsp(ColumnGetReq::Flag flag, DB::SchemaPtr schema)
              : flag(flag), schema(schema) {     
  }

  ColumnGetReq::Flag  flag;
  DB::SchemaPtr       schema = nullptr;
  int64_t             cid {};

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      return 1 + 
            (flag == ColumnGetReq::Flag::ID_BY_NAME
            ? Serialization::encoded_length_vi64(schema->cid)
            : schema->encoded_length());
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_i8(bufp, (uint8_t)flag);
      if(flag == ColumnGetReq::Flag::ID_BY_NAME)
        Serialization::encode_vi64(bufp, schema->cid);
      else
        schema->encode(bufp);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      flag = (ColumnGetReq::Flag)Serialization::decode_i8(bufp, remainp);
       if(flag == ColumnGetReq::Flag::ID_BY_NAME)
        cid = Serialization::decode_vi64(bufp, remainp);
      else
        schema = std::make_shared<DB::Schema>(bufp, remainp);
    }

};

}}}

#endif // swc_db_protocol_params_ColumnGetRsp_h
