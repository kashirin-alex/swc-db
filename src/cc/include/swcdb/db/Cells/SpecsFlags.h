/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsFlags_h
#define swcdb_db_cells_SpecsFlags_h

#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/Comparators.h"


namespace SWC { namespace DB { namespace Specs {

enum LimitType{
  KEY,
};


class Flags {
  public:

  explicit Flags(): limit(0), offset(0), max_versions(0), 
                    limit_by(LimitType::KEY), offset_by(LimitType::KEY),
                    return_deletes(false), keys_only(false), was_set(false) {}

  explicit Flags(const Flags &other){
    copy(other);
  }

  void copy(const Flags &other){
    //std::cout << " copy(const Flags &other)\n";
    limit           = other.limit;
    offset          = other.offset;
    max_versions    = other.max_versions;
    limit_by        = other.limit_by;
    offset_by       = other.offset_by;
    return_deletes  = other.return_deletes;
    keys_only       = other.keys_only;
    was_set         = other.was_set;
  }

  virtual ~Flags(){
    //std::cout << " ~Flags\n";
  }

  const bool equal(const Flags &other) const {
    return  limit == other.limit && 
            offset == other.offset  && 
            max_versions == other.max_versions  && 
            limit_by == other.limit_by  && 
            offset_by == other.offset_by  && 
            return_deletes == other.return_deletes  && 
            keys_only == other.keys_only && 
            was_set == other.was_set 
            ;
  }

  const size_t encoded_length() const {
    return 4+Serialization::encoded_length_vi32(limit)
            +Serialization::encoded_length_vi32(offset)
            +Serialization::encoded_length_vi32(max_versions);
  }

  void encode(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)limit_by);
    Serialization::encode_vi32(bufp, limit);
    Serialization::encode_i8(bufp, (uint8_t)offset_by);
    Serialization::encode_vi32(bufp, offset);
    Serialization::encode_vi32(bufp, max_versions);
    Serialization::encode_bool(bufp, return_deletes);
    Serialization::encode_bool(bufp, keys_only);
  }
  
  void decode(const uint8_t **bufp, size_t *remainp){
    limit_by = (LimitType)Serialization::decode_i8(bufp, remainp);
    limit = Serialization::decode_vi32(bufp, remainp);
    offset_by = (LimitType)Serialization::decode_i8(bufp, remainp);
    offset = Serialization::decode_vi32(bufp, remainp);
    max_versions = Serialization::decode_vi32(bufp, remainp);
    return_deletes = Serialization::decode_bool(bufp, remainp);
    keys_only = Serialization::decode_bool(bufp, remainp);
  }
  
  const std::string to_string() const {
    std::string s("Flags(");
    
    s.append("limit=");
    s.append(std::to_string(limit));
    s.append(" limit_by=");
    s.append(std::to_string((uint8_t)limit_by));
    s.append(" offset=");
    s.append(std::to_string(offset));
    s.append(" offset_by=");
    s.append(std::to_string((uint8_t)offset_by));

    s.append(" max_versions=");
    s.append(std::to_string(max_versions));

    s.append(" return_deletes=");
    s.append(std::to_string(return_deletes));
    s.append(" keys_only=");
    s.append(std::to_string(keys_only));
    s.append(" was_set=");
    s.append(was_set? "TRUE" : "FALSE");
    
    return s;
  }

  uint32_t 	limit, offset, max_versions;
  LimitType limit_by, offset_by;
  bool 	 	  return_deletes, keys_only, was_set;

};


}}}
#endif