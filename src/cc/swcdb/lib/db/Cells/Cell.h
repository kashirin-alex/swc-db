/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_Cell_h
#define swcdb_db_cells_Cell_h

#include <cassert>
#include "CellKey.h"

#include "swcdb/lib/core/DynamicBuffer.h"


namespace SWC { namespace DB { namespace Cells {

enum Flag {
  INSERT                    = 0x0,
  DELETE                    = 0x1,
  DELETE_VERSION            = 0x2,
  DELETE_FRACTION           = 0x3,
  DELETE_FRACTION_VERSION   = 0x4
};

static const int64_t TIMESTAMP_MIN  = INT64_MIN;
static const int64_t TIMESTAMP_MAX  = INT64_MAX;
static const int64_t TIMESTAMP_NULL = INT64_MIN + 1;
static const int64_t TIMESTAMP_AUTO = INT64_MIN + 2;
static const int64_t AUTO_ASSIGN    = TIMESTAMP_AUTO;

static const uint8_t HAVE_REVISION      =  0x80;
static const uint8_t HAVE_TIMESTAMP     =  0x40;
static const uint8_t AUTO_TIMESTAMP     =  0x20;
static const uint8_t REV_IS_TS          =  0x10;
static const uint8_t TS_DESC            =   0x1;


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

  
class Cell {
  public:
  
  static void get_key_and_seek(DB::Cell::Key& key, SWC::DynamicBuffer &src_buf){
    ++src_buf.ptr;
    size_t remain = src_buf.fill();
    key.decode((const uint8_t **)&src_buf.ptr, &remain);
    uint8_t control = *++src_buf.ptr;
    if(control & HAVE_TIMESTAMP || control & AUTO_TIMESTAMP)
      src_buf.ptr += 8;
    if(control & HAVE_REVISION)
      src_buf.ptr += 8;
    src_buf.ptr += Serialization::decode_vi32((const uint8_t **)&src_buf.ptr);
  }

  static bool is_next(uint8_t flag, SWC::DynamicBuffer &src_buf) {
    return *src_buf.ptr == flag;
  }


  Cell(): flag(0), control(0), 
          timestamp(0), revision(0),
          value(0), vlen(0) { }

  Cell(const Cell& other){
    copy(other);
  }

  Cell(Cell* other) {
    copy((const Cell)*other);
  }

  void copy(const Cell& other) {
    //std::cout << " copy(const Cell &other) vlen=" << other.vlen << "\n";
    flag      = other.flag;
    key.copy(other.key);
    control   = other.control;
    timestamp = other.timestamp;
    revision  = other.revision;
    vlen      = other.vlen;
    if(vlen > 0) {
      value = new uint8_t[vlen];
      memcpy(value, other.value, vlen);
    }
  }

  virtual ~Cell(){
    //std::cout << " ~Cell\n";
    if(value != 0)
      delete [] value;
    vlen=0;
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
    
  // READ
  void read(uint8_t **ptr, size_t* remainp) {
    const uint8_t* tmp_ptr = *ptr;

    flag = *tmp_ptr++;
    key.decode(&tmp_ptr, remainp);
    control = *tmp_ptr++;
    *remainp-=2;
  
    if (control & HAVE_TIMESTAMP){
      timestamp = decode_ts64(&tmp_ptr, (control & TS_DESC) != 0);
      *remainp-=8;
    } else if(control & AUTO_TIMESTAMP)
      timestamp = AUTO_ASSIGN;

    if (control & HAVE_REVISION) {
      revision = decode_ts64(&tmp_ptr, (control & TS_DESC) != 0);
      *remainp-=8;
    } else if (control & REV_IS_TS)
      revision = timestamp;
      
    vlen = Serialization::decode_vi32(&tmp_ptr, remainp);
    if(vlen > 0){
      value = new uint8_t[vlen];
      memcpy(value, tmp_ptr, vlen);
      tmp_ptr += vlen;
      *remainp -= vlen;
    }

    *ptr += tmp_ptr - *ptr;
  }

  // WRITE
  void write(SWC::DynamicBuffer &dst_buf){
    uint32_t klen = 1+key.encoded_length()+1;
    if(control & HAVE_TIMESTAMP)
      klen += 8;
    if(control & HAVE_REVISION)
      klen += 8;
    dst_buf.ensure(klen+vlen+Serialization::encoded_length_vi32(vlen));

    *dst_buf.ptr++ = flag;
    key.encode(&dst_buf.ptr);
    *dst_buf.ptr++ = control;
    if(control & HAVE_TIMESTAMP)
      encode_ts64(&dst_buf.ptr, timestamp, (control & TS_DESC) != 0);
    if(control & HAVE_REVISION)
      encode_ts64(&dst_buf.ptr, revision, (control & TS_DESC) != 0);
      
    Serialization::encode_vi32(&dst_buf.ptr, vlen);
    if(vlen > 0)
      dst_buf.add_unchecked(value, vlen);

    assert(dst_buf.fill() <= dst_buf.size);
  }

  bool equal(Cell &other){
    return  flag == other.flag && 
            control == other.control &&
            timestamp == other.timestamp && 
            revision == other.revision && 
            vlen == other.vlen &&
            key.equal(other.key) &&
            memcmp(value, other.value, vlen) == 0;
  }

  const std::string to_string() {
    std::string s("Cell(");
    s.append("flag=");
    s.append(std::to_string(flag));

    s.append(" key=");
    s.append(key.to_string());

    s.append(" control=");
    s.append(std::to_string(control));

    s.append(" ts=");
    s.append(std::to_string(timestamp));

    s.append(" rev=");
    s.append(std::to_string(revision));

    s.append(" value=(");
    s.append(vlen>0?std::string((const char *)value, vlen):std::string(""));
    s.append(", len=");
    s.append(std::to_string(vlen));
    s.append(")");
    return s;
  }

  uint8_t         flag;
  DB::Cell::Key   key;
  uint8_t         control;
  int64_t         timestamp;
  uint64_t        revision;
    
  uint8_t*  value;
  uint32_t  vlen;
};

 
// (int ScanBlock::load()) ScanBlock::next(CellSerial)


}}}
#endif