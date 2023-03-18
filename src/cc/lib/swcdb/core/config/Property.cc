/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/config/Property.h"


namespace SWC { namespace Config { namespace Property {

const char* Value::to_string(Type type) noexcept {
  switch(type) {
    case TYPE_BOOL:
      return "TYPE_BOOL";
    case TYPE_UINT8:
      return "TYPE_UINT8";
    case TYPE_UINT16:
      return "TYPE_UINT16";
    case TYPE_INT32:
      return "TYPE_INT32";
    case TYPE_INT64:
      return "TYPE_INT64";
    case TYPE_DOUBLE:
      return "TYPE_DOUBLE";
    case TYPE_STRING:
      return "TYPE_STRING";
    case TYPE_ENUM:
      return "TYPE_ENUM";
    case TYPE_STRINGS:
      return "TYPE_STRINGS";
    case TYPE_INT64S:
      return "TYPE_INT64S";
    case TYPE_DOUBLES:
      return "TYPE_DOUBLES";
    case TYPE_BOOL_G:
      return "TYPE_BOOL_G";
    case TYPE_UINT8_G:
      return "TYPE_UINT8_G";
    case TYPE_UINT16_G:
      return "TYPE_UINT16_G";
    case TYPE_INT32_G:
      return "TYPE_INT32_G";
    case TYPE_UINT64_G:
      return "TYPE_UINT64_G";
    case TYPE_ENUM_G:
      return "TYPE_ENUM_G";
    case TYPE_STRINGS_G:
      return "TYPE_STRINGS_G";
    default:
      return "unknown";
  }
}


void Value::assure_match(Type t1, Type t2) {
  SWC_ASSERTF(
    t1 == t2,
    "Value-Type mismatch expectation t1=%s == t2=%s",
    to_string(t1), to_string(t2)
  );
}

Value::Value(uint8_t a_flags) noexcept : flags(a_flags) { }

Value::Value(Value::Ptr ptr) noexcept : flags(ptr->flags.load()) { }

Value::~Value() noexcept { }

std::ostream& Value::operator<<(std::ostream& ostream) {
  return ostream << to_string();
}

bool Value::is_skippable() const noexcept {
  return flags & Value::SKIPPABLE;
}

bool Value::is_guarded() const noexcept {
  return flags & Value::GUARDED;
}

Value::Ptr Value::default_value(bool defaulted) noexcept {
  defaulted
    ? flags.fetch_xor(Value::DEFAULT)
    : flags.fetch_and(Value::DEFAULT);
  return this;
}

bool Value::is_default() const noexcept {
  return flags & Value::DEFAULT;
}

Value::Ptr Value::zero_token() noexcept {
  flags.fetch_xor(Value::NO_TOKEN);
  return this;
}

bool Value::is_zero_token() const noexcept {
  return flags & Value::NO_TOKEN;
}



/**
 * Convertors & Validators from std::string
*/

void from_string(const char* s, double* value) {
  char *last;
  double res = strtod(s, &last);

  if (s == last)
    SWC_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s);

  switch (*last) {
    case 'k':
    case 'K': res *= 1000LL;         break;
    case 'm':
    case 'M': res *= 1000000LL;      break;
    case 'g':
    case 'G': res *= 1000000000LL;   break;
    case '\0':                       break;
    default:
      SWC_THROWF(Error::CONFIG_GET_ERROR,
                "Bad Value %s unknown suffix %s", s, last);
  }
  *value = res;
}

void from_string(const char* s, int64_t* value) {
  char *last;
  errno = 0;
  *value = strtoll(s, &last, 0);

  if (s == last)
    SWC_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s);

  if(errno)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
              "Bad Value %s, number out of range of 64-bit integer", s);

  switch (*last) {
    case 'k':
    case 'K': *value *= 1000LL;         break;
    case 'm':
    case 'M': *value *= 1000000LL;      break;
    case 'g':
    case 'G': *value *= 1000000000LL;   break;
    case '\0':                          break;
    default:
      SWC_THROWF(Error::CONFIG_GET_ERROR,
                "Bad Value %s unknown suffix %s", s, last);
  }
}

