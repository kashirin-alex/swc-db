/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/Error.h"
#include "swcdb/core/config/Property.h"

namespace SWC {


namespace Property {


Value::Value(uint8_t flags) : flags(flags) { }
  
Value::Value(Value::Ptr ptr) : flags(ptr->flags.load()) { }

Value::~Value() { }

std::ostream& Value::operator<<(std::ostream& ostream) {
  return ostream << to_string();
}
  
bool Value::is_skippable() const {
  return flags & Value::SKIPPABLE;
}

bool Value::is_guarded() const {
  return flags & Value::GUARDED;
}

Value::Ptr Value::default_value(bool defaulted) {
  if(defaulted)
    flags ^= Value::DEFAULT;
  else
    flags &= Value::DEFAULT;
  return this;
}

bool Value::is_default() const {
  return flags & Value::DEFAULT;
}
  
Value::Ptr Value::zero_token() {
  flags ^= Value::NO_TOKEN;
  return this;
}

bool Value::is_zero_token() const {
  return flags & Value::NO_TOKEN;
}



/**
 * Convertors & Validators from std::string
*/

void from_string(const std::string& s, double* value) {
  char *last;
  double res = strtod(s.c_str(), &last);

  if (s.c_str() == last)
    SWC_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s.c_str());

  switch (*last) {
    case 'k':
    case 'K': res *= 1000LL;         break;
    case 'm':
    case 'M': res *= 1000000LL;      break;
    case 'g':
    case 'G': res *= 1000000000LL;   break;
    case '\0':                          break;
    default: 
      SWC_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s unknown suffix %s", s.c_str(), last);
  }
  *value = res;
}

void from_string(const std::string& s, int64_t* value) {
  char *last;
  errno = 0;
  *value = strtoll(s.c_str(), &last, 0);

  if (s.c_str() == last)
    SWC_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s.c_str());
  
  if(errno) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, number out of range of 64-bit integer", s.c_str());
  
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
                "Bad Value %s unknown suffix %s", s.c_str(), last);
  }
}

void from_string(const std::string& s, uint8_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > UINT8_MAX || res < 0) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
      "Bad Value %s, number out of range of 8-bit unsigned integer", s.c_str());
  *value = (uint8_t)res;
}

void from_string(const std::string& s, uint16_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > UINT16_MAX || res < 0) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
      "Bad Value %s, number out of range of 16-bit unsigned integer", s.c_str());
  *value = (uint16_t)res;
}

void from_string(const std::string& s, int32_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > INT32_MAX || res < INT32_MIN) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
        "Bad Value %s, number out of range of 32-bit integer", s.c_str());
  *value = (int32_t)res;
}





V_BOOL::V_BOOL(const bool& v, uint8_t flags) : Value(flags), value(v) { }
  
V_BOOL::V_BOOL(V_BOOL* ptr) : Value(ptr), value(ptr->get()) { }
  
