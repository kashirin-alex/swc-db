/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/common/Serialization.h"
#include "ScanSpecs.h"


namespace SWC {

std::ostream &operator<<(std::ostream &os, const ScanSpecs::Keys &keys){
  if(keys.keys.size()>0){
    os << "keys:[";
    for(auto it=keys.keys.begin(); it < keys.keys.end();++it)
      os << "{key:\"" << (*it).key << "\",comp:\"" <<  (*it).comp << "\"},";
    os << "]";
  }
  return os;  
}

std::ostream &operator<<(std::ostream &os, const ScanSpecs::Timestamp &ts){
  os << "timestamp:{ts:" << ts.ts << ",comp:\"" << ts.comp << "\"}";
  return os;
}

std::ostream &operator<<(std::ostream &os, const ScanSpecs::Flags &flags){
  os << "flags:{"
     << "limit:"          << flags.limit << "," 
        "limit_by:"       << flags.limit_by << ","
     << "offset:"         << flags.offset << "," 
        "offset_by:"      << flags.offset_by << ","
     << "max_versions:"   << flags.max_versions << ","
     << "return_deletes:" << flags.return_deletes << ","
     << "keys_only:"      << flags.keys_only << ","
     << "}";
  return os;
}

std::ostream &operator<<(std::ostream &os, const ScanSpecs::CellsInterval &cs_i){
  os << "{cells_interval:{";

  if(cs_i.keys_start.keys.size()>0)
    os << "start_" << cs_i.keys_start << ",";
  if(cs_i.keys_finish.keys.size()>0)
    os << "finish_" << cs_i.keys_finish << ",";
  
  if(cs_i.value.comp != Comparator::NONE)
    os << "value:{data:\"" << cs_i.value.value << "\",comp:\"" <<  cs_i.value.comp << "\"},";
  
  if(cs_i.ts_start.comp != Comparator::NONE)
    os << "start_"  << cs_i.ts_start << ",";
  if(cs_i.ts_finish.comp != Comparator::NONE)
    os << "finish_"  << cs_i.ts_finish << ",";
    
  os << cs_i.flags << "}}";
  return os;
}

std::ostream &operator<<(std::ostream &os, const ScanSpecs::ColumnIntervals &cs_is){
  os << "{cid:" << cs_is.cid 
     << ",column_intervals:[";
  for(auto it=cs_is.cells_interval.begin(); 
      it < cs_is.cells_interval.end();++it)
    os << (*it) << ",";
  os << "]}";
  return os;
}

std::ostream &operator<<(std::ostream &os, const ScanSpecs::ScanSpec &ss){
  os << "{scan_spec:{columns:[";
  for(auto it=ss.columns.begin();it<ss.columns.end();++it)
    os << (*it) << ",";
  os << "],"
    << ss.flags 
    << "}}";
  return os;
}

using namespace ScanSpecs;


// KEY: encode, encoded_length, decode
void Key::encode(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)comp);
  if(comp != Comparator::NONE)
    Serialization::encode_vstr(bufp, key, len);
}
size_t Key::encoded_length() const {
  return 1+(comp!=Comparator::NONE?Serialization::encoded_length_vstr(len):0);
}
void Key::decode(const uint8_t **bufp, size_t *remainp){
  comp = (Comparator)Serialization::decode_i8(bufp, remainp);
  if(comp != Comparator::NONE)
   key  = Serialization::decode_vstr(bufp, remainp, &len);
}



// KEYS: encode, encoded_length, decode
void Keys::encode(uint8_t **bufp) const {
  Serialization::encode_vi32(bufp, keys.size());
  for(auto it=keys.begin();it<keys.end();++it)
    (*it).encode(bufp);
}
size_t Keys::encoded_length() const {
  size_t len = Serialization::encoded_length_vi32(keys.size());
  for(auto it=keys.begin();it<keys.end();++it)
    len += (*it).encoded_length(); 
  return len;
}
void Keys::decode(const uint8_t **bufp, size_t *remainp){
  for (size_t i = Serialization::decode_vi32(bufp, remainp); i--;)
    keys.push_back(Key(bufp, remainp));
}



