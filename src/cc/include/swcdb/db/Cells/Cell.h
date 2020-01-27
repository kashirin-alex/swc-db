/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_Cell_h
#define swcdb_db_cells_Cell_h

#include <cassert>

#include "swcdb/core/Time.h"
#include "swcdb/core/DynamicBuffer.h"

#include "swcdb/db/Types/Column.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace DB { 

enum DisplayFlag {
  TIMESTAMP     = 0x01,
  DATETIME      = 0x04,
  BINARY        = 0x08,
  SPECS         = 0x10,
  STATS         = 0x20
};

enum OutputFlag {
  NO_TS     = 0x01,
  NO_VALUE  = 0x04
};

namespace Cells {

enum Flag {
  NONE                      = 0x0, // empty instance
  INSERT                    = 0x1,
  DELETE                    = 0x2,
  DELETE_VERSION            = 0x3
};

const std::string to_string(Flag flag) {
  switch(flag){
    case Flag::INSERT:
      return std::string("INSERT");
    case Flag::DELETE:
      return std::string("DELETE");
    case Flag::DELETE_VERSION:
      return std::string("DELETE_VERSION");
    case Flag::NONE:
      return std::string("NONE");
    default:
      return std::string("UKNONWN");
  }
}

const Flag flag_from(const uint8_t* rptr, uint32_t len) {
  const char* ptr = (const char*)rptr;
  if(len >= 14) {
    if(strncasecmp(ptr, "delete_version", 14) == 0)
      return Flag::DELETE_VERSION;
  }
  if(len >= 6) {
    if(strncasecmp(ptr, "insert", 6) == 0)
      return Flag::INSERT;
    if(strncasecmp(ptr, "delete", 6) == 0)
      return Flag::DELETE;
  }
  return Flag::NONE;
}



static const int64_t TIMESTAMP_MIN  = INT64_MIN;
static const int64_t TIMESTAMP_MAX  = INT64_MAX;
static const int64_t TIMESTAMP_NULL = INT64_MIN + 1;
static const int64_t TIMESTAMP_AUTO = INT64_MIN + 2;
static const int64_t AUTO_ASSIGN    = TIMESTAMP_AUTO;

static const uint8_t HAVE_REVISION      =  0x80;
static const uint8_t HAVE_TIMESTAMP     =  0x40;
static const uint8_t AUTO_TIMESTAMP     =  0x20;
//static const uint8_t REV_IS_TS        =  0x10;
//static const uint8_t TYPE_DEFINED     =  0x2;
static const uint8_t TS_DESC            =  0x1;

static const uint8_t OP_EQUAL  = 0x1;

class Cell final {
  public:
  typedef std::shared_ptr<Cell> Ptr;

  explicit Cell():  flag(Flag::NONE), control(0), 
                    timestamp(0), //revision(0),
                    value(0), vlen(0), own(false) { }

  explicit Cell(const uint8_t** bufp, size_t* remainp, bool own=false)
                : timestamp(0), //revision(0), 
                  value(0), vlen(0) { 
    read(bufp, remainp, own);             
  }

  explicit Cell(const Cell& other)
                : value(0), vlen(0) {
    copy(other);
  }

  explicit Cell(const Cell& other, bool no_value)
                : value(0), vlen(0) {
    copy(other, no_value);
  }

  void copy(const Cell& other, bool no_value=false) {
    if(no_value)
      free();
    else 
      set_value(other.value, other.vlen, true);
      
    flag        = other.flag;
    key.copy(other.key);
    control     = other.control;
    timestamp   = other.timestamp;
    //revision    = other.revision;
  }

  ~Cell() {
    if(own && value)
      delete [] value;
    key.free();
  }

  void free() {
    if(own && value)
      delete [] value;
    vlen = 0;
    value = 0;
  }

  void set_time_order_desc(bool desc) {
    if(desc)  control |= TS_DESC;
    else      control != TS_DESC;
  }

  void set_timestamp(int64_t ts) {
    timestamp = ts;
    control |= HAVE_TIMESTAMP;
  }

