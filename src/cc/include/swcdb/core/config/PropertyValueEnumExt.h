/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_PropertyValueEnumExt_h
#define swc_core_config_PropertyValueEnumExt_h



namespace SWC { namespace Property {


class ValueEnumExtBase {
  public:

  virtual void set_value(int nv);

  virtual int get();

  ValueEnumExtBase* operator =(int nv);

  bool operator==(ValueEnumExtBase a);
  bool operator!=(ValueEnumExtBase a);

  operator ValueEnumExtBase*();
  operator int(); 
  operator std::string();
    

  ValueEnumExtBase* operator =(ValueEnumExtBase other);

  void set_from(ValueEnumExtBase &other);

  ValueEnumExtBase& set_from_string(std::function<int(std::string)> cb);
    
  ValueEnumExtBase& set_repr(std::function<std::string(int)> cb);

  int from_string(const std::string& opt);
    
  void set_default_calls();

  std::function<int(std::string)> get_call_from_string();

  std::function<std::string(int)> get_call_repr();

  const std::string str();

  const std::string to_str();

  virtual ~ValueEnumExtBase();

  private:
  bool cb_set = false; 
  std::function<int(std::string)> call_from_string;
  std::function<std::string(int)> call_repr;
};


class ValueEnumExt : public ValueEnumExtBase {
  public:

  ValueEnumExt(int nv = -1);

  void set_value(int nv) ;

  int get() override;

  ~ValueEnumExt ();

  private:
  int value;
};


class ValueGuardedEnumExt : public ValueEnumExtBase {
  public:

  ValueGuardedEnumExt(int nv = -1);

  ValueGuardedEnumExt(ValueGuardedEnumExt& other);
    
  ValueGuardedEnumExt& operator=(ValueGuardedEnumExt& other);

  void set_value(int nv) override;

  int get() override;

  void set_cb_on_chg(std::function<void(int)> cb);

  ~ValueGuardedEnumExt ();

  private:
  std::atomic<int> value;
  std::function<void(int)> on_chg_cb;
  
};

} // namespace Property


typedef Property::ValueEnumExt          EnumExt;
typedef Property::ValueGuardedEnumExt   gEnumExt;
typedef Property::ValueGuardedEnumExt*  gEnumExtPtr;

} // namespace SWC


#ifdef SWC_IMPL_SOURCE
#include "../../../../lib/swcdb/core/config/PropertyValueEnumExt.cc"
#endif 

#endif // swc_core_config_PropertyValueEnumExt_h
