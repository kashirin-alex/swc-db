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

double double_from_string(const std::string& s);

int64_t int64_t_from_string(const std::string& s);

uint8_t uint8_t_from_string(const std::string& s);

uint16_t uint16_t_from_string(const std::string& s);

int32_t int32_t_from_string(const std::string& s);

enum ValueType {
  UNKNOWN,
  DOUBLE,
  BOOL,
  STRING,
  UINT8_T,
  UINT16_T,
  INT32_T,
  INT64_T,
  STRINGS,
  INT64S,
  DOUBLES,
  ENUMEXT,
  ENUM,
  
  G_BOOL,
  G_UINT8_T,
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

    if(v_type == typeid(uint8_t))
      return ValueType::UINT8_T;  
    if(v_type == typeid(gInt8t))
      return ValueType::G_UINT8_T;

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
void ValueDef<EnumExt>::from_strings(const Strings& values);

template <>
void ValueDef<gEnumExt>::from_strings(const Strings& values);

template <>
ValueDef<EnumExt>::ValueDef(const Strings& values, EnumExt defaulted);

template <>
ValueDef<gEnumExt>::ValueDef(const Strings& values, gEnumExt defaulted);


/* set_value from_strings */

template <>
void ValueDef<bool>::from_strings(const Strings& values);

template <>
void ValueDef<gBool>::from_strings(const Strings& values);

template <>
void ValueDef<std::string>::from_strings(const Strings& values);

template <>
void ValueDef<double>::from_strings(const Strings& values);

template <>
void ValueDef<uint8_t>::from_strings(const Strings& values);

template <>
void ValueDef<gInt8t>::from_strings(const Strings& values);

template <>
void ValueDef<uint16_t>::from_strings(const Strings& values);

template <>
void ValueDef<int32_t>::from_strings(const Strings& values);

template <>
void ValueDef<gInt32t>::from_strings(const Strings& values);

template <>
void ValueDef<int64_t>::from_strings(const Strings& values);

template <>
void ValueDef<Doubles>::from_strings(const Strings& values);

template <>
void ValueDef<Int64s>::from_strings(const Strings& values);

template <>
void ValueDef<Strings>::from_strings(const Strings& values);

template <>
void ValueDef<gStrings>::from_strings(const Strings& values);

/* return string representation */
template <>
const std::string ValueDef<bool>::str();
template <>
const std::string ValueDef<gBool>::str();
template <>
const std::string ValueDef<std::string>::str();
template <>
const std::string ValueDef<double>::str();
template <>
const std::string ValueDef<uint8_t>::str();
template <>
const std::string ValueDef<gInt8t>::str();
template <>
const std::string ValueDef<uint16_t>::str();
template <>
const std::string ValueDef<int32_t>::str();
template <>
const std::string ValueDef<gInt32t>::str();
template <>
const std::string ValueDef<int64_t>::str();
template <>
const std::string ValueDef<Doubles>::str();
template <>
const std::string ValueDef<Int64s>::str();
template <>
const std::string ValueDef<Strings>::str();
template <>
const std::string ValueDef<gStrings>::str();
template <>
const std::string ValueDef<EnumExt>::str();
template <>
const std::string ValueDef<gEnumExt>::str();

    
/**
* Property::Value 
* holds:
*   * the pointer to TypeDef and short forms to ValueDef operations
*   * the whether the value is a default value or skippable
 */

class Value final {
  public:

  typedef Value* Ptr;

  static Ptr make_new(Value::Ptr p, const Strings& values = Strings());
  
  template<typename T>
  Value(T v, bool skippable=false, bool guarded=false)
        : m_skippable(skippable), m_guarded(guarded) {
    set_value(v);
  }
    
    /* init from (TypeDef*)ValueDef<T> */
  Value(TypeDef* v);

  ~Value();
    
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
  void set_value_from(Value::Ptr from);

  template<typename T>
  T get() const {
    if (type_ptr == nullptr)
      getting_error(typeid(T).name());

    return ((ValueDef<T>*)type_ptr)->get_value();
  }

  template<typename T>
  T* get_ptr() const {
    return ((ValueDef<T>*)type_ptr)->get_ptr();
  }

  TypeDef* get_type_ptr();

  const ValueType get_type() const;

  void getting_error(const char* tname) const;

  /* a Default Value */
  Ptr default_value(bool defaulted=true);

  const bool is_default() const;

  /* a Skippable property (no default value) */
  const bool is_skippable() const;
    
  /* a Zero Token Type */
  Ptr zero_token();
  
  const bool is_zero_token() const;

  /* a Guarded Type */
  const bool is_guarded() const;

  void guarded(bool guarded);

  operator Value*();

  /* represent value in string */
  const std::string str();
  private:

  TypeDef* type_ptr = nullptr;

  std::atomic<bool> m_defaulted = false;
  std::atomic<bool> m_no_token = false;
  std::atomic<bool> m_skippable = false;
  std::atomic<bool> m_guarded = false;

};



}} // namespace SWC::Property


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Property.cc"
#endif 

#endif // swc_core_config_Property_h