  /*
  void set_revision(int64_t rev) {
    revision = rev;
    control |= HAVE_REVISION;
  }
  */

  // SET_VALUE
  void set_value(uint8_t* v, uint32_t len, bool cpy) {
    free();
    vlen = len;
    if(!cpy) {
      value = v;
      own = cpy;
    } else if(vlen) {
      value = new uint8_t[vlen];
      memcpy(value, v, vlen);
      own = cpy;
    }
  }

  void set_value(const char* v, uint32_t len, bool cpy) {
    set_value((uint8_t *)v, len, cpy);
  }

  void set_value(const char* v, bool cpy=false){
    set_value((uint8_t *)v, strlen(v), cpy);
  }

  void set_value(const std::string& v, bool cpy=false) {
    set_value((uint8_t *)v.data(), v.length(), cpy);
  }

  void set_counter(uint8_t op, int64_t v,  
                  Types::Column typ = Types::Column::COUNTER_I64, 
                  int64_t rev = TIMESTAMP_NULL) {
    free();
    own = true;

    switch(typ) {
      case Types::Column::COUNTER_I8:
        v = (int8_t)v;
        break;
      case Types::Column::COUNTER_I16:
        v = (int16_t)v;
        break;
      case Types::Column::COUNTER_I32:
        v = (int32_t)v;
        break;
      default:
        break;
    }

    vlen = 1 + Serialization::encoded_length_vi64(v);
    if(op & OP_EQUAL && rev != TIMESTAMP_NULL) {
      op |= HAVE_REVISION;
      vlen += Serialization::encoded_length_vi64(rev);
    }

    value = new uint8_t[vlen];
    uint8_t* ptr = value;
    Serialization::encode_vi64(&ptr, v);
    *ptr++ = op;
    if(op & HAVE_REVISION)
      Serialization::encode_vi64(&ptr, rev);
    // +? i64's storing epochs 
  }

  const uint8_t get_counter_op() const {
    const uint8_t* ptr = value;
    Serialization::decode_vi64(&ptr);
    return *ptr;
  }

  const int64_t get_counter() const {
    const uint8_t *ptr = value;
    return Serialization::decode_vi64(&ptr);
  }

  const int64_t get_counter(uint8_t& op, int64_t& rev) const {
    const uint8_t *ptr = value;
    int64_t v = Serialization::decode_vi64(&ptr);
    rev = ((op = *ptr++) & HAVE_REVISION) 
          ? Serialization::decode_vi64(&ptr) 
          : TIMESTAMP_NULL;
    return v;
  }

    
  // READ
  void read(const uint8_t **bufp, size_t* remainp, bool owner=false) {

    flag = Serialization::decode_i8(bufp, remainp);
    key.decode(bufp, remainp, owner);
    control = Serialization::decode_i8(bufp, remainp);

    if (control & HAVE_TIMESTAMP)
      timestamp = Serialization::decode_i64(bufp, remainp);
    else if(control & AUTO_TIMESTAMP)
      timestamp = AUTO_ASSIGN;

    /*
    if (control & HAVE_REVISION)
      revision = Serialization::decode_i64(bufp, remainp);
    else if (control & REV_IS_TS)
      revision = timestamp;
    */
    free();
    own = owner;
    if(vlen = Serialization::decode_vi32(bufp, remainp)) {
      if(own) {
        value = new uint8_t[vlen];
        memcpy(value, *bufp, vlen);
      } else
        value = (uint8_t *)*bufp;
      
      *bufp += vlen;
      assert(*remainp >= vlen);
      *remainp -= vlen;
    }
  }

  const uint32_t encoded_length() const {
    uint32_t len = 1+key.encoded_length()+1;
    if(control & HAVE_TIMESTAMP)
      len += 8;
    //if(control & HAVE_REVISION)
    //  len += 8;
    return len + Serialization::encoded_length_vi32(vlen) + vlen;
  }

