/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/CellValueSerialFields.h"



namespace SWC { namespace DB { namespace Cells {



const char* to_string(Flag flag) noexcept {
  switch(flag) {
    case Flag::INSERT:
      return "INSERT";
    case Flag::DELETE_LE:
      return "DELETE_LE";
    case Flag::DELETE_EQ:
      return "DELETE_EQ";
    case Flag::NONE:
      return "NONE";
    default:
      return "UNKNOWN";
  }
}

Flag flag_from(const uint8_t* rptr, uint32_t len) noexcept {
  const char* ptr = reinterpret_cast<const char*>(rptr);
  if(len >= 9) {
    if(Condition::str_case_eq(ptr, "DELETE_LE", 9))
      return Flag::DELETE_LE;
    if(Condition::str_case_eq(ptr, "DELETE_EQ", 9))
      return Flag::DELETE_EQ;
  }
  if(len >= 6) {
    if(Condition::str_case_eq(ptr, "insert", 6))
      return Flag::INSERT;
  }
  return Flag::NONE;
}


void Cell::set_value(Types::Encoder encoder, const uint8_t* v, uint32_t len) {
  _free();
  if(!len) {
    vlen = 0;
    value = nullptr;
    return;
  }

  int err = Error::OK;
  size_t len_enc = 0;
  DynamicBuffer output;
  Core::Encoder::encode(err, encoder, v, len, &len_enc, output,
                        Serialization::encoded_length_vi32(len) + 1, false);
  if(len_enc) {
    control |= HAVE_ENCODER;
    uint8_t* ptr = output.base;
    Serialization::encode_vi32(&ptr, len);
    Serialization::encode_i8(&ptr, uint8_t(encoder));
    vlen = output.fill();
    value = _value(output.base);
    // or keep as - value = output.base, output.own = false;
  } else {
    control &= MASK_HAVE_ENCODER;
    vlen = len;
    value = _value(v);
  }
  own = true;
}

Types::Encoder Cell::get_value(StaticBuffer& v, bool owner) const {
  Types::Encoder encoder;
  if(have_encoder()) {
    const uint8_t* ptr = value;
    size_t remain = vlen;
    v.reallocate(Serialization::decode_vi32(&ptr, &remain));
    encoder = Types::Encoder(Serialization::decode_i8(&ptr, &remain));
    int err = Error::OK;
    Core::Encoder::decode(err, encoder, ptr, remain, v.base, v.size);
    if(err) {
      v.free();
      SWC_THROWF(Error::ENCODER_DECODE, "Cell(key=%s) value-encoder(%s)",
        key.to_string().c_str(), Core::Encoder::to_string(encoder));
    }
  } else {
    encoder = Types::Encoder::DEFAULT;
    if(vlen)
      v.set(value, vlen, owner);
  }
  return encoder;
}


bool Cell::equal(const Cell& other) const noexcept {
  return  flag == other.flag &&
          control == other.control &&
          vlen == other.vlen &&
          (!(control & HAVE_TIMESTAMP) || timestamp == other.timestamp) &&
          (!(control & HAVE_REVISION) || revision == other.revision) &&
          key.equal(other.key) &&
          Condition::mem_eq(value, other.value, vlen);
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
    out << '(' << Cells::to_string(Flag(flag)) << ')';
    return;
  }

  if(!vlen)
    return;

  if(Types::is_counter(typ)) {
    if(bin) {
      uint8_t op;
      int64_t eq_rev = TIMESTAMP_NULL;
      out << get_counter(op, eq_rev);
      if(op & OP_EQUAL && !(op & HAVE_REVISION))
        eq_rev = get_timestamp();
      if(eq_rev != TIMESTAMP_NULL)
        out << " EQ-SINCE(" << Time::fmt_ns(eq_rev) << ")";
    } else {
      out << get_counter();
    }

  } else if(meta && !bin) {
    StaticBuffer v;
    get_value(v, false);
    const uint8_t* ptr = v.base;
    size_t remain = v.size;
    DB::Cell::Key de_key;
    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    de_key.decode(&ptr, &remain, false);
    de_key.display(out << "end=", true);
    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    out << " rid=" << Serialization::decode_vi64(&ptr, &remain);
    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    de_key.decode(&ptr, &remain, false);
    de_key.display(out << " min=", true);
    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    de_key.decode(&ptr, &remain, false);
    de_key.display(out << " max=", true);

  } else {
    StaticBuffer v;
    get_value(v, false);

    if(typ == Types::Column::SERIAL) {
      DB::Cell::Serial::Value::Fields::display(v.base, v.size, out);

    } else {
      const uint8_t* ptr = v.base;
      char hex[5];
      hex[4] = 0;

      for(uint32_t i=v.size; i; --i, ++ptr) {
        if(!bin && (*ptr < 32 || *ptr > 126)) {
          sprintf(hex, "0x%X", *ptr);
          out << hex;
        } else {
          out << *ptr;
        }
      }
    }
  }
}

void Cell::print(std::ostream& out, Types::Column typ) const {
  out << "Cell(flag=" << Cells::to_string(Flag(flag));
  key.print(out << " key=");
  out << " control=" << int(control)
      << " ts=" << get_timestamp()
      << " rev=" << get_revision()
      << " enc=" << have_encoder()
      << " value=(len=" << vlen;

  if(flag != Flag::INSERT || !vlen) {
    out << ')' << ')';
    return;
  }

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
    StaticBuffer v;
    get_value(v, false);

    if(typ == Types::Column::SERIAL) {
      DB::Cell::Serial::Value::Fields::display(v.base, v.size, out);

    } else {
      char hex[5];
      hex[4] = 0;
      const uint8_t* ptr = v.base;
      for(uint32_t len = v.size; len; --len, ++ptr) {
        if(*ptr == '"')
          out << '\\';
        if(31 < *ptr && *ptr < 127) {
          out << *ptr;
        } else {
          sprintf(hex, "0x%X", *ptr);
          out << hex;
        }
      }
    }
    out << "\")";
  }
  out << ')';
}



}}}
