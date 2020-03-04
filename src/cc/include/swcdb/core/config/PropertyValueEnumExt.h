/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_PropertyValueEnumExt_h
#define swc_core_config_PropertyValueEnumExt_h



namespace SWC { namespace Property {


class ValueEnumExtBase {
  public:

  typedef std::function<int(const std::string&)> FromString_t; 
  typedef std::function<std::string(int)>        Repr_t; 

  virtual void set_value(int nv) = 0;

  virtual int get() = 0;

  operator int(); 
  
  operator std::string();

  ValueEnumExtBase& set_from_string(const FromString_t& cb);
    
  ValueEnumExtBase& set_repr(const Repr_t& cb);

  int from_string(const std::string& opt);
    
  void set_default_calls();

  const FromString_t& get_call_from_string();

  const Repr_t& get_call_repr();

  const std::string str();

  const std::string to_str();

  protected:
  bool          cb_set; 
  FromString_t  call_from_string = 0;
  Repr_t        call_repr = 0;
};


class ValueEnumExt : public ValueEnumExtBase {
  public:

  ValueEnumExt(int nv = -1);
  
  ValueEnumExt(int nv, 
               const FromString_t& from_string, 
               const Repr_t& repr);

  ValueEnumExt(ValueEnumExt& other);
    
  ValueEnumExt& operator=(ValueEnumExt& other);
  
  void set_from(ValueEnumExt &other);

  void set_value(int nv) override;

  int get() override;

  virtual ~ValueEnumExt();

  private:
  int value;
};


class ValueGuardedEnumExt : public ValueEnumExtBase {
  public:

  typedef std::function<void(int)> OnChg_t; 

  ValueGuardedEnumExt(int nv = -1);

  ValueGuardedEnumExt(int nv, 
                      const OnChg_t& cb,
                      const FromString_t& from_string, 
                      const Repr_t& repr);

  ValueGuardedEnumExt(ValueGuardedEnumExt& other);
    
  ValueGuardedEnumExt& operator=(ValueGuardedEnumExt& other);

  void set_from(ValueGuardedEnumExt &other);

  void set_value(int nv) override;

  int get() override;

  void set_cb_on_chg(const OnChg_t& cb);

  virtual ~ValueGuardedEnumExt();

  private:
  std::atomic<int> value;
  OnChg_t          on_chg_cb = 0;
  
};

} // namespace Property


typedef Property::ValueEnumExt          EnumExt;
typedef Property::ValueGuardedEnumExt   gEnumExt;
typedef Property::ValueGuardedEnumExt*  gEnumExtPtr;

} // namespace SWC


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/PropertyValueEnumExt.cc"
#endif 

#endif // swc_core_config_PropertyValueEnumExt_h
