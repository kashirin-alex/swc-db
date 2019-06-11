/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_include_ScanSpecs_h
#define swcdb_include_ScanSpecs_h

#include "swcdb/lib/db/Comparators/Comparators.h"

#include <cstring>
#include <ostream>
#include <vector>


namespace SWC {
namespace ScanSpecs {

  enum LimitType{
    KEYS,
  };
  
  static const int64_t TIMESTAMP_MIN  = INT64_MIN;
  static const int64_t TIMESTAMP_MAX  = INT64_MAX;
  static const int64_t TIMESTAMP_NULL = INT64_MIN + 1;
  static const int64_t TIMESTAMP_AUTO = INT64_MIN + 2;
  static const int64_t AUTO_ASSIGN    = INT64_MIN + 2;

  static const uint32_t FLAG_DELETE_ROW            = 0x00;
  static const uint32_t FLAG_DELETE_COLUMN_FAMILY  = 0x01;
  static const uint32_t FLAG_DELETE_CELL           = 0x02;
  static const uint32_t FLAG_DELETE_CELL_VERSION   = 0x03;
  enum {
    KEYSPEC_DELETE_MAX = 4
  };
  static const uint32_t FLAG_INSERT                = 0xFF;

     
  class Key {
    public:
    Key():key(0), len(0), comp(Comparator::NONE){}
    Key(const char* k, Comparator c): key(k), len(strlen(k)), comp(c){}
    Key(const char* k, const uint32_t len, Comparator c):  key(k), len(len), comp(c){}
    Key operator=(Key &other){
      key = other.key;
      len = other.len;
      comp = other.comp;
      return *this; 
    }
    virtual ~Key(){}

    const char*  key;
    uint32_t     len;    
    Comparator   comp;
    
    bool is_matching(const char **other, const uint32_t other_len){
      if(matcher == nullptr) matcher = matcher->get_matcher(comp);
      return matcher->is_matching(&key, len, other, other_len);
    }

    private:
    ComparatorBase* matcher = {};
  };
  typedef std::vector<Key> ListKeys;



  class Keys {
    public:
    Keys() {}
    Keys(ListKeys ks) : keys(ks) {}
    Keys operator=(Keys &other){
      keys.assign(other.keys.begin(), other.keys.end());
      return *this; 
    }
    virtual ~Keys(){}

    ListKeys   keys;
  };
  std::ostream &operator<<(std::ostream &os, const Keys &keys){
    if(keys.keys.size()>0){
      os << "Keys:[";
      for(auto it=keys.keys.begin(); it < keys.keys.end();++it)
        os << "{key:\"" << (*it).key << "\",comp:\"" <<  (*it).comp << "\"}";
      os << "]";
    }
    return os;
  }
  
     

  class Value {
    public:
    Value():value(0), len(0), comp(Comparator::NONE){}
    Value(const char* v, Comparator c): value(v), len(strlen(v)), comp(c){}
    Value(const char* v, const uint32_t len, Comparator c): value(v), len(len), comp(c){}
    Value operator=(Value &other){
      value = other.value;
      len = other.len;
      comp = other.comp;
      return *this; 
    }
    virtual ~Value(){}

    const char*  value;
    uint32_t     len;    
    Comparator   comp;

    bool is_matching(const char **other, const uint32_t other_len){
      if(matcher == nullptr) matcher = matcher->get_matcher(comp);
      return matcher->is_matching(&value, len, other, other_len);
    }
    /*
    bool is_matching_counter(int64_t other){
      if(matcher == nullptr) matcher = matcher->get_matcher(comp);
      return matcher->is_matching(&value, other);
    }
    */
    private:
    ComparatorBase* matcher = {};
  };


     
  class Timestamp {
    public:
    Timestamp(): ts(0), comp(Comparator::NONE){}
    Timestamp(int64_t timestamp, Comparator c): ts(timestamp), comp(c){}
    Timestamp operator=(Timestamp &other){
      ts = other.ts;
      comp = other.comp;
      return *this; 
    }
    virtual ~Timestamp(){}

    int64_t     ts; 
    Comparator  comp;

    bool is_matching(int64_t other){
      if(matcher == nullptr) matcher = matcher->get_matcher(comp);
      return matcher->is_matching(ts, other);
    }
    void reset_matcher(){
      matcher = {};
    }
    private:
    ComparatorBase* matcher = {};
  };
  std::ostream &operator<<(std::ostream &os, const Timestamp &ts){
    os << "Timestamp:{ts:" << ts.ts << ",comp:\"" << ts.comp << "\"}";
    return os;
  }



