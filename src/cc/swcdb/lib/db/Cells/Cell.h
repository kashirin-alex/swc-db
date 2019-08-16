/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cell_h
#define swcdb_db_Cell_h


#include <iostream>
#include <ostream>
#include <vector>
#include <memory>
#include <cassert>

#include "swcdb/lib/core/DynamicBuffer.h"
#include "swcdb/lib/core/Serialization.h"

#include "swcdb/lib/db/ScanSpecs/ScanSpecs.h"


namespace SWC {

namespace Cells {
  
  static inline void encode_ts64(uint8_t **bufp, int64_t val, bool asc=true) {
    if (asc)
      val = ~val;
#ifdef HT_LITTLE_ENDIAN
    *(*bufp)++ = (uint8_t)(val >> 56);
    *(*bufp)++ = (uint8_t)(val >> 48);
    *(*bufp)++ = (uint8_t)(val >> 40);
    *(*bufp)++ = (uint8_t)(val >> 32);
    *(*bufp)++ = (uint8_t)(val >> 24);
    *(*bufp)++ = (uint8_t)(val >> 16);
    *(*bufp)++ = (uint8_t)(val >> 8);
    *(*bufp)++ = (uint8_t)val;
#else
    memcpy(*bufp, &val, 8);
    *bufp += 8;
#endif
  }

  static inline int64_t decode_ts64(const uint8_t **bufp,  bool asc=true) {
    int64_t val;
#ifdef HT_LITTLE_ENDIAN
    val = ((int64_t)*(*bufp)++ << 56);
    val |= ((int64_t)(*(*bufp)++) << 48);
    val |= ((int64_t)(*(*bufp)++) << 40);
    val |= ((int64_t)(*(*bufp)++) << 32);
    val |= ((int64_t)(*(*bufp)++) << 24);
    val |= (*(*bufp)++ << 16);
    val |= (*(*bufp)++ << 8);
    val |= *(*bufp)++;
#else
    memcpy(&val, *bufp, 8);
    *bufp += 8;
#endif
    return (asc ? ~val : val);
  }



  class Key {
    public:
    Key(): key(0), len(0){}
    Key(const char* k): key(k), len(strlen(k)){}
    Key(const char* k, const uint32_t l): key(k), len(l){}
    Key operator=(Key &other){
      std::cout << "other-Key\n";
      key = other.key;
      len = other.len;
      return *this; 
    }
    virtual ~Key(){}

    const char*  key;
    uint32_t     len;    
  
    static const std::string to_string(const Key &k){
      std::string s("{key:\"");
      s.append(k.key);
      s.append("\",len:\"");
      s.append(std::to_string(k.len));
      s.append("\"}");
      return s;
    }
  };
  std::ostream &operator<<(std::ostream &os, const Key &k){
    os << Key::to_string(k);
    return os;
  }
  typedef std::vector<Key> ListKeys;


  bool operator==(ListKeys &ks1, ListKeys &ks2){
    if(ks1.size() != ks1.size()) return false;
    auto it1=ks1.begin();
    for(auto it2=ks2.begin();it2<ks2.end();++it2,++it1){
      if((*it1).len != (*it2).len 
          || memcmp((*it1).key , (*it2).key , (*it1).len ) != 0)
        return false;

    }
    return true;
  }
}

namespace Serialization {
  
  inline size_t encoded_length(Cells::ListKeys &keys){
   size_t len = 0;
    for(auto k : keys)
      len += k.len+1;
    return Serialization::encoded_length_vi32(len)+len;
  }

  inline void encode(Cells::ListKeys &keys, uint8_t **ptr){
    size_t len = 0;
    for(auto k : keys)
      len += k.len+1;
    Serialization::encode_vi32(ptr, len);

    for(auto k : keys){
      memcpy(*ptr, k.key, k.len);
      *ptr += k.len;
      **ptr = 0;
      *ptr += 1;
    }
  }

