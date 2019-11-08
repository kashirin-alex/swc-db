/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_Property_h
#define swc_core_config_Property_h

#include "PropertyValueEnumExt.h"
#include "PropertyValueGuarded.h"

#include "../Compat.h"
#include "../String.h"
#include "../Logger.h"
#include "../Error.h"

// #include <string>
#include <atomic>
#include <mutex>


namespace SWC {

/** @addtogroup Common
 *  @{
 */

const uint64_t K = 1000;
const uint64_t KiB = 1024;
const uint64_t M = K * 1000;
const uint64_t MiB = KiB * 1024;
const uint64_t G = M * 1000;
const uint64_t GiB = MiB * 1024;



namespace Property {

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

const ValueType get_value_type(const std::type_info &v_type);

/**
 * Convertors & Validators from String
*/
double double_from_string(const String& s);
int64_t int64_t_from_string(const String& s);
uint16_t uint16_t_from_string(const String& s);
int32_t int32_t_from_string(const String& s);

/**
* Property::TypeDef
* a Helper class for casting ValueDef<T> on a single type Pointer
* with a hold for corresponding ValueType enum
 */
class TypeDef {
  public:
    ValueType get_type()  { return typ; }
    void set_type(const ValueType t){ 
      if(typ != t)
        typ = t;  
    }
    virtual ~TypeDef() {}
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
    static constexpr bool is_enum() {  
      return std::is_enum<T>::value; 
    }

    ValueDef(ValueType typ, const Strings& values, T defaulted) {
      set_type(typ);
      if(!values.empty())
        from_strings(values);
      else
        set_value(defaulted);
    }

    ValueDef(T nv) { 
      if(is_enum())
        set_type(ValueType::ENUM);
      else
        set_type(get_value_type(typeid(T)));
      set_value(nv);  
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

    const String str(){
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
ValueDef<EnumExt>::ValueDef(ValueType typ, const Strings& values, 
                            EnumExt defaulted);
template <>
ValueDef<gEnumExt>::ValueDef(ValueType typ, const Strings& values, 
                              gEnumExt defaulted);


/* set_value from_strings */
template <>
void ValueDef<bool>::from_strings(const Strings& values);
template <>
void ValueDef<gBool>::from_strings(const Strings& values);
template <>
void ValueDef<String>::from_strings(const Strings& values);
template <>
void ValueDef<double>::from_strings(const Strings& values);
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
template <>
void ValueDef<EnumExt>::from_strings(const Strings& values);
template <>
void ValueDef<gEnumExt>::from_strings(const Strings& values);

/* return string representation */
template <>
const String ValueDef<bool>::str();
template <>
const String ValueDef<gBool>::str();
template <>
const String ValueDef<String>::str();
template <>
const String ValueDef<double>::str();
template <>
const String ValueDef<uint16_t>::str();
template <>
const String ValueDef<int32_t>::str();
template <>
const String ValueDef<gInt32t>::str();
template <>
const String ValueDef<int64_t>::str();
template <>
const String ValueDef<Doubles>::str();
template <>
const String ValueDef<Int64s>::str();
template <>
const String ValueDef<Strings>::str();
template <>
const String ValueDef<gStrings>::str();
template <>
const String ValueDef<EnumExt>::str();
template <>
const String ValueDef<gEnumExt>::str();


    
/**
* Property::Value 
* holds:
*   * the pointer to TypeDef and short forms to ValueDef operations
*   * the whether the value is a default value or skippable
 */

class Value;
typedef Value* ValuePtr;

class Value {
  public:

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
    
    // update/set the new value to the ValueDef<T>
    template<typename T>
    void set_value(T v) {
      if(type_ptr == nullptr){
        type_ptr = new ValueDef<T>(v);
        return;
      } 
      ((ValueDef<T>*)type_ptr)->set_value(v);
    }

    /* set value from from other ValuePtr */
    void set_value_from(ValuePtr from);

    template<typename T>
    T get() {
      if (type_ptr == nullptr)
        HT_THROWF(Error::CONFIG_GET_ERROR, 
                  "T get(): type=%s (UNKNOWN VALUE TYPE)", typeid(T).name());;

      return ((ValueDef<T>*)type_ptr)->get_value();
    }

    template<typename T>
    T* get_ptr() {
      return ((ValueDef<T>*)type_ptr)->get_ptr();
    }

    TypeDef* get_type_ptr() {
      return type_ptr;
    }

    ValueType get_type() {
      return type_ptr->get_type();
    }
    
    /* represent value in string */
    const String str();
    
    /* a Default Value */
    ValuePtr default_value(bool defaulted=true){
      m_defaulted = defaulted;
      return *this;
    }

    bool is_default() {
      return m_defaulted;
    }

    /* a Skippable property (no default value) */
    bool is_skippable() {
      return m_skippable;
    }
    
    /* a Zero Token Type */
    ValuePtr zero_token(){
      m_no_token = true;
      return *this;
    }
    bool is_zero_token(){
      return m_no_token;
    }

    /* a Guarded Type */
    bool is_guarded(){
      return m_guarded;
    }
    void guarded(bool guarded){
      m_guarded = guarded;
    }

    operator Value*() { 
      return this;
    }

    virtual ~Value() {}

  private:

    TypeDef* type_ptr = nullptr; //

    std::atomic<bool> m_defaulted = false;
    std::atomic<bool> m_no_token = false;
    std::atomic<bool> m_skippable = false;
    std::atomic<bool> m_guarded = false;

};


ValuePtr make_new(ValuePtr p, Strings values = Strings());



} // namespace Property

/** @} */

} // namespace SWC

#endif // swc_core_config_Property_h
