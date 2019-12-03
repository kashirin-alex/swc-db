/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include <mutex>
#include <atomic>
#include <functional>
#include <vector>

#include "swcdb/core/Error.h"
#include "swcdb/core/config/Property.h"


namespace SWC { namespace Property {

double double_from_string(const std::string& s){
  char *last;
  double res = strtod(s.c_str(), &last);

  if (s.c_str() == last)
    HT_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s.c_str());

  switch (*last) {
    case 'k':
    case 'K': res *= 1000LL;         break;
    case 'm':
    case 'M': res *= 1000000LL;      break;
    case 'g':
    case 'G': res *= 1000000000LL;   break;
    case '\0':                          break;
    default: 
      HT_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s unknown suffix %s", s.c_str(), last);
  }
  return res;
}

int64_t int64_t_from_string(const std::string& s){
  char *last;
  int64_t res = strtoll(s.c_str(), &last, 0);

  if (s.c_str() == last)
    HT_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s.c_str());

  if (res > INT64_MAX || res < INT64_MIN) 
    HT_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, number out of range of 64-bit integer", s.c_str());
  
  switch (*last) {
    case 'k':
    case 'K': res *= 1000LL;         break;
    case 'm':
    case 'M': res *= 1000000LL;      break;
    case 'g':
    case 'G': res *= 1000000000LL;   break;
    case '\0':                          break;
    default: 
      HT_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s unknown suffix %s", s.c_str(), last);
  }
  return res;
}

uint16_t uint16_t_from_string(const std::string& s){
  int64_t res = int64_t_from_string(s);

  if (res > UINT16_MAX || res < -UINT16_MAX) 
    HT_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, number out of range of 16-bit integer", s.c_str());
  return (uint16_t)res;
}

int32_t int32_t_from_string(const std::string& s){
  int64_t res = int64_t_from_string(s);

  if (res > INT32_MAX || res < INT32_MIN) 
    HT_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, number out of range of 32-bit integer", s.c_str());
  return (int32_t)res;
}


template <>
void ValueDef<EnumExt>::from_strings(const Strings& values){
  get_ptr()->from_string(values.back());
}

template <>
void ValueDef<gEnumExt>::from_strings(const Strings& values){
  get_ptr()->from_string(values.back());
}

template <>
ValueDef<EnumExt>::ValueDef(const Strings& values, EnumExt defaulted)
                            : TypeDef(ValueType::ENUMEXT) {
  get_ptr()->set_from(defaulted);  // assign call functions if set
  if(!values.empty())
    from_strings(values);
}

template <>
ValueDef<gEnumExt>::ValueDef(const Strings& values, gEnumExt defaulted) 
                            : TypeDef(ValueType::G_ENUMEXT) {
  get_ptr()->set_from(defaulted);  // assign call functions if set
  if(!values.empty())
    from_strings(values);
}


/* set_value from_strings */

template <>
void ValueDef<bool>::from_strings(const Strings& values) {
  bool res;
  std::string str = values.back();
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  res = (str.compare("1")==0)||(str.compare("true")==0)||(str.compare("yes")==0);
  set_value(res);
}

template <>
void ValueDef<gBool>::from_strings(const Strings& values) {
  std::string str = values.back();
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  set_value(str.compare("1") == 0 || 
            str.compare("true") == 0 || 
            str.compare("yes") == 0);
}

template <>
void ValueDef<std::string>::from_strings(const Strings& values){
  set_value(values.back());
}

template <>
void ValueDef<double>::from_strings(const Strings& values) {
  set_value(double_from_string(values.back()));
}

template <>
void ValueDef<uint16_t>::from_strings(const Strings& values) {
  set_value(uint16_t_from_string(values.back()));
}

template <>
void ValueDef<int32_t>::from_strings(const Strings& values) {
  set_value(int32_t_from_string(values.back()));
}

template <>
void ValueDef<gInt32t>::from_strings(const Strings& values) {
  set_value(int32_t_from_string(values.back()));
}

template <>
void ValueDef<int64_t>::from_strings(const Strings& values) {
  set_value(int64_t_from_string(values.back()));
}

template <>
void ValueDef<Doubles>::from_strings(const Strings& values){
  Doubles value;
  for(const std::string& s: values)
    value.push_back(double_from_string(s));
  set_value(value);
}