// VALUE: encode, encoded_length, decode
void Value::encode(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)comp);
  if(comp != Comparator::NONE)
    Serialization::encode_vstr(bufp, value, len);
}
size_t Value::encoded_length() const {
  return 1+(comp!=Comparator::NONE?Serialization::encoded_length_vstr(len):0);
}
void Value::decode(const uint8_t **bufp, size_t *remainp){
  comp = (Comparator)Serialization::decode_i8(bufp, remainp);
  if(comp != Comparator::NONE)
    value = Serialization::decode_vstr(bufp, remainp, &len);
}



// TIMESTAMP: encode, encoded_length, decode
void Timestamp::encode(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)comp);
  if(comp != Comparator::NONE)
    Serialization::encode_i64(bufp, ts);
}
size_t Timestamp::encoded_length() const {
  return 1+(comp!=Comparator::NONE?8:0);
}
void Timestamp::decode(const uint8_t **bufp, size_t *remainp){
  comp = (Comparator)Serialization::decode_i8(bufp, remainp);
  if(comp != Comparator::NONE)
    ts = Serialization::decode_i64(bufp, remainp);
}



// FLAGS: encode, encoded_length, decode
void Flags::encode(uint8_t **bufp) const {
  Serialization::encode_i8(bufp, (uint8_t)limit_by);
  Serialization::encode_vi32(bufp, limit);
  Serialization::encode_i8(bufp, (uint8_t)offset_by);
  Serialization::encode_vi32(bufp, offset);
  Serialization::encode_vi32(bufp, max_versions);
  Serialization::encode_bool(bufp, return_deletes);
  Serialization::encode_bool(bufp, keys_only);
}
size_t Flags::encoded_length() const {
  return 4+Serialization::encoded_length_vi32(limit)
          +Serialization::encoded_length_vi32(offset)
          +Serialization::encoded_length_vi32(max_versions);
}
void Flags::decode(const uint8_t **bufp, size_t *remainp){
  limit_by = (LimitType)Serialization::decode_i8(bufp, remainp);
  limit = Serialization::decode_vi32(bufp, remainp);
  offset_by = (LimitType)Serialization::decode_i8(bufp, remainp);
  offset = Serialization::decode_vi32(bufp, remainp);
  max_versions = Serialization::decode_vi32(bufp, remainp);
  return_deletes = Serialization::decode_bool(bufp, remainp);
  keys_only = Serialization::decode_bool(bufp, remainp);
}



// CELLS_INTERVAL: encode, encoded_length, decode
void CellsInterval::encode(uint8_t **bufp) const {
  keys_start.encode(bufp);
  keys_finish.encode(bufp);
  value.encode(bufp);
  ts_start.encode(bufp);
  ts_finish.encode(bufp);
  flags.encode(bufp);
}
size_t CellsInterval::encoded_length() const {
  return keys_start.encoded_length()
        + keys_finish.encoded_length()
        + value.encoded_length()
        + ts_start.encoded_length()
        + ts_finish.encoded_length()
        + flags.encoded_length();
}
void CellsInterval::decode(const uint8_t **bufp, size_t *remainp){
  keys_start.decode(bufp, remainp);
  keys_finish.decode(bufp, remainp);
  value.decode(bufp, remainp);
  ts_start.decode(bufp, remainp);
  ts_finish.decode(bufp, remainp);
  flags.decode(bufp, remainp);
}



// COLUMN_INTERVALS, Serializable: encode, encoded_length, decode
uint8_t ColumnIntervals::encoding_version() const {
  return 1;
}
size_t ColumnIntervals::encoded_length_internal() const {
  size_t len = Serialization::encoded_length_vi32(cells_interval.size())
              +Serialization::encoded_length_vi64(cid);
  for(auto it=cells_interval.begin();it<cells_interval.end();++it)
    len += (*it).encoded_length(); 
  return len;
}
void ColumnIntervals::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi64(bufp, cid);
  Serialization::encode_vi32(bufp, (uint32_t)cells_interval.size());
  for(auto it=cells_interval.begin();it<cells_interval.end();++it)
    (*it).encode(bufp);
}
void ColumnIntervals::decode_internal(uint8_t version, const uint8_t **bufp,
			 size_t *remainp){
  cid = Serialization::decode_vi64(bufp, remainp);
  for (size_t i = Serialization::decode_vi32(bufp, remainp); i--;) 
    cells_interval.push_back(CellsInterval(bufp, remainp));
}



}