  // WRITE
  void write(SWC::DynamicBuffer &dst_buf) const {
    dst_buf.ensure(encoded_length());

    Serialization::encode_i8(&dst_buf.ptr, flag);
    key.encode(&dst_buf.ptr);

    Serialization::encode_i8(&dst_buf.ptr, control);
    if(control & HAVE_TIMESTAMP)
      Serialization::encode_i64(&dst_buf.ptr, timestamp);
    //if(control & HAVE_REVISION)
    //  Serialization::encode_i64(&dst_buf.ptr, revision);
      
    Serialization::encode_vi32(&dst_buf.ptr, vlen);
    if(vlen)
      dst_buf.add_unchecked(value, vlen);

    assert(dst_buf.fill() <= dst_buf.size);
  }

  const bool equal(Cell &other) const {
    return  flag == other.flag && 
            control == other.control &&
            timestamp == other.timestamp && 
            //revision == other.revision && 
            vlen == other.vlen &&
            key.equal(other.key) &&
            memcmp(value, other.value, vlen) == 0;
  }

  const bool removal() const {
    return flag != Flag::INSERT;
  }
  
  const bool is_removing(const int64_t& rev) const {
    return removal() && (
      (flag == DELETE  && get_revision() >= rev )
      ||
      (flag == DELETE_VERSION && get_revision() == rev )
      );
  }

  const int64_t get_revision() const {
    //return control & HAVE_REVISION ? revision 
    //        : (control & REV_IS_TS ? timestamp : AUTO_ASSIGN );
    return control & HAVE_TIMESTAMP ? timestamp : AUTO_ASSIGN;
  }

  const bool has_expired(const uint64_t ttl) const {
    return ttl && control & HAVE_TIMESTAMP && Time::now_ns() >= timestamp + ttl;
  }

  const std::string to_string(Types::Column typ = Types::Column::PLAIN) const {
    std::string s("Cell(");
    s.append("flag=");
    s.append(Cells::to_string((Flag)flag));

    s.append(" key=");
    s.append(key.to_string());

    s.append(" control=");
    s.append(std::to_string(control));
    
    s.append(" ts=");
    s.append(std::to_string(timestamp));

    //s.append(" rev=");
    //s.append(std::to_string(revision));

    s.append(" value=(len="); 
    s.append(std::to_string(vlen));  
    s.append(" ");  
    if(Types::is_counter(typ)) {
      s.append(std::to_string(get_counter()));
    } else {
      char c;
      for(int i=0; i<vlen;++i) {
        c = *(value+i);
        s.append(std::string(&c, 1));
      }
    }
    s.append(")");
    return s;
  }

  void display(std::ostream& out, Types::Column typ = Types::Column::PLAIN, 
               uint8_t flags=0) const {

    if(flags & DisplayFlag::DATETIME) 
      out << Time::fmt_ns(timestamp) << "  ";

    if(flags & DisplayFlag::TIMESTAMP) 
      out << timestamp << "  ";
    
    bool bin = flags & DisplayFlag::BINARY;
    key.display(out, !bin);
    out << "  ";

    if(flag != Flag::INSERT) {
      out << "(" << Cells::to_string((Flag)flag) << ")";
      return;
    } 

    if(!vlen) 
      return;

    if(Types::is_counter(typ)) {
      if(bin) {
        uint8_t op;
        int64_t eq_rev = TIMESTAMP_NULL;
        int64_t value = get_counter(op, eq_rev);
        if(op & OP_EQUAL && !(op & HAVE_REVISION))
          eq_rev = get_revision();
        out << value;
        if(eq_rev != TIMESTAMP_NULL)
          out << " EQ-SINCE(" << Time::fmt_ns(eq_rev) << ")";
      } else
        out << get_counter();

    } else {
      const uint8_t* ptr = value;
      char hex[2];

      for(uint32_t i=vlen; i--; ++ptr) {
        if(!bin && (*ptr < 32 || *ptr > 126)) {
          sprintf(hex, "%X", *ptr);
          out << "0x" << hex;
        } else
          out << *ptr; 
      }
    }
  }


  uint8_t         flag;
  DB::Cell::Key   key;
  uint8_t         control;
  int64_t         timestamp;
  //int64_t         revision;
    
  uint8_t*        value;
  uint32_t        vlen;
  bool            own;
};



}}}
#endif