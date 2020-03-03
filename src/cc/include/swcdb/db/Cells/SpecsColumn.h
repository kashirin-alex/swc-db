/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_SpecsColumn_h
#define swcdb_db_cells_SpecsColumn_h

#include <vector>
#include "swcdb/core/Serializable.h"

#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace DB { namespace Specs {


class Column : public Serializable {
  public:
  
  typedef std::vector<Interval::Ptr> Intervals;
  typedef std::shared_ptr<Column> Ptr;
  
  inline static Ptr make_ptr(int64_t cid=0, uint32_t reserve=0){
    return std::make_shared<Column>(cid, reserve);
  }

  inline static Ptr make_ptr(int64_t cid, const Intervals& intervals){
    return std::make_shared<Column>(cid, intervals);
  }

  inline static Ptr make_ptr(const uint8_t **bufp, size_t *remainp){
    return std::make_shared<Column>(bufp, remainp);
  }

  inline static Ptr make_ptr(const Column& other){
    return std::make_shared<Column>(other);
  }

  inline static Ptr make_ptr(Ptr other){
    return std::make_shared<Column>(*other.get());
  }

  explicit Column(int64_t cid=0, uint32_t reserve=0)
                  : cid(cid), intervals(0) {
    intervals.reserve(reserve);
  }

  explicit Column(int64_t cid, const Intervals& intervals)
                  : cid(cid), intervals(intervals) {}

  explicit Column(const uint8_t **bufp, size_t *remainp) {
    decode_internal(encoding_version(), bufp, remainp); 
  }

  explicit Column(const Column& other) {
    copy(other);
  }

  void copy(const Column &other) {
    cid = other.cid;
    free();
    intervals.resize(other.intervals.size());
    int i = 0;
    for(const auto& intval : other.intervals)
      intervals[i++] = Interval::make_ptr(intval);
  }

  virtual ~Column(){
    free();
  }
  void free() {
    intervals.clear();
    /*
    while(!intervals.empty()){
      auto it = intervals.begin();
      auto p = *it;
      intervals.erase(it);
      auto interval = *p; //delete p;
    }
    */
  }

  bool equal(const Column &other) {
    if(cid != other.cid || intervals.size() != other.intervals.size())
      return false;

    auto it2=other.intervals.begin();
    for(auto it1=intervals.begin(); it1 < intervals.end(); it1++, it2++)
      if(!(*it1)->equal(*(*it2)))
        return false;
    return true;
  }

  uint8_t encoding_version() const {
    return 1;
  }
  
  size_t encoded_length_internal() const {
    size_t len = Serialization::encoded_length_vi64(cid)
                + Serialization::encoded_length_vi32(intervals.size());
    for(auto& intval : intervals)
      len += intval->encoded_length(); 
    return len;
  }

  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi32(bufp, (uint32_t)intervals.size());
    for(auto& intval : intervals)
      intval->encode(bufp);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
	                		size_t *remainp){
    cid = Serialization::decode_vi64(bufp, remainp);
    uint32_t sz = Serialization::decode_vi32(bufp, remainp);
    free();
    intervals.resize(sz);
    for (auto i=0;i<sz;++i)
      intervals[i] = Interval::make_ptr(bufp, remainp);
  }
  
  const std::string to_string() {
    std::string s("Column(cid=");
    s.append(std::to_string(cid));
    s.append(" intervals=[");
    for(auto& intval : intervals){
      s.append(intval->to_string());
      s.append(", ");
    }
    s.append("])");
    return s;
  }

  void display(std::ostream& out, bool pretty=false, 
               std::string offset = "") const {
    out << offset << "Column(\n"
        << offset << " cid=" << cid << " size=" << intervals.size() << "\n"
        << offset << " intervals=[\n"; 
    for(auto& intval : intervals)
      intval->display(out, pretty, offset+"  ");
    out << offset << " ]\n"
        << offset << ")\n"; 
  }
  
  int64_t    cid;
  Intervals  intervals;
};


}}}

#endif