template <>
void ValueDef<Int64s>::from_strings(const Strings& values){
  Int64s value;
  for(const std::string& s: values)
    value.push_back(int64_t_from_string(s));
  set_value(value);
}

template <>
void ValueDef<Strings>::from_strings(const Strings& values){
  set_value(values);
}

template <>
void ValueDef<gStrings>::from_strings(const Strings& values){
  set_value(values);
}

/* return string representation */
template <>
const std::string ValueDef<bool>::str(){
  return get_value() ? "true" : "false";;
}
template <>
const std::string ValueDef<gBool>::str(){
  return (bool)get_value() ? "true" : "false";;
}
template <>
const std::string ValueDef<std::string>::str(){
  return get_value();
}
template <>
const std::string ValueDef<double>::str(){
  return format("%g", get_value());
}
template <>
const std::string ValueDef<uint16_t>::str(){
  return format("%u", (unsigned)get_value());
}
template <>
const std::string ValueDef<int32_t>::str(){
  return format("%d", get_value());
}
template <>
const std::string ValueDef<gInt32t>::str(){
  return format("%d", (int32_t)get_value());
}
template <>
const std::string ValueDef<int64_t>::str(){
  return format("%ld", get_value());
}
template <>
const std::string ValueDef<Doubles>::str(){
  return format_list(get_value());
}
template <>
const std::string ValueDef<Int64s>::str(){
  return format_list(get_value());
}
template <>
const std::string ValueDef<Strings>::str(){
  return format_list(get_value());
}
template <>
const std::string ValueDef<gStrings>::str(){
  return format_list((Strings)get_value());
}
template <>
const std::string ValueDef<EnumExt>::str(){
  return v.to_str();
}
template <>
const std::string ValueDef<gEnumExt>::str(){
  return v.to_str();
}


Value::Ptr Value::make_new(Value::Ptr p, const Strings& values) {
  switch(p->get_type()){

      case ValueType::STRING:
        return new Value(
          (TypeDef*)new ValueDef<std::string>(values, p->get<std::string>()));

      case ValueType::BOOL: 
        return new Value(
          (TypeDef*)new ValueDef<bool>(values, p->get<bool>()));
      case ValueType::G_BOOL: 
        return new Value(
          (TypeDef*)new ValueDef<gBool>(values, p->get<gBool>()));

      case ValueType::DOUBLE:
        return new Value(
          (TypeDef*)new ValueDef<double>(values, p->get<double>()));

      case ValueType::UINT16_T:
        return new Value(
          (TypeDef*)new ValueDef<uint16_t>(values, p->get<uint16_t>()));

      case ValueType::INT32_T:
        return new Value(
          (TypeDef*)new ValueDef<int32_t>(values, p->get<int32_t>()));
      case ValueType::G_INT32_T: 
        return new Value(
          (TypeDef*)new ValueDef<gInt32t>(values, p->get<gInt32t>()));

      case ValueType::INT64_T:
        return new Value(
          (TypeDef*)new ValueDef<int64_t>(values, p->get<int64_t>()));

      case ValueType::STRINGS:
        return new Value(
          (TypeDef*)new ValueDef<Strings>(values, p->get<Strings>()));
      case ValueType::G_STRINGS:
        return new Value(
          (TypeDef*)new ValueDef<gStrings>(values, p->get<gStrings>()));

      case ValueType::INT64S:
        return new Value(
          (TypeDef*)new ValueDef<Int64s>(values, p->get<Int64s>()));

      case ValueType::DOUBLES:
        return new Value(
          (TypeDef*)new ValueDef<Doubles>(values, p->get<Doubles>()));

      case ValueType::ENUMEXT:
        return new Value(
          (TypeDef*)new ValueDef<EnumExt>(values, p->get<EnumExt>()));
      case ValueType::G_ENUMEXT:
        return new Value(
          (TypeDef*)new ValueDef<gEnumExt>(values, p->get<gEnumExt>()));
      
      case ValueType::ENUM:
      default:
        HT_THROWF(Error::CONFIG_GET_ERROR, 
                  "Bad Type for values %s", format_list(values).c_str());
  }
}  

/* init from (TypeDef*)ValueDef<T> */
Value::Value(TypeDef* v) {
  type_ptr = v;
}

