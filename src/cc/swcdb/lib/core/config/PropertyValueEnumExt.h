/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_PropertyEnumExt_h
#define swc_core_config_PropertyEnumExt_h



#include <functional>



namespace SWC { namespace Property {


class ValueEnumExtBase {
  public:

  virtual void set_value(int nv){}

  virtual int get() { return -1; }

  ValueEnumExtBase* operator =(int nv){
    set_value(nv);
    return *this;
  }

  bool operator==(ValueEnumExtBase a) { return get() == a.get(); }
  bool operator!=(ValueEnumExtBase a) { return get() != a.get(); }

  operator ValueEnumExtBase*()  { return this;  }
  operator int()                { return get(); }
  operator std::string()        { return str(); }
    

  ValueEnumExtBase* operator =(ValueEnumExtBase other){
    set_from(other);
    return *this;
  }

  void set_from(ValueEnumExtBase &other){
    if((int)other != -1)
      set_value((int)other);
    if(!other.cb_set) 
      return;
      
    set_repr(other.get_call_repr());
    set_from_string(other.get_call_from_string());
  }

  ValueEnumExtBase& set_from_string(std::function<int(std::string)> cb) {
    call_from_string = cb;
    cb_set = true;
    return *this;
  }
    
  ValueEnumExtBase& set_repr(std::function<std::string(int)> cb) {
    call_repr = cb;
    cb_set = true;
    return *this;
  }

  int from_string(const std::string& opt) {
    int nv = get_call_from_string()(opt);
    if(nv > -1)
      set_value(nv);
    else {
      if(get() == -1)
        HT_THROWF(Error::CONFIG_GET_ERROR, 
                  "Bad Value %s, no corresponding enum", opt.c_str());
      else
        HT_WARNF("Bad cfg Value %s, no corresponding enum", opt.c_str());
    }
    return get();
  }
    
  void set_default_calls() {
    call_from_string = [](const std::string& opt) {
      HT_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s, no from_string cb set", opt.c_str());
      return -1;
    };
    call_repr = [](int v){ return "No repr cb defined!"; };
  }

  std::function<int(std::string)> get_call_from_string(){
    if(!cb_set) set_default_calls();
    return call_from_string;
  }

  std::function<std::string(int)> get_call_repr(){
    if(!cb_set) set_default_calls();
    return call_repr;
  }

  const std::string str() {
    return get_call_repr()(get());
  }

  const std::string to_str() {
    return format("%s  # (%d)", get_call_repr()(get()).c_str(), get());
  }

  virtual ~ValueEnumExtBase() {};

  private:
  bool cb_set = false; 
  std::function<int(std::string)> call_from_string;
  std::function<std::string(int)> call_repr;
};


class ValueEnumExt : public ValueEnumExtBase {
  public:

  ValueEnumExt(int nv = -1) { 
    set_value(nv);
  }

  void set_value(int nv) override {
    value = nv;
  }

  int get() override {
    return value; 
  }

  ~ValueEnumExt () {};

  private:
  int value;
};


class ValueGuardedEnumExt : public ValueEnumExtBase {
  public:

  ValueGuardedEnumExt(int nv = -1) {
    set_value(nv);
  }

  ValueGuardedEnumExt(ValueGuardedEnumExt& other) {
    set_from(other);
  }
    
  ValueGuardedEnumExt& operator=(ValueGuardedEnumExt& other) {
    set_from(other); 
    return *this; 
  }

  void set_value(int nv) override {
    if(nv == get()) 
      return;
        
    value.store(nv);
    if(on_chg_cb)
      on_chg_cb(get());
  }

  int get() override { 
    return value.load(); 
  }

  void set_cb_on_chg(std::function<void(int)> cb) {
    on_chg_cb = cb;
  }

  ~ValueGuardedEnumExt () {};

  private:
  std::atomic<int> value;
  std::function<void(int)> on_chg_cb;
  
};

} // namespace Property


typedef Property::ValueEnumExt          EnumExt;
typedef Property::ValueGuardedEnumExt   gEnumExt;
typedef Property::ValueGuardedEnumExt*  gEnumExtPtr;

} // namespace SWC

#endif // swc_core_config_PropertyEnumExt_h
