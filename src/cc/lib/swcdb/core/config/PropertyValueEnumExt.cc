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


void ValueEnumExtBase::set_value(int nv){}

int ValueEnumExtBase::get() { return -1; }

ValueEnumExtBase* ValueEnumExtBase::operator =(int nv){
  set_value(nv);
  return *this;
}

bool ValueEnumExtBase::operator==(ValueEnumExtBase a) { return get() == a.get(); }
bool ValueEnumExtBase::operator!=(ValueEnumExtBase a) { return get() != a.get(); }

ValueEnumExtBase::operator ValueEnumExtBase*()  { return this;  }
ValueEnumExtBase::operator int()                { return get(); }
ValueEnumExtBase::operator std::string()        { return str(); }
    

ValueEnumExtBase* ValueEnumExtBase::operator =(ValueEnumExtBase other){
  set_from(other);
  return *this;
}

void ValueEnumExtBase::set_from(ValueEnumExtBase &other){
  if((int)other != -1)
    set_value((int)other);
  if(!other.cb_set) 
    return;
      
  set_repr(other.get_call_repr());
  set_from_string(other.get_call_from_string());
}

ValueEnumExtBase& ValueEnumExtBase::set_from_string(std::function<int(std::string)> cb) {
  call_from_string = cb;
  cb_set = true;
  return *this;
}
    
ValueEnumExtBase& ValueEnumExtBase::set_repr(std::function<std::string(int)> cb) {
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

std::function<int(std::string)> ValueEnumExtBase::get_call_from_string(){
  if(!cb_set) set_default_calls();
  return call_from_string;
}

std::function<std::string(int)> ValueEnumExtBase::get_call_repr(){
  if(!cb_set) set_default_calls();
  return call_repr;
}

const std::string ValueEnumExtBase::str() {
  return get_call_repr()(get());
}

const std::string ValueEnumExtBase::to_str() {
  return format("%s  # (%d)", get_call_repr()(get()).c_str(), get());
}

ValueEnumExtBase::~ValueEnumExtBase() {};



ValueEnumExt::ValueEnumExt(int nv) { 
  set_value(nv);
}

void ValueEnumExt::set_value(int nv) {
  value = nv;
}

int ValueEnumExt::get() {
  return value; 
}

ValueEnumExt::~ValueEnumExt () {};


ValueGuardedEnumExt::ValueGuardedEnumExt(int nv) {
  set_value(nv);
}

ValueGuardedEnumExt::ValueGuardedEnumExt(ValueGuardedEnumExt& other) {
  set_from(other);
}
    
ValueGuardedEnumExt& ValueGuardedEnumExt::operator=(ValueGuardedEnumExt& other) {
  set_from(other); 
  return *this; 
}

void ValueGuardedEnumExt::set_value(int nv) {
  if(nv == get()) 
    return;
        
  value.store(nv);
  if(on_chg_cb)
    on_chg_cb(get());
}

int ValueGuardedEnumExt::get() { 
  return value.load(); 
}

void ValueGuardedEnumExt::set_cb_on_chg(std::function<void(int)> cb) {
  on_chg_cb = cb;
}

ValueGuardedEnumExt::~ValueGuardedEnumExt () {};


}} // namespace SWC::Property