Value::Ptr V_BOOL::make_new(const Strings& values) {
  auto ptr = new V_BOOL(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void V_BOOL::set_from(Value::Ptr ptr) {
  auto from = ((V_BOOL*)ptr);
  flags.store(from->flags);
  value = from->value;
}
  
void V_BOOL::set_from(const Strings& values) {
  auto& str = values.back();
  value = str.compare("1") == 0 ||
          strncasecmp(str.data(), "true", 4) == 0 ||
          strncasecmp(str.data(), "yes", 3) == 0;
}

Value::Type V_BOOL::type() const {
  return value_type;
}

std::string V_BOOL::to_string() const {
  return value ? "true" : "false";
}

bool V_BOOL::get() const {
  return value;
}



V_UINT8::V_UINT8(const uint8_t& v, uint8_t flags) : Value(flags), value(v) { }
  
V_UINT8::V_UINT8(V_UINT8* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_UINT8::make_new(const Strings& values) {
  auto ptr = new V_UINT8(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_UINT8::set_from(Value::Ptr ptr) {
  auto from = ((V_UINT8*)ptr);
  flags.store(from->flags);
  value = from->value;
}
  
void V_UINT8::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type V_UINT8::type() const {
  return value_type;
}

std::string V_UINT8::to_string() const {
  return std::to_string((int16_t)value);
}

uint8_t V_UINT8::get() const {
  return value;
}



V_UINT16::V_UINT16(const uint16_t& v, uint8_t flags) : Value(flags), value(v) { }
  
V_UINT16::V_UINT16(V_UINT16* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_UINT16::make_new(const Strings& values) {
  auto ptr = new V_UINT16(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_UINT16::set_from(Value::Ptr ptr) {
  auto from = ((V_UINT16*)ptr);
  flags.store(from->flags);
  value = from->value;
}

void V_UINT16::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type V_UINT16::type() const {
  return value_type;
}

std::string V_UINT16::to_string() const {
  return std::to_string(value);
}

uint16_t V_UINT16::get() const {
  return value;
}
  


V_INT32::V_INT32(const int32_t& v, uint8_t flags) : Value(flags), value(v) { }

V_INT32::V_INT32(V_INT32* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_INT32::make_new(const Strings& values) {
  auto ptr = new V_INT32(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_INT32::set_from(Value::Ptr ptr) {
  auto from = ((V_INT32*)ptr);
  flags.store(from->flags);
  value = from->value;
}

void V_INT32::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type V_INT32::type() const {
  return value_type;
}

std::string V_INT32::to_string() const {
  return std::to_string(value);
}

int32_t V_INT32::get() const {
  return value;
}
  


V_INT64::V_INT64(const int64_t& v, uint8_t flags) : Value(flags), value(v) { }

V_INT64::V_INT64(V_INT64* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_INT64::make_new(const Strings& values) {
  auto ptr = new V_INT64(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_INT64::set_from(Value::Ptr ptr) {
  auto from = ((V_INT64*)ptr);
  flags.store(from->flags);
  value = from->value;
}

void V_INT64::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type V_INT64::type() const {
  return value_type;
}

std::string V_INT64::to_string() const {
  return std::to_string(value);
}

int64_t V_INT64::get() const {
  return value;
}
  


V_DOUBLE::V_DOUBLE(const double& v, uint8_t flags) : Value(flags), value(v) { }
  
V_DOUBLE::V_DOUBLE(V_DOUBLE* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_DOUBLE::make_new(const Strings& values) {
  auto ptr = new V_DOUBLE(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_DOUBLE::set_from(Value::Ptr ptr) {
  auto from = ((V_DOUBLE*)ptr);
  flags.store(from->flags);
  value = from->value;
}

void V_DOUBLE::set_from(const Strings& values) {
  from_string(values.back(), &value);
}

Value::Type V_DOUBLE::type() const {
  return value_type;
}

std::string V_DOUBLE::to_string() const {
  return format("%g", value);
}

double V_DOUBLE::get() const {
  return value;
}



V_STRING::V_STRING(const std::string& v, uint8_t flags) : Value(flags), value(v) { }
  
V_STRING::V_STRING(V_STRING* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_STRING::make_new(const Strings& values) {
  auto ptr = new V_STRING(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_STRING::set_from(Value::Ptr ptr) {
  auto from = ((V_STRING*)ptr);
  flags.store(from->flags);
  value = from->value;
}

void V_STRING::set_from(const Strings& values) {
  value = values.back();
}

Value::Type V_STRING::type() const {
  return value_type;
}

std::string V_STRING::to_string() const {
  return value;
}

std::string V_STRING::get() const {
  return value;
}



V_ENUM::V_ENUM(const int32_t& v, 
               const FromString_t& from_string, 
               const Repr_t& repr,
               uint8_t flags)
              : Value(flags), value(v), 
                call_from_string(from_string), 
                call_repr(repr) {
}

V_ENUM::V_ENUM(V_ENUM* ptr) 
              : Value(ptr), value(ptr->get()),
                call_from_string(ptr->call_from_string), 
                call_repr(ptr->call_repr) {
}

Value::Ptr V_ENUM::make_new(const Strings& values) {
  auto ptr = new V_ENUM(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_ENUM::set_from(Value::Ptr ptr) {
  auto from = ((V_ENUM*)ptr);
  flags.store(from->flags);
  value = from->value;
  call_from_string = from->call_from_string;
  call_repr = from->call_repr;
}

void V_ENUM::set_from(const Strings& values) {
  if(!call_from_string)
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, no from_string cb set", values.back().c_str());
    
  int nv = call_from_string(values.back());
  if(nv < 0) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, no corresponding enum", values.back().c_str());
  value = nv;
}

Value::Type V_ENUM::type() const {
  return value_type;
}

std::string V_ENUM::to_string() const {
  return format(
    "%s  # (%d)", 
    (call_repr ? call_repr(get()).c_str() : "repr not defined"), get());
}

int32_t V_ENUM::get() const {
  return value;
}
  

// lists
V_STRINGS::V_STRINGS(const Strings& v, uint8_t flags)
                    : Value(flags), value(v) { }
  
V_STRINGS::V_STRINGS(V_STRINGS* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_STRINGS::make_new(const Strings& values) {
  auto ptr = new V_STRINGS(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_STRINGS::set_from(Value::Ptr ptr) {
  auto from = ((V_STRINGS*)ptr);
  flags.store(from->flags);
  value = from->value;
}

void V_STRINGS::set_from(const Strings& values) {
  value = values;
}

Value::Type V_STRINGS::type() const {
  return value_type;
}

std::string V_STRINGS::to_string() const {
  return format_list(value);
}

Strings V_STRINGS::get() const {
  return value;
}
  


V_INT64S::V_INT64S(const Int64s& v, uint8_t flags)
                  : Value(flags), value(v) {
}
  
V_INT64S::V_INT64S(V_INT64S* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_INT64S::make_new(const Strings& values) {
  auto ptr = new V_INT64S(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_INT64S::set_from(Value::Ptr ptr) {
  auto from = ((V_INT64S*)ptr);
  flags.store(from->flags);
  value = from->value;
}

void V_INT64S::set_from(const Strings& values) {
  value.clear();
  int64_t v;
  for(const std::string& s: values) {
    from_string(s, &v);
    value.push_back(v);
  }
}

Value::Type V_INT64S::type() const {
  return value_type;
}

std::string V_INT64S::to_string() const {
  return format_list(value);
}

Int64s V_INT64S::get() const {
  return value;
}
  


V_DOUBLES::V_DOUBLES(const Doubles& v, uint8_t flags)
                     : Value(flags), value(v) {
}
  
V_DOUBLES::V_DOUBLES(V_DOUBLES* ptr) : Value(ptr), value(ptr->get()) { }

Value::Ptr V_DOUBLES::make_new(const Strings& values) {
  auto ptr = new V_DOUBLES(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_DOUBLES::set_from(Value::Ptr ptr) {
  auto from = ((V_DOUBLES*)ptr);
  flags.store(from->flags);
  value = from->get();
}

void V_DOUBLES::set_from(const Strings& values) {
  value.clear();
  double v;
  for(const std::string& s: values) {
    from_string(s, &v);
    value.push_back(v);
  }
}

Value::Type V_DOUBLES::type() const {
  return value_type;
}

std::string V_DOUBLES::to_string() const {
  return format_list(value);
}

Doubles V_DOUBLES::get() const {
  return value;
}



// Guarded Atomic
V_GBOOL::V_GBOOL(const bool& v, const V_GBOOL::OnChg_t& cb, uint8_t flags)
                : Value(flags | Value::GUARDED), value(v), on_chg_cb(cb) {
}

V_GBOOL::V_GBOOL(V_GBOOL* ptr) 
                : Value(ptr), 
                  value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}
  
Value::Ptr V_GBOOL::make_new(const Strings& values) {
  auto ptr = new V_GBOOL(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}

void V_GBOOL::set_from(Value::Ptr ptr) {
  auto from = ((V_GBOOL*)ptr);
  flags.store(from->flags);
    
  bool chg = value == from->value;
  value.store(from->value.load());
  on_chg_cb = from->on_chg_cb;
  if(chg)
    on_change();
}
  
void V_GBOOL::set_from(const Strings& values) {
  auto& str = values.back();
  value = str.compare("1") == 0 ||
          strncasecmp(str.data(), "true", 4) == 0 ||
          strncasecmp(str.data(), "yes", 3) == 0;
}

Value::Type V_GBOOL::type() const {
  return value_type;
}

std::string V_GBOOL::to_string() const {
  return value ? "true" : "false";
}

bool V_GBOOL::get() const {
  return value;
}

void V_GBOOL::set(bool v) {
  value.store(v);
}


void V_GBOOL::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void V_GBOOL::set_cb_on_chg(const V_GBOOL::OnChg_t& cb) {
  on_chg_cb = cb;
}



V_GUINT8::V_GUINT8(const uint8_t& v, const V_GUINT8::OnChg_t& cb, uint8_t flags)
                  : Value(flags | Value::GUARDED), value(v), on_chg_cb(cb) {
}

V_GUINT8::V_GUINT8(V_GUINT8* ptr) 
                  : Value(ptr), 
                    value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value::Ptr V_GUINT8::make_new(const Strings& values) {
  auto ptr = new V_GUINT8(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_GUINT8::set_from(Value::Ptr ptr) {
  auto from = ((V_GUINT8*)ptr);
  flags.store(from->flags);
  bool chg = value == from->value;
  value.store(from->value.load());
  on_chg_cb = from->on_chg_cb;
  if(chg)
    on_change();
}
  
void V_GUINT8::set_from(const Strings& values) {
  uint8_t v;
  from_string(values.back(), &v);
  value.store(v);
}

Value::Type V_GUINT8::type() const {
  return value_type;
}

std::string V_GUINT8::to_string() const {
  return std::to_string((int16_t)value);
}

uint8_t V_GUINT8::get() const {
  return value;
}

void V_GUINT8::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void V_GUINT8::set_cb_on_chg(const V_GUINT8::OnChg_t& cb) {
  on_chg_cb = cb;
}
  


V_GINT32::V_GINT32(const int32_t& v, const V_GINT32::OnChg_t& cb, uint8_t flags)
                  : Value(flags | Value::GUARDED), value(v), on_chg_cb(cb) {
}

V_GINT32::V_GINT32(V_GINT32* ptr) 
                  : Value(ptr), 
                    value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value::Ptr V_GINT32::make_new(const Strings& values) {
  auto ptr = new V_GINT32(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_GINT32::set_from(Value::Ptr ptr) {
  auto from = ((V_GINT32*)ptr);
  flags.store(from->flags);
  bool chg = value == from->value;
  value.store(from->value.load());
  on_chg_cb = from->on_chg_cb;
  if(chg)
    on_change();
}

void V_GINT32::set_from(const Strings& values) {
  int32_t v;
  from_string(values.back(), &v);
  value.store(v);
}

Value::Type V_GINT32::type() const {
  return value_type;
}

std::string V_GINT32::to_string() const {
  return std::to_string(value);
}

int32_t V_GINT32::get() const {
  return value;
}

void V_GINT32::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void V_GINT32::set_cb_on_chg(const V_GINT32::OnChg_t& cb) {
  on_chg_cb = cb;
}



V_GENUM::V_GENUM(const int32_t& v, 
                 const V_GENUM::OnChg_t& cb, 
                 const V_GENUM::FromString_t& from_string, 
                 const V_GENUM::Repr_t& repr,
                 uint8_t flags)
                : Value(flags | Value::GUARDED), value(v), 
                  on_chg_cb(cb), 
                  call_from_string(from_string), 
                  call_repr(repr) {
}

V_GENUM::V_GENUM(V_GENUM* ptr) 
                : Value(ptr), value(ptr->get()), 
                  on_chg_cb(ptr->on_chg_cb),
                  call_from_string(ptr->call_from_string), 
                  call_repr(ptr->call_repr) { 
}

Value::Ptr V_GENUM::make_new(const Strings& values) {
  auto ptr = new V_GENUM(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_GENUM::set_from(Value::Ptr ptr) {
  auto from = ((V_GENUM*)ptr);
  flags.store(from->flags);
  bool chg = value == from->value;
  value.store(from->get());
  on_chg_cb = from->on_chg_cb;
  call_from_string = from->call_from_string;
  call_repr = from->call_repr;
  if(chg)
    on_change();
}

void V_GENUM::set_from(const Strings& values) {
  if(!call_from_string)
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, no from_string cb set", values.back().c_str());    
  int nv = call_from_string(values.back());
  if(nv < 0) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, no corresponding enum", values.back().c_str());
  value.store(nv);
}

Value::Type V_GENUM::type() const {
  return value_type;
}

std::string V_GENUM::to_string() const {
  return format(
    "%s  # (%d)", 
    (call_repr ? call_repr(get()).c_str() : "repr not defined"), get());
}

int32_t V_GENUM::get() const {
  return value;
}

void V_GENUM::set(int32_t nv) {
  value.store(nv);
  on_change();
}

void V_GENUM::on_change() const {
  if(on_chg_cb)
    on_chg_cb(get());
}

void V_GENUM::set_cb_on_chg(const V_GENUM::OnChg_t& cb) {
  on_chg_cb = cb;
}



// Guarded Mutex
V_GSTRINGS::V_GSTRINGS(const Strings& v, const V_GSTRINGS::OnChg_t& cb, 
                       uint8_t flags)
                      : Value(flags | Value::GUARDED), value(v), on_chg_cb(cb) {
}

V_GSTRINGS::V_GSTRINGS(V_GSTRINGS* ptr) 
                      : Value(ptr), 
                        value(ptr->get()), on_chg_cb(ptr->on_chg_cb) {
}

Value::Ptr V_GSTRINGS::make_new(const Strings& values) {
  auto ptr = new V_GSTRINGS(this);
  if(!values.empty())
    ptr->set_from(values);
  return ptr;
}
  
void V_GSTRINGS::set_from(Value::Ptr ptr) {
  auto from = ((V_GSTRINGS*)ptr);
  flags.store(from->flags);
  bool chg;
  {
    std::lock_guard lock(mutex);
    chg = value == from->get();
    value = from->get();
    on_chg_cb = from->on_chg_cb;
  }
  on_change();
}

void V_GSTRINGS::set_from(const Strings& values) {
  std::lock_guard lock(mutex);
  value = values;
}

Value::Type V_GSTRINGS::type() const {
  return value_type;
}

std::string V_GSTRINGS::to_string() const {
  std::lock_guard lock(mutex);
  return format_list(value);
}

Strings V_GSTRINGS::get() const {
  std::lock_guard lock(mutex);
  return value;
}

size_t V_GSTRINGS::size() {
  std::lock_guard lock(mutex);	
  return value.size();
}

std::string V_GSTRINGS::get_item(size_t n) {
  std::lock_guard lock(mutex);
  return value[n];
}

void V_GSTRINGS::on_change() const {
  if(on_chg_cb)
    on_chg_cb();
}

void V_GSTRINGS::set_cb_on_chg(const V_GSTRINGS::OnChg_t& cb) {
  std::lock_guard lock(mutex);
  on_chg_cb = cb;
}



}} // namespace SWC::Property

