/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Cells/Cell.h"

#include "swcdb/core/Time.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Cells {


std::string to_string(Flag flag) {
  switch(flag) {
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

Flag flag_from(const uint8_t* rptr, uint32_t len) {
  const char* ptr = (const char*)rptr;
  if(len >= 14) {
    if(!strncasecmp(ptr, "delete_version", 14))
      return Flag::DELETE_VERSION;
  }
  if(len >= 6) {
    if(!strncasecmp(ptr, "insert", 6))
      return Flag::INSERT;
    if(!strncasecmp(ptr, "delete", 6))
      return Flag::DELETE;
  }
  return Flag::NONE;
}


SWC_SHOULD_INLINE
Cell::Cell() :  own(false), flag(Flag::NONE), control(0), 
                vlen(0), value(0) { 
}

SWC_SHOULD_INLINE
Cell::Cell(const Cell& other)
  : key(other.key), own(other.vlen), flag(other.flag), 
    control(other.control), 
    vlen(other.vlen), 
    timestamp(other.timestamp), 
    revision(other.revision), 
    value(_value(other.value)) {
}

SWC_SHOULD_INLINE
Cell::Cell(const Cell& other, bool no_value)
  : key(other.key), own(!no_value && other.vlen), flag(other.flag), 
    control(other.control), 
    vlen(own ? other.vlen : 0), 
    timestamp(other.timestamp), 
    revision(other.revision), 
    value(_value(other.value)) {
}

SWC_SHOULD_INLINE
Cell::Cell(const uint8_t** bufp, size_t* remainp, bool own)
              : value(0) { 
  read(bufp, remainp, own);
}

void Cell::copy(const Cell& other, bool no_value) {
  key.copy(other.key);
  flag      = other.flag;
  control   = other.control;
  timestamp = other.timestamp;
  revision  = other.revision;
  
  if(no_value)
    free();
  else 
    set_value(other.value, other.vlen, true); 
}

Cell::~Cell() {
  if(own && value)
    delete [] value;
}

void Cell::free() {
  if(own && value)
    delete [] value;
  vlen = 0;
  value = 0;
}

void Cell::set_time_order_desc(bool desc) {
  if(desc)  
    control |= TS_DESC;
  else if(control & TS_DESC)  
    control -= TS_DESC;
}

void Cell::set_timestamp(int64_t ts) {
  timestamp = ts;
  control |= HAVE_TIMESTAMP;
}

void Cell::set_revision(int64_t ts) {
  revision = ts;
  control |= HAVE_REVISION;
}

void Cell::set_value(uint8_t* v, uint32_t len, bool owner) {
  free();
  vlen = len;
  value = (own = owner) ? _value(v) : v;
}

SWC_SHOULD_INLINE
void Cell::set_value(const char* v, uint32_t len, bool owner) {
  set_value((uint8_t *)v, len, owner);
}

SWC_SHOULD_INLINE
void Cell::set_value(const char* v, bool owner) {
  set_value((uint8_t *)v, strlen(v), owner);
}

SWC_SHOULD_INLINE
void Cell::set_value(const std::string& v, bool owner) {
  set_value((uint8_t *)v.data(), v.length(), owner);
}

void Cell::set_counter(uint8_t op, int64_t v, Types::Column typ, int64_t rev) {
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

  uint8_t* ptr = (value = new uint8_t[vlen]);
  Serialization::encode_vi64(&ptr, v);
  *ptr++ = op;
  if(op & HAVE_REVISION)
    Serialization::encode_vi64(&ptr, rev);
  // +? i64's storing epochs 
}

uint8_t Cell::get_counter_op() const {
  const uint8_t* ptr = value;
  Serialization::decode_vi64(&ptr);
  return *ptr;
}

int64_t Cell::get_counter() const {
  const uint8_t *ptr = value;
  return Serialization::decode_vi64(&ptr);
}

int64_t Cell::get_counter(uint8_t& op, int64_t& rev) const {
  const uint8_t *ptr = value;
  int64_t v = Serialization::decode_vi64(&ptr);
  rev = ((op = *ptr) & HAVE_REVISION) 
        ? Serialization::decode_vi64(&++ptr) 
        : TIMESTAMP_NULL;
  return v;
}

void Cell::read(const uint8_t** bufp, size_t* remainp, bool owner) {

  flag = Serialization::decode_i8(bufp, remainp);
  key.decode(bufp, remainp, owner);
  control = Serialization::decode_i8(bufp, remainp);

  if(control & HAVE_TIMESTAMP)
    timestamp = Serialization::decode_i64(bufp, remainp);
  else if(control & AUTO_TIMESTAMP)
    timestamp = AUTO_ASSIGN;
 
  if(control & HAVE_REVISION)
    revision = Serialization::decode_i64(bufp, remainp);
  else if(control & REV_IS_TS)
    revision = timestamp;

  free();
  if((vlen = Serialization::decode_vi32(bufp, remainp))) {
    value = (own = owner) ? _value(*bufp) : (uint8_t *)*bufp;
    *bufp += vlen;
    SWC_ASSERT(*remainp >= vlen);
    *remainp -= vlen;
  }
}

uint32_t Cell::encoded_length(bool no_value) const {
  uint32_t len = key.encoded_length();
  len += 2;
  if(control & HAVE_TIMESTAMP)
    len += 8;
  if(control & HAVE_REVISION)
    len += 8;
  if(no_value)
    return ++len;
  len += Serialization::encoded_length_vi32(vlen);
  return len += vlen;
}

void Cell::write(DynamicBuffer &dst_buf, bool no_value) const {
  dst_buf.ensure(encoded_length( no_value || (no_value = !vlen) ));

  Serialization::encode_i8(&dst_buf.ptr, flag);
  key.encode(&dst_buf.ptr);

  Serialization::encode_i8(&dst_buf.ptr, control);
  if(control & HAVE_TIMESTAMP)
    Serialization::encode_i64(&dst_buf.ptr, timestamp);
  if(control & HAVE_REVISION)
    Serialization::encode_i64(&dst_buf.ptr, revision);
    
  if(no_value) {
    Serialization::encode_i8(&dst_buf.ptr, 0);
  } else {
    Serialization::encode_vi32(&dst_buf.ptr, vlen);
    dst_buf.add_unchecked(value, vlen);
  }

  SWC_ASSERT(dst_buf.fill() <= dst_buf.size);
}

bool Cell::equal(const Cell& other) const {
  return  flag == other.flag && 
          control == other.control &&
          (!(control & HAVE_TIMESTAMP) || timestamp == other.timestamp) &&
          (!(control & HAVE_REVISION) || revision == other.revision) && 
          vlen == other.vlen &&
          key.equal(other.key) &&
          !memcmp(value, other.value, vlen);
}

SWC_SHOULD_INLINE
bool Cell::removal() const {
  return flag != Flag::INSERT;
}

bool Cell::is_removing(const int64_t& rev) const {
  return rev != AUTO_ASSIGN && removal() && (
    (flag == DELETE  && get_timestamp() >= rev )
    ||
    (flag == DELETE_VERSION && get_timestamp() == rev )
    );
}

int64_t Cell::get_timestamp() const {
  return control & HAVE_TIMESTAMP ? timestamp : AUTO_ASSIGN;
}

int64_t Cell::get_revision() const {
  return control & HAVE_REVISION ? revision 
        : (control & REV_IS_TS ? timestamp : AUTO_ASSIGN );
}

bool Cell::has_expired(const int64_t ttl) const {
  return ttl && control & HAVE_TIMESTAMP && Time::now_ns() >= timestamp + ttl;
}

void Cell::display(std::ostream& out, 
                   Types::Column typ, uint8_t flags, bool meta) const {

  if(flags & DisplayFlag::DATETIME) 
    out << Time::fmt_ns(timestamp) << '\t';

  if(flags & DisplayFlag::TIMESTAMP) 
    out << timestamp << '\t';
  
  bool bin = flags & DisplayFlag::BINARY;
  key.display(out, !bin);
  out << '\t';

  if(flag != Flag::INSERT) {
    out << '(' << Cells::to_string((Flag)flag) << ')';
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
        eq_rev = get_timestamp();
      out << value;
      if(eq_rev != TIMESTAMP_NULL)
        out << " EQ-SINCE(" << Time::fmt_ns(eq_rev) << ")";
    } else
        out << get_counter();

  } else if(meta && !bin) {    
    const uint8_t* ptr = value;
    size_t remain = vlen;
    DB::Cell::Key de_key;
    de_key.decode(&ptr, &remain, false);
    out << "end=";
    de_key.display(out, true);
    out << " rid=" << Serialization::decode_vi64(&ptr, &remain);
    de_key.decode(&ptr, &remain, false);
    out << " min=";
    de_key.display(out, true);
    de_key.decode(&ptr, &remain, false);
    out << " max=";
    de_key.display(out, true);

  } else {
    const uint8_t* ptr = value;
    char hex[5];
    hex[4] = 0;

    for(uint32_t i=vlen; i; --i, ++ptr) {
      if(!bin && (*ptr < 32 || *ptr > 126)) {
        sprintf(hex, "0x%X", *ptr);
        out << hex;
      } else {
        out << *ptr;
      }
    }
  }
}

std::string Cell::to_string(Types::Column typ) const {
  std::stringstream ss;
  print(ss, typ);
  return ss.str();
}

void Cell::print(std::ostream& out, Types::Column typ) const {
  out << "Cell(flag=" << Cells::to_string((Flag)flag);
  key.print(out << " key=");
  out << " control=" << int(control)
      << " ts=" << get_timestamp()
      << " rev=" << get_revision()
      << " value=(len=" << vlen;

  if(Types::is_counter(typ)) {
    out << " count=";  
    uint8_t op;
    int64_t eq_rev = TIMESTAMP_NULL;
    out << get_counter(op, eq_rev);
    if(op & OP_EQUAL && !(op & HAVE_REVISION))
      eq_rev = get_timestamp();
    if(eq_rev != TIMESTAMP_NULL)
      out << " eq-since=" << Time::fmt_ns(eq_rev);

  } else {
    out << " data=\"";  
    char hex[5];
    hex[4] = 0;
    const uint8_t* ptr = value;
    for(uint32_t len = vlen; len; --len, ++ptr) {
      if(*ptr == '"')
        out << '\\';
      if(31 < *ptr && *ptr < 127) {
        out << *ptr;
      } else {
        sprintf(hex, "0x%X", *ptr);
        out << hex;
      }
    }
    out << "\")";
  }
  out << ')';
}

SWC_SHOULD_INLINE
uint8_t* Cell::_value(const uint8_t* v) {
  return vlen ? (uint8_t*)memcpy(new uint8_t[vlen], v, vlen) : 0;
}


}}}