void from_string(const char* s, uint64_t* value) {
  char *last;
  errno = 0;
  *value = strtoull(s, &last, 0);

  if (s == last)
    SWC_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s);

  if(errno)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
      "Bad Value %s, number out of range of unsigned 64-bit integer", s);

  switch (*last) {
    case 'k':
    case 'K': *value *= 1000LL;         break;
    case 'm':
    case 'M': *value *= 1000000LL;      break;
    case 'g':
    case 'G': *value *= 1000000000LL;   break;
    case '\0':                          break;
    default:
      SWC_THROWF(Error::CONFIG_GET_ERROR,
                "Bad Value %s unknown suffix %s", s, last);
  }
}

void from_string(const char* s, uint8_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > UINT8_MAX || res < 0)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
      "Bad Value %s, number out of range of 8-bit unsigned integer", s);
  *value = res;
}


void from_string(const char* s, uint16_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > UINT16_MAX || res < 0)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
      "Bad Value %s, number out of range of 16-bit unsigned integer", s);
  *value = res;
}

void from_string(const char* s, int32_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > INT32_MAX || res < INT32_MIN)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
        "Bad Value %s, number out of range of 32-bit integer", s);
  *value = res;
}





Value_bool::Value_bool(const bool& v, uint8_t a_flags) noexcept
              : Value(a_flags), value(v) {
}

Value_bool::Value_bool(Value_bool* ptr) noexcept : Value(ptr), value(ptr->get()) { }

Value_bool::~Value_bool() noexcept { }