  inline void decode(Cells::ListKeys &keys, uint8_t* &bufp, 
                     const uint8_t **ptr, size_t *remain){
    uint32_t len = Serialization::decode_vi32(ptr, remain);
    bufp = new uint8_t[len];
    memcpy(bufp, *ptr, len);
    *remain -= len;
    *ptr += len;

    uint8_t* tmpptr = bufp;
    const uint8_t* ptr_end = tmpptr+len;
    uint32_t num_keys = 0;
    for(uint8_t* i=tmpptr;i<ptr_end;i++) {
      if(*i == 0)
        num_keys++;
    }
    keys.reserve(num_keys);

    while (tmpptr < ptr_end){
      Cells::Key k;
      k.key = (const char *)tmpptr;
      while (*tmpptr != 0)
        *tmpptr++;
      k.len = tmpptr - (uint8_t *)k.key;
      *tmpptr++;
      keys.push_back(k);
    }
  }

}


namespace Cells {
  
  class Cell {

    static const uint8_t HAVE_REVISION      =  0x80;
    static const uint8_t HAVE_TIMESTAMP     =  0x40;
    static const uint8_t AUTO_TIMESTAMP     =  0x20;
    static const uint8_t REV_IS_TS          =  0x10;
    static const uint8_t TS_DESC            =   0x1;

    public:
    Cell(): flag(0), control(0), 
            timestamp(0), revision(0), 
            skey(0), klen(0), 
            value(0), vlen(0) { 

    }
    
    Cell(Cell* other, bool only_serial=false){
      std::cout << "other-Cell\n";
      
      if(other->skey != 0)
        other->make_skey();
        
      if(other->skey != 0) {
        klen = other->klen;
        skey = new uint8_t[klen];
        memcpy(skey, other->skey, klen);
        if(!only_serial)
          load_skey();
      }

      if(other->value != 0) {
        vlen = other->vlen;
        value = new uint8_t[vlen];
        memcpy(value, other->value, vlen);
      }
    }
    
    Cell* make_copy(bool only_serial=false){
      return new Cell(this, only_serial);
    }

    Cell operator=(Cell &other){
      std::cout << "other-Cell\n";
      return *(new Cell(&other));
    }

    bool operator==(Cell &other){
      return  control == other.control && 
              flag == other.flag && 
              timestamp == other.timestamp && 
              revision == other.revision && 
              klen == other.klen &&
              vlen == other.vlen &&
              memcmp(skey,  other.skey,  klen) == 0 && 
              memcmp(value, other.value, vlen) == 0 && 
              keys == other.keys;
    }

    virtual ~Cell(){
      std::cout << " ~Cell " 
                << ",skey-ptr:" << (size_t)skey 
                << ",value-ptr:" << (size_t)value 
                << "\n";
      if(skey!=0)
        delete [] skey;
      if(value!=0)
        delete [] value;
    }
        
    // SET_KEY_OPTIONS
    void set_flag(uint8_t f){
      flag = f;
    }
    void set_time_order_desc(bool desc){
      if(desc)  control |= TS_DESC;
      else      control != TS_DESC;
    }
    void set_timestamp(int64_t ts){
      timestamp = ts;
      control |= HAVE_TIMESTAMP;
    }
    void set_revision(uint64_t rev){
      revision = rev;
      control |= HAVE_REVISION;
    }
    
    // SET_KEYS
    void set_keys(ListKeys ks){
      keys.assign(ks.begin(), ks.end());
    }

    // SET_VALUE
    void set_value(uint8_t* v, uint32_t len){
      value = v;
      vlen = len;
    }
    void set_value(const char* v, uint32_t len){
      set_value((uint8_t *)v, len);
    }
    void set_value(const char* v){
      set_value((uint8_t *)v, strlen(v));
    }

    // READ - serialized
    void read(uint8_t **ptr, bool load=true) {
      const uint8_t *tmp_ptr = *ptr;

      klen = Serialization::decode_vi32(&tmp_ptr);
      skey = new uint8_t[klen];
      memcpy(skey, tmp_ptr, klen);
      tmp_ptr += klen;

      vlen = Serialization::decode_vi32(&tmp_ptr);
      if(vlen > 0){
        value = new uint8_t[vlen];
        memcpy(value, tmp_ptr, vlen);
        tmp_ptr += vlen;
      }
      // assert(*(++*ptr)==0);

      *ptr += tmp_ptr - *ptr;

      if(load)
        load_skey();
    }
    
