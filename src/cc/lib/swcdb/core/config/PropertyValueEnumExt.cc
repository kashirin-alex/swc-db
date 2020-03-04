/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */



#include <mutex>
#include <atomic>
#include <functional>
#include <vector>

#include "swcdb/core/Error.h"
#include "swcdb/core/config/PropertyValueEnumExt.h"


namespace SWC { namespace Property {


ValueEnumExtBase::operator int() { 
  return get(); 
}

ValueEnumExtBase::operator std::string() { 
  return str(); 
}

ValueEnumExtBase& ValueEnumExtBase::set_from_string(const FromString_t& cb) {
  call_from_string = cb;
  cb_set = true;
  return *this;
}
    
ValueEnumExtBase& ValueEnumExtBase::set_repr(const Repr_t& cb) {
  call_repr = cb;
  cb_set = true;
  return *this;
}

int ValueEnumExtBase::from_string(const std::string& opt) {
  int nv = get_call_from_string()(opt);
  if(nv > -1)
    set_value(nv);
  else {
    if(get() == -1)
      SWC_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s, no corresponding enum", opt.c_str());
    else
      SWC_LOGF(LOG_WARN, "Bad cfg Value %s, no corresponding enum", opt.c_str());
  }
  return get();
}
    
void ValueEnumExtBase::set_default_calls() {
  call_from_string = [](const std::string& opt) {
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, no from_string cb set", opt.c_str());
    return -1;
  };
  call_repr = [](int v){ return "No repr cb defined!"; };
}

const ValueEnumExtBase::FromString_t& 
ValueEnumExtBase::get_call_from_string() {
  if(!cb_set) 
    set_default_calls();
  return call_from_string;
}

const ValueEnumExtBase::Repr_t& 
ValueEnumExtBase::get_call_repr() {
  if(!cb_set) 
    set_default_calls();
  return call_repr;
}

const std::string ValueEnumExtBase::str() {
  return get_call_repr()(get());
}

const std::string ValueEnumExtBase::to_str() {
  return format("%s  # (%d)", get_call_repr()(get()).c_str(), get());
}


//
ValueEnumExt::ValueEnumExt(int nv)
                           : value(nv) { 
}

ValueEnumExt::ValueEnumExt(int nv, 
                           const FromString_t& from_string, 
                           const Repr_t& repr)
                          : value(nv) {
  set_from_string(from_string);
  set_repr(repr);
}

ValueEnumExt::ValueEnumExt(ValueEnumExt& other) 
                          : value(other.get()) {
  set_from(other); 
}

ValueEnumExt& ValueEnumExt::operator=(ValueEnumExt& other) {
  set_from(other); 
  set_value(other.get());
  return *this; 
}

void ValueEnumExt::set_from(ValueEnumExt &other) {
  if(other.cb_set) {
    set_repr(other.get_call_repr());
    set_from_string(other.get_call_from_string());
  }
}

void ValueEnumExt::set_value(int nv) {
  value = nv;
}

int ValueEnumExt::get() {
  return value; 
}

ValueEnumExt::~ValueEnumExt() {};


//
ValueGuardedEnumExt::ValueGuardedEnumExt(int nv) : value(nv) { }

ValueGuardedEnumExt::ValueGuardedEnumExt(int nv, 
                                         const OnChg_t& cb,
                                         const FromString_t& from_string, 
                                         const Repr_t& repr)
                                         : value(nv), on_chg_cb(cb) {
  set_from_string(from_string);
  set_repr(repr);
}

ValueGuardedEnumExt::ValueGuardedEnumExt(ValueGuardedEnumExt& other) 
                                        : value(other.get()) {
  set_from(other); 
}

ValueGuardedEnumExt& ValueGuardedEnumExt::operator=(ValueGuardedEnumExt& other) {
  set_from(other); 
  set_value(other.get());
  return *this; 
}

void ValueGuardedEnumExt::set_from(ValueGuardedEnumExt& other) {
  if(other.on_chg_cb)
    set_cb_on_chg(other.on_chg_cb);
  if(other.cb_set) {
    set_repr(other.get_call_repr());
    set_from_string(other.get_call_from_string());
  }
  set_value(other.get());
}

void ValueGuardedEnumExt::set_value(int nv) {
  if(nv == -1 || nv == get()) 
    return;
        
  value.store(nv);
  if(on_chg_cb)
    on_chg_cb(get());
}

int ValueGuardedEnumExt::get() { 
  return value.load(); 
}

void ValueGuardedEnumExt::set_cb_on_chg(const OnChg_t& cb) {
  on_chg_cb = cb;
}

ValueGuardedEnumExt::~ValueGuardedEnumExt () {};


}} // namespace SWC::Property