/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_include_ScanSpecs_h
#define swcdb_include_ScanSpecs_h

#include "swcdb/lib/core/Serializable.h"
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
    Key(const uint8_t **bufp, size_t *remainp) {
       decode(bufp, remainp); 
    }
    Key operator=(Key &other){
      key = other.key;
      len = other.len;
      comp = other.comp;
      return *this; 
    }
    virtual ~Key(){}

    size_t encoded_length() const;
    void encode(uint8_t **bufp) const;
    void decode(const uint8_t **bufp, size_t *remainp);

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
    Keys(): keys(0) {}
    Keys(ListKeys ks) : keys(ks) {}
    Keys operator=(Keys &other){
      set(other.keys);
      return *this; 
    }
    void set(ListKeys& other) {
      keys.assign(other.begin(), other.end());
    }
    virtual ~Keys(){}

    size_t encoded_length(int skip=0) const;
    void encode(uint8_t **bufp, int skip=0) const;
    void decode(const uint8_t **bufp, size_t *remainp, int reserve=0);

    const std::string to_string();

    ListKeys   keys;
  };
  
     

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

    size_t encoded_length() const;
    void encode(uint8_t **bufp) const;
    void decode(const uint8_t **bufp, size_t *remainp);

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

    size_t encoded_length() const;
    void encode(uint8_t **bufp) const;
    void decode(const uint8_t **bufp, size_t *remainp);

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

    size_t encoded_length() const;
    void encode(uint8_t **bufp) const;
    void decode(const uint8_t **bufp, size_t *remainp);
    
    uint32_t 	limit, offset, max_versions;
    LimitType limit_by, offset_by;
    bool 	 	  return_deletes, keys_only;
  };



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
    CellsInterval(const uint8_t **bufp, size_t *remainp) {
       decode(bufp, remainp); 
    }
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

    size_t encoded_length() const;
    void encode(uint8_t **bufp) const;
    void decode(const uint8_t **bufp, size_t *remainp);

    Keys       keys_start, keys_finish;
    Value      value;
    Timestamp  ts_start, ts_finish;
    Flags      flags;
  };
  typedef std::vector<CellsInterval> ListCellsInterval;



  class ColumnIntervals : public Serializable {
    public:
    ColumnIntervals(): cid(0), cells_interval(0) {}

    ColumnIntervals(int64_t col_id): cid(col_id), cells_interval(0) {}

    ColumnIntervals(int64_t col_id, ListCellsInterval& cells_interval)
                    : cid(col_id), cells_interval(cells_interval) {}
                    
    ColumnIntervals(int64_t col_id, ListCellsInterval cells_interval)
                    : cid(col_id), cells_interval(cells_interval) {}

    ColumnIntervals(const uint8_t **bufp, size_t *remainp)
                    : cells_interval(0) {
       decode(bufp, remainp); 
    }
    ColumnIntervals operator=(ColumnIntervals &other){
      cells_interval.assign(
        other.cells_interval.begin(), other.cells_interval.end()); 
      cid = other.cid;
      return *this; 
    }
    virtual ~ColumnIntervals(){}

    int64_t cid;
    ListCellsInterval cells_interval;

    size_t encoded_length_internal() const override;
    uint8_t encoding_version() const override;
    void encode_internal(uint8_t **bufp) const override;
    void decode_internal(uint8_t version, const uint8_t **bufp,
			 size_t *remainp) override;
  };
  typedef std::vector<ColumnIntervals> ListColumns;
  


  class ScanSpec {
    public:
    ScanSpec(): columns(0) {}
    ScanSpec(ListColumns& columns): columns(columns) {}
       
    ScanSpec operator=(ScanSpec &other){
      columns.assign(other.columns.begin(), other.columns.end());
      flags = other.flags;
      return *this; 
    }
    virtual ~ScanSpec(){}

    ListColumns columns;
    Flags      	flags;
  };



} // END namespace ScanSpecs


std::ostream &operator<<(std::ostream &os, const ScanSpecs::ListKeys &keys);
std::ostream &operator<<(std::ostream &os, const ScanSpecs::Keys &keys);
std::ostream &operator<<(std::ostream &os, const ScanSpecs::Timestamp &ts);
std::ostream &operator<<(std::ostream &os, const ScanSpecs::Flags &flags);
std::ostream &operator<<(std::ostream &os, const ScanSpecs::CellsInterval &cs_i);
std::ostream &operator<<(std::ostream &os, const ScanSpecs::ColumnIntervals &cs_is);
std::ostream &operator<<(std::ostream &os, const ScanSpecs::ScanSpec &ss);

} // namespace SWC


#endif