    // LOAD - serialized key (skey)
    void load_skey(){
      
      const uint8_t* ptr = skey;
      const uint8_t* end_ptr = ptr + klen;

      flag = *ptr++;
      control = *ptr++;
      uint8_t num_keys = *ptr++;
      keys.reserve(num_keys);
      
      for(int i=0;i<num_keys;i++) {
        Key k;
        k.key = (const char *)ptr;
        while (ptr < end_ptr && *ptr != 0)
          *ptr++;
        k.len = ptr - (uint8_t *)k.key;
        *ptr++;
        if(ptr >= end_ptr){
          std::cerr << "key: {" << i << "} decode overrun" << std::endl;
          assert(ptr >= end_ptr);
        }
        assert(strlen(k.key) == k.len);
        keys.push_back(k);
      }

      if (control & HAVE_TIMESTAMP)
        timestamp = decode_ts64((const uint8_t **)&ptr, (control & TS_DESC) != 0);
      else if(control & AUTO_TIMESTAMP)
        timestamp = ScanSpecs::AUTO_ASSIGN;

      if (control & HAVE_REVISION)
        revision = decode_ts64((const uint8_t **)&ptr, (control & TS_DESC) != 0);
      else if (control & REV_IS_TS)
        revision = timestamp;
    
      assert(ptr == end_ptr);
    }

    // Make - serialized-key
    void make_skey() {
      klen = 3; // 1(flag) + 1(control) + 1(keys_num)

      for(auto it=keys.begin();it<keys.end();++it)
        klen+=(*it).len+1;
      if (control & HAVE_TIMESTAMP)
        klen += 8;
      if (control & HAVE_REVISION)
        klen += 8;
      
      skey = new uint8_t[klen];
      uint8_t* skey_base = skey; 
  
      *skey++ = flag;
      *skey++ = control;      
      *skey++ = (uint8_t)keys.size();

      for(auto it=keys.begin();it<keys.end();++it) {
        if((*it).len > 0) {
          memcpy(skey, (*it).key, (*it).len);
          skey+=(*it).len;
        }
        *skey++ = 0;
      }

      if (control & HAVE_TIMESTAMP)
        encode_ts64(&skey, timestamp, (control & TS_DESC) != 0);
      if (control & HAVE_REVISION)
        encode_ts64(&skey, revision, (control & TS_DESC) != 0);

      skey = skey_base;
    }

    // WRITE - DynamicBuffer
    void write(SWC::DynamicBuffer &dst_buf){

      if(!skey) 
        make_skey();

      dst_buf.ensure(klen+vlen+12); // 12(2x encode_vi32)

      Serialization::encode_vi32(&dst_buf.ptr, klen);
      dst_buf.add_unchecked(skey, klen);
      
      Serialization::encode_vi32(&dst_buf.ptr, vlen);
      if(vlen>0)
        dst_buf.add_unchecked(value, vlen);

      assert(dst_buf.fill() <= dst_buf.size);
    };
    
    uint8_t   flag;
    uint8_t   control;
    ListKeys  keys;
    int64_t   timestamp;
    uint64_t  revision;
    
    uint8_t*  skey;
    uint32_t  klen;

    uint8_t*  value;
    uint32_t  vlen;
  };
  std::ostream &operator<<(std::ostream &os, Cell &cell){

    if(!cell.skey) 
      cell.make_skey();

    os << "{Cell:{"
       << "flag:" << cell.flag << ","
       << "control:" << cell.control << ","
       << "keys:[";
    if(cell.keys.size()>0){
      for(auto it=cell.keys.begin(); it < cell.keys.end();it++)
        os << (*it) << ",";
    }
    os << "],"
       << "timestamp:" << cell.timestamp << ","
       << "revision:" << cell.revision << ","

       << "value:\"" << (cell.vlen>0?std::string((const char *)cell.value, cell.vlen):"") << "\","
       << "vlen:" << cell.vlen << ","

       << "skey:\"" << std::string((const char *)cell.skey, cell.klen) << "\","
       << "klen:" <<cell.klen << ","

       << "}}";
    return os;
  }

  // (int ScanBlock::load()) ScanBlock::next(CellSerial)


}}
#endif