Value::~Value() {
  if(type_ptr)
    delete type_ptr;
}

/* set value from from other Value::Ptr */
void Value::set_value_from(Value::Ptr from) {
  switch(get_type()) {
      case ValueType::STRING:
        return set_value(from->get<std::string>());

      case ValueType::BOOL: 
        return set_value(from->get<bool>());
      case ValueType::G_BOOL: 
        return set_value(from->get<gBool>());

      case ValueType::DOUBLE:
        return set_value(from->get<double>());
      case ValueType::UINT16_T:
        return set_value(from->get<uint16_t>());

      case ValueType::INT32_T:
        return set_value(from->get<int32_t>());
      case ValueType::G_INT32_T: 
        return set_value(from->get<gInt32t>());

      case ValueType::INT64_T:
        return set_value(from->get<int64_t>());

      case ValueType::STRINGS:
        return set_value(from->get<Strings>());
      case ValueType::G_STRINGS:
        return set_value(from->get<gStrings>());

      case ValueType::INT64S:
        return set_value(from->get<Int64s>());
      case ValueType::DOUBLES:
        return set_value(from->get<Doubles>());

      case ValueType::ENUMEXT:
        return set_value(from->get<EnumExt>());
      case ValueType::G_ENUMEXT:
        return set_value(from->get<gEnumExt>());

      case ValueType::ENUM:
      default:
        HT_THROWF(Error::CONFIG_GET_ERROR, "Bad Type %s", str().c_str());
  }
}

TypeDef* Value::get_type_ptr() {
  return type_ptr;
}

const ValueType Value::get_type() const {
  return type_ptr->get_type();
}
    
void Value::getting_error(const char* tname) const {
  HT_THROWF(Error::CONFIG_GET_ERROR, 
            "T get(): type=%s (UNKNOWN VALUE TYPE)", tname);
}

/* a Default Value */
Value::Ptr Value::default_value(bool defaulted){
  m_defaulted = defaulted;
  return *this;
}

const bool Value::is_default() const {
  return m_defaulted;
}

/* a Skippable property (no default value) */
const bool Value::is_skippable() const {
  return m_skippable;
}
    
/* a Zero Token Type */
Value::Ptr Value::zero_token() {
  m_no_token = true;
  return *this;
}
  
const bool Value::is_zero_token() const {
  return m_no_token;
}

/* a Guarded Type */
const bool Value::is_guarded() const {
  return m_guarded;
}

void Value::guarded(bool guarded){
  m_guarded = guarded;
}

Value::operator Value*() { 
  return this;
}

/* represent value in string */
const std::string Value::str() {
  if (type_ptr == nullptr)
    return "nullptr";
      
  switch(get_type()) {

      case ValueType::STRING:
        return ((ValueDef<std::string>*)type_ptr)->str();

      case ValueType::BOOL: 
        return ((ValueDef<bool>*)type_ptr)->str();
      case ValueType::G_BOOL: 
        return ((ValueDef<gBool>*)type_ptr)->str();

      case ValueType::DOUBLE:
        return ((ValueDef<double>*)type_ptr)->str();

      case ValueType::UINT16_T:
        return ((ValueDef<uint16_t>*)type_ptr)->str();

      case ValueType::INT32_T:
        return ((ValueDef<int32_t>*)type_ptr)->str();
      case ValueType::G_INT32_T: 
        return ((ValueDef<gInt32t>*)type_ptr)->str();

      case ValueType::INT64_T:
        return ((ValueDef<int64_t>*)type_ptr)->str();

      case ValueType::STRINGS:
        return ((ValueDef<Strings>*)type_ptr)->str();
      case ValueType::G_STRINGS: 
        return ((ValueDef<gStrings>*)type_ptr)->str();

      case ValueType::INT64S:
        return ((ValueDef<Int64s>*)type_ptr)->str();
      case ValueType::DOUBLES:
        return ((ValueDef<Doubles>*)type_ptr)->str();

      case ValueType::ENUMEXT:
        return ((ValueDef<EnumExt>*)type_ptr)->str();
      case ValueType::G_ENUMEXT:
        return ((ValueDef<gEnumExt>*)type_ptr)->str();

      case ValueType::ENUM:
        return "An ENUM TYPE";
      default:
        return "invalid option type";
  }
}




}} // namespace SWC::Property
