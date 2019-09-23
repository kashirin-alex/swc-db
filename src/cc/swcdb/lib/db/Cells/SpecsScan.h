/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsScan_h
#define swcdb_db_cells_SpecsScan_h

#include "SpecsColumn.h"


namespace SWC { namespace DB { namespace Specs {


class Scan : public Serializable {
  public:

  typedef std::vector<Column::Ptr> Columns;

  explicit Scan(uint32_t reserve=0): columns(0) {
    columns.reserve(reserve);
  }

  explicit Scan(Columns& columns): columns(columns) {}
       
  explicit Scan(const uint8_t **bufp, size_t *remainp) {
    decode_internal(encoding_version(), bufp, remainp); 
  }

  void copy(const Scan &other) {
    //std::cout  << " copy(const Scan &other)\n";
    free();
    columns.reserve(other.columns.size());
    for(const auto& col : other.columns)
      columns.push_back(Column::make_ptr(col));
    flags.copy(other.flags);
  }
  
  virtual ~Scan(){
    //std::cout << " ~Scan \n";
    free();
  }

  void free() {
    columns.clear();
    /*
    while(!columns.empty()){
      auto it = columns.begin();
      auto p = *it;
      columns.erase(it);
      auto col = *p; //delete p;
    }
    */
  }

  bool equal(const Scan &other) {
    if(columns.size() != other.columns.size())
      return false;

    auto it2=other.columns.begin();
    for(auto it1=columns.begin(); it1 < columns.end(); it1++, it2++)
      if(!(*it1)->equal(*(*it2)))
        return false;
    return true;
  }

  uint8_t encoding_version() const {
    return 1;
  }
  
  size_t encoded_length_internal() const {
    size_t len = Serialization::encoded_length_vi32(columns.size());
    for(auto& col : columns)
      len += col->encoded_length_internal(); 
    return len + flags.encoded_length();
  }

  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, (uint32_t)columns.size());
    for(auto& col : columns)
      col->encode_internal(bufp);
    flags.encode(bufp);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
	                		size_t *remainp){
    uint32_t sz = Serialization::decode_vi32(bufp, remainp);
    free();
    columns.reserve(sz);
    for (auto i=sz;i--;)
      columns.push_back(Column::make_ptr(bufp, remainp));
    flags.decode(bufp, remainp);
  }
  
  const std::string to_string() {
    std::string s("Scan(columns=[");
    for(auto& col : columns){
      s.append(col->to_string());
      s.append(", ");
    }
    s.append("], flags=");
    s.append(flags.to_string());
    s.append(")");
    return s;
  }

  Columns   columns;
  Flags     flags;
};

}}}

#endif