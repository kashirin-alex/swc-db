/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_Property_h
#define swc_core_config_Property_h

#include <mutex>
#include <atomic>
#include <functional>
#include <vector>

#include "swcdb/core/config/PropertyValueEnumExt.h"
#include "swcdb/core/config/PropertyValueGuarded.h"



namespace SWC {

const uint64_t K = 1000;
const uint64_t KiB = 1024;
const uint64_t M = K * 1000;
const uint64_t MiB = KiB * 1024;
const uint64_t G = M * 1000;
const uint64_t GiB = MiB * 1024;



namespace Property {


/**
 * Convertors & Validators from std::string
*/

inline double double_from_string(const std::string& s){
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

inline int64_t int64_t_from_string(const std::string& s){
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

inline uint16_t uint16_t_from_string(const std::string& s){
  int64_t res = int64_t_from_string(s);

  if (res > UINT16_MAX || res < -UINT16_MAX) 
    HT_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, number out of range of 16-bit integer", s.c_str());
  return (uint16_t)res;
}

inline int32_t int32_t_from_string(const std::string& s){
  int64_t res = int64_t_from_string(s);

  if (res > INT32_MAX || res < INT32_MIN) 
    HT_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, number out of range of 32-bit integer", s.c_str());
  return (int32_t)res;
}

enum ValueType {
  UNKNOWN,
  DOUBLE,
  BOOL,
  STRING,
  UINT16_T,
  INT32_T,
  INT64_T,
  STRINGS,
  INT64S,
  DOUBLES,
  ENUMEXT,
  ENUM,
  
  G_BOOL,
  G_INT32_T,
  G_STRINGS,
  G_ENUMEXT
};

/**
* Property::TypeDef
* a Helper class for casting ValueDef<T> on a single type Pointer
* with a hold for corresponding ValueType enum
 */
class TypeDef {
  public:
  TypeDef(ValueType typ) : typ(typ) {}

  virtual ~TypeDef() {}

  ValueType get_type() { 
    return typ; 
  }

  void set_type(const ValueType t){ 
    if(typ != t)
      typ = t;  
  }

  private:
  ValueType typ;
};
 
/**
* Property::ValueDef
* the Class handle the specific requirments to the 
* value type
 */
template <class T>
class  ValueDef : public TypeDef {
  public:

  static constexpr ValueType get_value_type() {
    if(std::is_enum<T>::value) 
      return ValueType::ENUM;
    
    const std::type_info &v_type = typeid(T);

    if(v_type == typeid(double))
      return ValueType::DOUBLE;

    if(v_type == typeid(bool))
      return ValueType::BOOL;  
    if(v_type == typeid(gBool))
      return ValueType::G_BOOL;

    if(v_type == typeid(std::string))
      return ValueType::STRING;  
    if(v_type == typeid(uint16_t))
      return ValueType::UINT16_T;  

    if(v_type == typeid(int32_t))
      return ValueType::INT32_T; 
    if(v_type == typeid(gInt32t))
      return ValueType::G_INT32_T;

    if(v_type == typeid(int64_t))
      return ValueType::INT64_T;  

    if(v_type == typeid(Strings))
      return ValueType::STRINGS;  
    if(v_type == typeid(gStrings))
      return ValueType::G_STRINGS;

    if(v_type == typeid(Int64s))
      return ValueType::INT64S;  
    if(v_type == typeid(Doubles))
      return ValueType::DOUBLES;

    if(v_type == typeid(EnumExt))
      return ValueType::ENUMEXT;
    if(v_type == typeid(gEnumExt))
      return ValueType::G_ENUMEXT;
    
    return ValueType::UNKNOWN;
  }

  ValueDef(T nv) : TypeDef(get_value_type()) { 
    set_value(nv);  
  }

  ValueDef(const Strings& values, T defaulted) : TypeDef(get_value_type()) { 
    if(!values.empty())
      from_strings(values);
    else
      set_value(defaulted);
  }

  void set_value(T nv) {
    v = nv; 
  }

  void from_strings(const Strings& values) {};

  T get_value() {
    return v;       
  }

  T* get_ptr() { 
    return &v;      
  }

  const std::string str(){
    return "invalid option type";
  }

  std::ostream& operator<<(std::ostream& ostream) {
    return ostream << str();
  }

  operator TypeDef*() { 
    return this;
  }

  // ValueDef<T> operator *()   { return this;    }
  virtual ~ValueDef()   {}

  private:
  T v;
};


template <>
inline void ValueDef<EnumExt>::from_strings(const Strings& values){
  get_ptr()->from_string(values.back());
}

template <>
inline void ValueDef<gEnumExt>::from_strings(const Strings& values){
  get_ptr()->from_string(values.back());
}

template <>
inline ValueDef<EnumExt>::ValueDef(const Strings& values, EnumExt defaulted)
                            : TypeDef(ValueType::ENUMEXT) {
  get_ptr()->set_from(defaulted);  // assign call functions if set
  if(!values.empty())
    from_strings(values);
}

template <>
inline ValueDef<gEnumExt>::ValueDef(const Strings& values, gEnumExt defaulted) 
                            : TypeDef(ValueType::G_ENUMEXT) {
  get_ptr()->set_from(defaulted);  // assign call functions if set
  if(!values.empty())
    from_strings(values);
}


/* set_value from_strings */

template <>
inline void ValueDef<bool>::from_strings(const Strings& values) {
  bool res;
  std::string str = values.back();
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  res = (str.compare("1")==0)||(str.compare("true")==0)||(str.compare("yes")==0);
  set_value(res);
}

template <>
inline void ValueDef<gBool>::from_strings(const Strings& values) {
  std::string str = values.back();
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  set_value(str.compare("1") == 0 || 
            str.compare("true") == 0 || 
            str.compare("yes") == 0);
}

template <>
inline void ValueDef<std::string>::from_strings(const Strings& values){
  set_value(values.back());
}

template <>
inline void ValueDef<double>::from_strings(const Strings& values) {
  set_value(double_from_string(values.back()));
}

template <>
inline void ValueDef<uint16_t>::from_strings(const Strings& values) {
  set_value(uint16_t_from_string(values.back()));
}

template <>
inline void ValueDef<int32_t>::from_strings(const Strings& values) {
  set_value(int32_t_from_string(values.back()));
}

template <>
inline void ValueDef<gInt32t>::from_strings(const Strings& values) {
  set_value(int32_t_from_string(values.back()));
}

template <>
inline void ValueDef<int64_t>::from_strings(const Strings& values) {
  set_value(int64_t_from_string(values.back()));
}

template <>
inline void ValueDef<Doubles>::from_strings(const Strings& values){
  Doubles value;
  for(const std::string& s: values)
    value.push_back(double_from_string(s));
  set_value(value);
}

template <>
inline void ValueDef<Int64s>::from_strings(const Strings& values){
  Int64s value;
  for(const std::string& s: values)
    value.push_back(int64_t_from_string(s));
  set_value(value);
}

template <>
inline void ValueDef<Strings>::from_strings(const Strings& values){
  set_value(values);
}

template <>
inline void ValueDef<gStrings>::from_strings(const Strings& values){
  set_value(values);
}

/* return string representation */
template <>
inline const std::string ValueDef<bool>::str(){
  return get_value() ? "true" : "false";;
}
template <>
inline const std::string ValueDef<gBool>::str(){
  return (bool)get_value() ? "true" : "false";;
}
template <>
inline const std::string ValueDef<std::string>::str(){
  return get_value();
}
template <>
inline const std::string ValueDef<double>::str(){
  return format("%g", get_value());
}
template <>
inline const std::string ValueDef<uint16_t>::str(){
  return format("%u", (unsigned)get_value());
}
template <>
inline const std::string ValueDef<int32_t>::str(){
  return format("%d", get_value());
}
template <>
inline const std::string ValueDef<gInt32t>::str(){
  return format("%d", (int32_t)get_value());
}
template <>
inline const std::string ValueDef<int64_t>::str(){
  return format("%ld", get_value());
}
template <>
inline const std::string ValueDef<Doubles>::str(){
  return format_list(get_value());
}
template <>
inline const std::string ValueDef<Int64s>::str(){
  return format_list(get_value());
}
template <>
inline const std::string ValueDef<Strings>::str(){
  return format_list(get_value());
}
template <>
inline const std::string ValueDef<gStrings>::str(){
  return format_list((Strings)get_value());
}
template <>
inline const std::string ValueDef<EnumExt>::str(){
  return v.to_str();
}
template <>
inline const std::string ValueDef<gEnumExt>::str(){
  return v.to_str();
}

    
/**
* Property::Value 
* holds:
*   * the pointer to TypeDef and short forms to ValueDef operations
*   * the whether the value is a default value or skippable
 */

class Value {
  public:

  typedef Value* Ptr;

  static Ptr make_new(Value::Ptr p, const Strings& values = Strings()) {
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
  
  template<typename T>
  Value(T v, bool skippable=false, bool guarded=false) {
    m_skippable = skippable;
    m_guarded = guarded;
    set_value(v);
  }
    
    /* init from (TypeDef*)ValueDef<T> */
  Value(TypeDef* v) {
    type_ptr = v;
  }

  virtual ~Value() {
    if(type_ptr)
      delete type_ptr;
  }
    
  // update/set the new value to the ValueDef<T>
  template<typename T>
  void set_value(T v) {
    if(type_ptr == nullptr){
      type_ptr = new ValueDef<T>(v);
      return;
    } 
    ((ValueDef<T>*)type_ptr)->set_value(v);
  }

  /* set value from from other Value::Ptr */
  void set_value_from(Value::Ptr from) {
    switch(get_type()){
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

  template<typename T>
  T get() const {
    if (type_ptr == nullptr)
      HT_THROWF(Error::CONFIG_GET_ERROR, 
                "T get(): type=%s (UNKNOWN VALUE TYPE)", typeid(T).name());;

    return ((ValueDef<T>*)type_ptr)->get_value();
  }

  template<typename T>
  T* get_ptr() const {
    return ((ValueDef<T>*)type_ptr)->get_ptr();
  }

  TypeDef* get_type_ptr() {
    return type_ptr;
  }

  const ValueType get_type() const {
    return type_ptr->get_type();
  }
    
  /* a Default Value */
  Ptr default_value(bool defaulted=true){
    m_defaulted = defaulted;
    return *this;
  }

  const bool is_default() const {
    return m_defaulted;
  }

  /* a Skippable property (no default value) */
  const bool is_skippable() const {
    return m_skippable;
  }
    
  /* a Zero Token Type */
  Ptr zero_token() {
    m_no_token = true;
    return *this;
  }
  
  const bool is_zero_token() const {
    return m_no_token;
  }

  /* a Guarded Type */
  const bool is_guarded() const {
    return m_guarded;
  }

  void guarded(bool guarded){
      m_guarded = guarded;
  }

  operator Value*() { 
    return this;
  }

  /* represent value in string */
  const std::string str() {
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
  private:

  TypeDef* type_ptr = nullptr;

  std::atomic<bool> m_defaulted = false;
  std::atomic<bool> m_no_token = false;
  std::atomic<bool> m_skippable = false;
  std::atomic<bool> m_guarded = false;

};



}} // namespace SWC::Property

#endif // swc_core_config_Property_h