Value::Ptr Value_bool::make_new(const Strings& values) {
  auto ptr = new Value_bool(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_bool::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_bool>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_bool::set_from(const Strings& values) {
  auto& str = values.back();
  value = (str.length() == 1 && *str.data() == '1') ||
          Condition::str_case_eq(str.data(), "true", 4) ||
          Condition::str_case_eq(str.data(), "yes", 3);
}

Value::Type Value_bool::type() const noexcept {
  return value_type;
}

std::string Value_bool::to_string() const {
  return value ? "true" : "false";
}



Value_uint8::Value_uint8(const uint8_t& v, uint8_t a_flags) noexcept
                : Value(a_flags), value(v) {
}

Value_uint8::Value_uint8(Value_uint8* ptr) noexcept : Value(ptr), value(ptr->get()) { }

Value_uint8::~Value_uint8() noexcept { }

Value::Ptr Value_uint8::make_new(const Strings& values) {
  auto ptr = new Value_uint8(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_uint8::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_uint8>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_uint8::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type Value_uint8::type() const noexcept {
  return value_type;
}

std::string Value_uint8::to_string() const {
  return std::to_string(int16_t(value));
}



Value_uint16::Value_uint16(const uint16_t& v, uint8_t a_flags) noexcept
                  : Value(a_flags), value(v) { }

Value_uint16::Value_uint16(Value_uint16* ptr) noexcept : Value(ptr), value(ptr->get()) { }

Value_uint16::~Value_uint16() noexcept { }

Value::Ptr Value_uint16::make_new(const Strings& values) {
  auto ptr = new Value_uint16(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_uint16::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_uint16>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_uint16::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type Value_uint16::type() const noexcept {
  return value_type;
}

std::string Value_uint16::to_string() const {
  return std::to_string(value);
}



Value_int32::Value_int32(const int32_t& v, uint8_t a_flags) noexcept
                : Value(a_flags), value(v) { }

Value_int32::Value_int32(Value_int32* ptr) noexcept : Value(ptr), value(ptr->get()) { }

Value_int32::~Value_int32() noexcept { }

Value::Ptr Value_int32::make_new(const Strings& values) {
  auto ptr = new Value_int32(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_int32::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_int32>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_int32::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type Value_int32::type() const noexcept {
  return value_type;
}

std::string Value_int32::to_string() const {
  return std::to_string(value);
}



Value_int64::Value_int64(const int64_t& v, uint8_t a_flags) noexcept
                 : Value(a_flags), value(v) { }

Value_int64::Value_int64(Value_int64* ptr) noexcept : Value(ptr), value(ptr->get()) { }

Value_int64::~Value_int64() noexcept { }

Value::Ptr Value_int64::make_new(const Strings& values) {
  auto ptr = new Value_int64(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_int64::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_int64>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_int64::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type Value_int64::type() const noexcept {
  return value_type;
}

std::string Value_int64::to_string() const {
  return std::to_string(value);
}



Value_double::Value_double(const double& v, uint8_t a_flags) noexcept
                  : Value(a_flags), value(v) { }

Value_double::Value_double(Value_double* ptr) noexcept : Value(ptr), value(ptr->get()) { }

Value_double::~Value_double() noexcept { }

Value::Ptr Value_double::make_new(const Strings& values) {
  auto ptr = new Value_double(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_double::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_double>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_double::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type Value_double::type() const noexcept {
  return value_type;
}

std::string Value_double::to_string() const {
  return format("%g", value);
}



Value_string::Value_string(std::string&& v, uint8_t a_flags) noexcept
                  : Value(a_flags), value(std::move(v)) {
}

Value_string::Value_string(Value_string* ptr) : Value(ptr), value(ptr->get()) { }

Value_string::~Value_string() noexcept { }

Value::Ptr Value_string::make_new(const Strings& values) {
  auto ptr = new Value_string(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_string::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_string>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_string::set_from(const Strings& values) {
  value = values.back();
}

Value::Type Value_string::type() const noexcept {
  return value_type;
}

std::string Value_string::to_string() const {
  return value;
}



Value_enum::Value_enum(const int32_t& v,
               FromString_t&& from_string,
               Repr_t&& repr,
               uint8_t a_flags)
              : Value(a_flags), value(v),
                call_from_string(std::move(from_string)),
                call_repr(std::move(repr)) {
}

Value_enum::Value_enum(Value_enum* ptr)
              : Value(ptr), value(ptr->get()),
                call_from_string(ptr->call_from_string),
                call_repr(ptr->call_repr) {
}

Value_enum::~Value_enum() noexcept { }

Value::Ptr Value_enum::make_new(const Strings& values) {
  auto ptr = new Value_enum(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_enum::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_enum>(ptr);
  flags.store(from->flags);
  value = from->value;
  if(!call_from_string)
    call_from_string = from->call_from_string;
  if(!call_repr)
    call_repr = from->call_repr;
}

void Value_enum::set_from(const Strings& values) {
  if(!call_from_string)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
              "Bad Value %s, no from_string cb set", values.back().c_str());

  int nv = call_from_string(values.back());
  if(nv < 0)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
              "Bad Value %s, no corresponding enum", values.back().c_str());
  value = nv;
}

Value::Type Value_enum::type() const noexcept {
  return value_type;
}

std::string Value_enum::to_string() const {
  return format(
    "%s  # (%d)",
    (call_repr ? call_repr(get()).c_str() : "repr not defined"), get());
}



// lists
Value_strings::Value_strings(Strings&& v, uint8_t a_flags) noexcept
                    : Value(a_flags), value(std::move(v)) {
}

Value_strings::Value_strings(Value_strings* ptr) : Value(ptr), value(ptr->get()) { }

Value_strings::~Value_strings() noexcept { }

Value::Ptr Value_strings::make_new(const Strings& values) {
  auto ptr = new Value_strings(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_strings::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_strings>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_strings::set_from(const Strings& values) {
  value = values;
}

Value::Type Value_strings::type() const noexcept {
  return value_type;
}

std::string Value_strings::to_string() const {
  return format_list(value);
}



Value_int64s::Value_int64s(Int64s&& v, uint8_t a_flags) noexcept
                  : Value(a_flags), value(std::move(v)) {
}

Value_int64s::Value_int64s(Value_int64s* ptr) : Value(ptr), value(ptr->get()) { }

Value_int64s::~Value_int64s() noexcept { }

Value::Ptr Value_int64s::make_new(const Strings& values) {
  auto ptr = new Value_int64s(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_int64s::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_int64s>(ptr);
  flags.store(from->flags);
  value = from->value;
}

void Value_int64s::set_from(const Strings& values) {
  value.clear();
  int64_t v;
  for(const std::string& s: values) {
    from_string(s, &v);
    value.push_back(v);
  }
}

Value::Type Value_int64s::type() const noexcept {
  return value_type;
}

std::string Value_int64s::to_string() const {
  return format_list(value);
}



Value_doubles::Value_doubles(Doubles&& v, uint8_t a_flags) noexcept
                     : Value(a_flags), value(std::move(v)) {
}

Value_doubles::Value_doubles(Value_doubles* ptr) : Value(ptr), value(ptr->get()) { }

Value_doubles::~Value_doubles() noexcept { }

Value::Ptr Value_doubles::make_new(const Strings& values) {
  auto ptr = new Value_doubles(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_doubles::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_doubles>(ptr);
  flags.store(from->flags);
  value = from->get();
}

void Value_doubles::set_from(const Strings& values) {
  value.clear();
  double v;
  for(const std::string& s: values) {
    from_string(s, &v);
    value.push_back(v);
  }
}

Value::Type Value_doubles::type() const noexcept {
  return value_type;
}

std::string Value_doubles::to_string() const {
  return format_list(value);
}



// Guarded Atomic
Value_bool_g::Value_bool_g(const bool& v, Value_bool_g::OnChg_t&& cb, uint8_t a_flags)
                : Value(a_flags | Value::GUARDED),
                  value(v), on_chg_cb(std::move(cb)) {
}

Value_bool_g::Value_bool_g(Value_bool_g* ptr)
                : Value(ptr),
                  value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value_bool_g::~Value_bool_g() noexcept { }

Value::Ptr Value_bool_g::make_new(const Strings& values) {
  auto ptr = new Value_bool_g(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_bool_g::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_bool_g>(ptr);
  flags.store(from->flags);

  bool chg = value != from->value;
  value.store(from->value.load());
  if(!on_chg_cb)
    on_chg_cb = from->on_chg_cb;
  if(chg)
    on_change();
}

void Value_bool_g::set_from(const Strings& values) {
  auto& str = values.back();
  value.store((str.length() == 1 && *str.data() == '1') ||
              Condition::str_case_eq(str.data(), "true", 4) ||
              Condition::str_case_eq(str.data(), "yes", 3));
}

Value::Type Value_bool_g::type() const noexcept {
  return value_type;
}

std::string Value_bool_g::to_string() const {
  return value ? "true" : "false";
}

void Value_bool_g::set(bool v) noexcept {
  value.store(v);
}


void Value_bool_g::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void Value_bool_g::set_cb_on_chg(Value_bool_g::OnChg_t&& cb) {
  on_chg_cb = std::move(cb);
}



Value_uint8_g::Value_uint8_g(const uint8_t& v, Value_uint8_g::OnChg_t&& cb, uint8_t a_flags)
                  : Value(a_flags | Value::GUARDED),
                    value(v), on_chg_cb(std::move(cb)) {
}

Value_uint8_g::Value_uint8_g(Value_uint8_g* ptr)
                  : Value(ptr),
                    value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value_uint8_g::~Value_uint8_g() noexcept { }

Value::Ptr Value_uint8_g::make_new(const Strings& values) {
  auto ptr = new Value_uint8_g(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_uint8_g::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_uint8_g>(ptr);
  flags.store(from->flags);
  bool chg = value != from->value;
  value.store(from->value.load());
  if(!on_chg_cb)
    on_chg_cb = from->on_chg_cb;
  if(chg)
    on_change();
}

void Value_uint8_g::set_from(const Strings& values) {
  uint8_t v;
  from_string(values.back(), &v);
  value.store(v);
}

Value::Type Value_uint8_g::type() const noexcept {
  return value_type;
}

std::string Value_uint8_g::to_string() const {
  return std::to_string(int16_t(value));
}

void Value_uint8_g::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void Value_uint8_g::set_cb_on_chg(Value_uint8_g::OnChg_t&& cb) {
  on_chg_cb = std::move(cb);
}



Value_uint16_g::Value_uint16_g(const uint16_t& v,
                      Value_uint16_g::OnChg_t&& cb, uint8_t a_flags)
                    : Value(a_flags | Value::GUARDED),
                      value(v), on_chg_cb(std::move(cb)) {
}

Value_uint16_g::Value_uint16_g(Value_uint16_g* ptr)
                    : Value(ptr),
                      value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value_uint16_g::~Value_uint16_g() noexcept { }

Value::Ptr Value_uint16_g::make_new(const Strings& values) {
  auto ptr = new Value_uint16_g(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_uint16_g::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_uint16_g>(ptr);
  flags.store(from->flags);
  bool chg = value != from->value;
  value.store(from->value.load());
  if(!on_chg_cb)
    on_chg_cb = from->on_chg_cb;
  if(chg)
    on_change();
}

void Value_uint16_g::set_from(const Strings& values) {
  uint16_t v;
  from_string(values.back(), &v);
  value.store(v);
}

Value::Type Value_uint16_g::type() const noexcept {
  return value_type;
}

std::string Value_uint16_g::to_string() const {
  return std::to_string(value);
}

void Value_uint16_g::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void Value_uint16_g::set_cb_on_chg(Value_uint16_g::OnChg_t&& cb) {
  on_chg_cb = std::move(cb);
}



Value_int32_g::Value_int32_g(const int32_t& v, Value_int32_g::OnChg_t&& cb, uint8_t a_flags)
                  : Value(a_flags | Value::GUARDED),
                    value(v), on_chg_cb(std::move(cb)) {
}

Value_int32_g::Value_int32_g(Value_int32_g* ptr)
                  : Value(ptr),
                    value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value_int32_g::~Value_int32_g() noexcept { }

Value::Ptr Value_int32_g::make_new(const Strings& values) {
  auto ptr = new Value_int32_g(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_int32_g::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_int32_g>(ptr);
  flags.store(from->flags);
  bool chg = value != from->value;
  value.store(from->value.load());
  if(!on_chg_cb)
    on_chg_cb = from->on_chg_cb;
  if(chg)
    on_change();
}

void Value_int32_g::set_from(const Strings& values) {
  int32_t v;
  from_string(values.back(), &v);
  value.store(v);
}

Value::Type Value_int32_g::type() const noexcept {
  return value_type;
}

std::string Value_int32_g::to_string() const {
  return std::to_string(value);
}

void Value_int32_g::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void Value_int32_g::set_cb_on_chg(Value_int32_g::OnChg_t&& cb) {
  on_chg_cb = std::move(cb);
}



Value_uint64_g::Value_uint64_g(const uint64_t& v,
                      Value_uint64_g::OnChg_t&& cb, uint8_t a_flags)
                    : Value(a_flags | Value::GUARDED),
                      value(v), on_chg_cb(std::move(cb)) {
}

Value_uint64_g::Value_uint64_g(Value_uint64_g* ptr)
                    : Value(ptr),
                      value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value_uint64_g::~Value_uint64_g() noexcept { }

Value::Ptr Value_uint64_g::make_new(const Strings& values) {
  auto ptr = new Value_uint64_g(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_uint64_g::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_uint64_g>(ptr);
  flags.store(from->flags);
  bool chg = value != from->value;
  value.store(from->value.load());
  if(!on_chg_cb)
    on_chg_cb = from->on_chg_cb;
  if(chg)
    on_change();
}

void Value_uint64_g::set_from(const Strings& values) {
  uint64_t v;
  from_string(values.back(), &v);
  value.store(v);
}

Value::Type Value_uint64_g::type() const noexcept {
  return value_type;
}

std::string Value_uint64_g::to_string() const {
  return std::to_string(value);
}

void Value_uint64_g::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void Value_uint64_g::set_cb_on_chg(Value_uint64_g::OnChg_t&& cb) {
  on_chg_cb = std::move(cb);
}



Value_enum_g::Value_enum_g(const int32_t& v,
                 Value_enum_g::OnChg_t&& cb,
                 Value_enum_g::FromString_t&& from_string,
                 Value_enum_g::Repr_t&& repr,
                 uint8_t a_flags)
                : Value(a_flags | Value::GUARDED), value(v),
                  on_chg_cb(std::move(cb)),
                  call_from_string(std::move(from_string)),
                  call_repr(std::move(repr)) {
}

Value_enum_g::Value_enum_g(Value_enum_g* ptr)
                : Value(ptr), value(ptr->get()),
                  on_chg_cb(ptr->on_chg_cb),
                  call_from_string(ptr->call_from_string),
                  call_repr(ptr->call_repr) {
}

Value_enum_g::~Value_enum_g() noexcept { }

Value::Ptr Value_enum_g::make_new(const Strings& values) {
  auto ptr = new Value_enum_g(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_enum_g::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_enum_g>(ptr);
  flags.store(from->flags);
  bool chg = value != from->value;
  value.store(from->get());
  if(!on_chg_cb)
    on_chg_cb = from->on_chg_cb;
  if(!call_from_string)
    call_from_string = from->call_from_string;
  if(!call_repr)
    call_repr = from->call_repr;

  if(chg)
    on_change();
}

void Value_enum_g::set_from(const Strings& values) {
  if(!call_from_string)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
              "Bad Value %s, no from_string cb set", values.back().c_str());
  int nv = call_from_string(values.back());
  if(nv < 0)
    SWC_THROWF(Error::CONFIG_GET_ERROR,
              "Bad Value %s, no corresponding enum", values.back().c_str());
  value.store(nv);
}

Value::Type Value_enum_g::type() const noexcept {
  return value_type;
}

std::string Value_enum_g::to_string() const {
  return format(
    "%s  # (%d)",
    (call_repr ? call_repr(get()).c_str() : "repr not defined"), get());
}

void Value_enum_g::set(int32_t nv) {
  value.store(nv);
  on_change();
}

void Value_enum_g::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void Value_enum_g::set_cb_on_chg(Value_enum_g::OnChg_t&& cb) {
  on_chg_cb = std::move(cb);
}



// Guarded Mutex
Value_strings_g::Value_strings_g(Strings&& v, Value_strings_g::OnChg_t&& cb,
                       uint8_t a_flags) noexcept
                      : Value(a_flags | Value::GUARDED),
                        mutex(), value(std::move(v)), on_chg_cb(std::move(cb)) {
}

Value_strings_g::Value_strings_g(Value_strings_g* ptr)
                      : Value(ptr),
                        mutex(), value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value_strings_g::~Value_strings_g() noexcept { }

Value::Ptr Value_strings_g::make_new(const Strings& values) {
  auto ptr = new Value_strings_g(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void Value_strings_g::set_from(Value::Ptr ptr) {
  auto from = get_pointer<Value_strings_g>(ptr);
  flags.store(from->flags);
  bool chg;
  {
    Core::MutexAtomic::scope lock(mutex);
    chg = value != from->get();
    value = from->get();
    if(!on_chg_cb)
      on_chg_cb = from->on_chg_cb;
  }
  if(chg)
    on_change();
}

void Value_strings_g::set_from(const Strings& values) {
  Core::MutexAtomic::scope lock(mutex);
  value = values;
}

Value::Type Value_strings_g::type() const noexcept {
  return value_type;
}

std::string Value_strings_g::to_string() const {
  Core::MutexAtomic::scope lock(mutex);
  return format_list(value);
}

Strings Value_strings_g::get() const {
  Core::MutexAtomic::scope lock(mutex);
  return value;
}

size_t Value_strings_g::size() noexcept {
  Core::MutexAtomic::scope lock(mutex);
  return value.size();
}

std::string Value_strings_g::get_item(size_t n) {
  Core::MutexAtomic::scope lock(mutex);
  return value[n];
}

void Value_strings_g::on_change() const {
  if(on_chg_cb)
    on_chg_cb();
}

void Value_strings_g::set_cb_on_chg(Value_strings_g::OnChg_t&& cb) {
  Core::MutexAtomic::scope lock(mutex);
  on_chg_cb = std::move(cb);
}



}}} // namespace SWC::Config::Property

