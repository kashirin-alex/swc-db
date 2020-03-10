/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/db/Cells/SpecsScan.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {

Scan::Scan(uint32_t reserve): columns(0) {
  columns.reserve(reserve);
}

Scan::Scan(Columns& columns): columns(columns) {}
     
Scan::Scan(const uint8_t **bufp, size_t *remainp) {
  decode_internal(encoding_version(), bufp, remainp); 
}

void Scan::copy(const Scan &other) {
  free();
  columns.resize(other.columns.size());
  int i = 0;
  for(const auto& col : other.columns)
    columns[i++] = Column::make_ptr(col);
  flags.copy(other.flags);
}

Scan::~Scan() {
  free();
}

void Scan::free() {
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

bool Scan::equal(const Scan &other) {
  if(columns.size() != other.columns.size())
    return false;

  auto it2=other.columns.begin();
  for(auto it1=columns.begin(); it1 < columns.end(); it1++, it2++)
    if(!(*it1)->equal(*(*it2)))
      return false;
  return true;
}

uint8_t Scan::encoding_version() const {
  return 1;
}

size_t Scan::encoded_length_internal() const {
  size_t len = Serialization::encoded_length_vi32(columns.size());
  for(auto& col : columns)
    len += col->encoded_length_internal(); 
  return len + flags.encoded_length();
}

void Scan::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi32(bufp, (uint32_t)columns.size());
  for(auto& col : columns)
    col->encode_internal(bufp);
  flags.encode(bufp);
}

void Scan::decode_internal(uint8_t version, const uint8_t **bufp,
	                		     size_t *remainp){
  uint32_t sz = Serialization::decode_vi32(bufp, remainp);
  free();
  columns.resize(sz);
  for(auto i=0;i<sz;++i)
    columns[i] = Column::make_ptr(bufp, remainp);
  flags.decode(bufp, remainp);
}

const std::string Scan::to_string() {  
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

void Scan::display(std::ostream& out, bool pretty, std::string offset) const {
  out << offset << "SpecsScan(\n"
      << offset << " columns=[\n"; 
  for(auto& col : columns)
    col->display(out, pretty, "  ");
  out << offset << " ]\n";
  
  out << offset << " Flags(";
  flags.display(out);
  out << ")\n"; 
  out << offset << ")\n"; 
}


}}}