  class Flags {
    public:
    Flags():
       limit(0), offset(0), max_versions(0), 
       limit_by(LimitType::KEYS), offset_by(LimitType::KEYS),
       return_deletes(false), keys_only(false) {}
    Flags operator=(Flags &other){
      limit = other.limit;
      offset = other.offset;
      max_versions = other.max_versions;
      limit_by = other.limit_by;
      offset_by = other.offset_by;
      return_deletes = other.return_deletes;
      keys_only = other.keys_only;
      return *this; 
    }
    virtual ~Flags(){}
    
    uint32_t 	limit, offset, max_versions;
    LimitType limit_by, offset_by;
    bool 	 	  return_deletes, keys_only;
  };
  std::ostream &operator<<(std::ostream &os, const Flags &flags){
    os << "Flags:{";
    if(flags.limit)
      os << "limit:" << flags.limit << "," 
            "limit_by:" << flags.limit_by << ",";
    if(flags.offset)
      os << "offset:" << flags.offset << "," 
            "offset_by:" << flags.offset_by << ",";
    if(flags.offset)
    if(flags.max_versions)
      os << "max_versions:" << flags.max_versions << ",";
     
    os << "return_deletes:" << flags.return_deletes << ",";
    os << "keys_only:" << flags.keys_only << ",";
    os << "}";
    return os;
  }



  class CellsInterval {
    public:
    CellsInterval() {}
    CellsInterval(Keys k_s, Keys k_f, Value v, 
                  Timestamp ts_s, Timestamp ts_f):
                  keys_start(k_s), keys_finish(k_f), value(v),
                  ts_start(ts_s), ts_finish(ts_f) {}
    CellsInterval(Keys k_s, Keys k_f, Value v, 
                  Timestamp ts_s, Timestamp ts_f, Flags f):
                  keys_start(k_s), keys_finish(k_f), value(v),
                  ts_start(ts_s), ts_finish(ts_f), flags(f) {}
    CellsInterval operator=(CellsInterval &other){
      keys_start = other.keys_start; 
      keys_finish = other.keys_finish;
      value = other.value;
      ts_start = other.ts_start;
      ts_finish = other.ts_finish;
      flags = other.flags;
      return *this; 
    }
    virtual ~CellsInterval(){}

    Keys       keys_start, keys_finish;
    Value      value;
    Timestamp  ts_start, ts_finish;
    Flags      flags;
  };
  std::ostream &operator<<(std::ostream &os, const CellsInterval &cs_i){
    os << "{CellsInterval:{";
    if(cs_i.keys_start.keys.size()>0)
      os << "start_" << cs_i.keys_start << ",";
    
    if(cs_i.keys_finish.keys.size()>0)
      os << "finish_" << cs_i.keys_finish << ",";
  
    if(cs_i.value.comp != Comparator::NONE)
      os << "Value:{data:\"" << cs_i.value.value << "\",comp:\"" <<  cs_i.value.comp << "\"}";
  
    if(cs_i.ts_start.comp != Comparator::NONE)
      os << "start_"  << cs_i.ts_start << ",";
    if(cs_i.ts_finish.comp != Comparator::NONE)
      os << "finish_"  << cs_i.ts_finish << ",";
    
    os << cs_i.flags;
    os << "}}";
    return os;
  }
  typedef std::vector<CellsInterval> ListCellsInterval;



  class ColumnIntervals {
    public:
    ColumnIntervals(int64_t col_id): cid(col_id) {}
    virtual ~ColumnIntervals(){}

    ColumnIntervals operator=(ColumnIntervals &other){
      cells_interval.assign(
        other.cells_interval.begin(), other.cells_interval.end()); 
      cid = other.cid;
      return *this; 
    }

    int64_t cid;
    ListCellsInterval cells_interval;
  };
  std::ostream &operator<<(std::ostream &os, const ColumnIntervals &cs_is){
    os << "{CID:" << cs_is.cid << ",";
    os << "ColumnIntervals:[";
    for(auto it=cs_is.cells_interval.begin(); 
        it < cs_is.cells_interval.end();++it)
      os << (*it) << ",";
    os << "]}";
    return os;
  }
  typedef std::vector<ColumnIntervals> ListColumns;



  class ScanSpec {
    public:
    ScanSpec() {}
       
    ScanSpec operator=(ScanSpec &other){
      columns.assign(other.columns.begin(), other.columns.end());
      flags = other.flags;
      return *this; 
    }
    virtual ~ScanSpec(){}
  
    ListColumns columns;
    Flags      	flags;
  };
  std::ostream &operator<<(std::ostream &os, const ScanSpec &ss){
    os << "{ScanSpec:{";

    os << "Columns:[";
    for(auto it=ss.columns.begin();it<ss.columns.end();++it)
      os << (*it) << ",";
    os << "],";

    os << ss.flags;
    os << "}}";
    return os;
  }



}
}


#endif