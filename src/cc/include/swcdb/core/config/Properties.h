/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_Properties_h
#define swc_core_config_Properties_h

#include <shared_mutex>
#include <map>

#include "swcdb/core/Error.h"
#include "swcdb/core/config/Property.h"
#include "swcdb/core/config/PropertiesParser.h"


namespace SWC {

class Properties {

  typedef std::map<std::string, Property::Value::Ptr>   Map;
  typedef std::pair<std::string, Property::Value::Ptr>  MapPair;
  typedef std::map<std::string, std::string>  AliasMap;
  typedef std::pair<Map::iterator, bool>      InsRet;
  
  public:

  std::shared_mutex    mutex;

  Properties();

  virtual ~Properties();

  void reset();

  void load_from(const Config::Parser::Options& opts, 
                 bool only_guarded=false);

  void load(const std::string &fname, 
            const Config::ParserConfig &filedesc,
            const Config::ParserConfig &cmddesc,
            bool allow_unregistered=false, bool only_guarded=false);
  
  void reload(const std::string &fname, 
              const Config::ParserConfig &filedesc,
              const Config::ParserConfig &cmddesc);

  void alias(const std::string &primary, const std::string &secondary);

  Property::Value::Ptr get_value_ptr(const std::string &name);

  const bool has(const std::string &name) const;

  const bool defaulted(const std::string &name);

  const std::string str(const std::string &name);
  
  void get_names(std::vector<std::string> &names) const;

  void remove(const std::string &name);

  template <typename T>
  Property::ValueDef<T>* get_type_ptr(const std::string &name) {
    return (Property::ValueDef<T>*)get_value_ptr(name)->get_type_ptr();
  }
  
  template <typename T>
  T* get_ptr(const std::string &name) {
    return get_value_ptr(name)->get_ptr<T>();
  }
  
  template <typename T>
  T get(const std::string &name) {
    try {
      return get_value_ptr(name)->get<T>();
    } catch (std::exception &e) {
      SWC_THROWF(Error::CONFIG_GET_ERROR, "getting value of '%s': %s",
                name.c_str(), e.what());
    }
  }
  
  template <typename T>
  T get_pref(const Strings& names){
    for(const std::string& name : names)
      if (has(name))
        return get<T>(name);
    SWC_THROWF(Error::CONFIG_GET_ERROR, "getting pref value by '%s'",
                                        format_list(names).c_str());
  }
  
  template <typename T>
  T get(const std::string &name, T default_value) {
    auto it = m_map.find(name);
    if (it != m_map.end())
      return it->second->get<T>();
    set(name, default_value);
    return get<T>(name);
  }
  
  template<typename T>
  InsRet add(const std::string &name, T v) {
    return m_map.emplace(name, new Property::Value(v));
  }
  
  InsRet add(const std::string &name, Property::Value::Ptr v) {
    return m_map.emplace(name, v);
  }
  
  template<typename T>
  void set(const std::string &name, T v, bool defaulted=false)  {
    InsRet r = add(name, v);
    if (!r.second)
      (*r.first).second = new Property::Value(v);
    else 
      (*r.first).second->set_value(v);
    if(defaulted)
      (*r.first).second->default_value();
  }
  
  template<typename T>
  void set(const std::string &name, Property::ValueDef<T> v)  {
    InsRet r = add(name, v.get_value());
    if (!r.second)
      (*r.first).second = new Property::Value(v.get_value());
    else 
      (*r.first).second->set_value(v.get_value());
  }
  
  void set(const std::string &name, Property::Value::Ptr p);


  bool get_bool(const std::string &name);
  
  std::string get_str(const std::string &name);
  
  Strings get_strs(const std::string &name);
  
  uint8_t get_i8(const std::string &name);

  uint16_t get_i16(const std::string &name);

  int32_t get_i32(const std::string &name);
  
  int64_t get_i64(const std::string &name);
  
  Int64s get_i64s(const std::string &name);
  
  double get_f64(const std::string &name);
  
  Doubles get_f64s(const std::string &name);
  
  bool get_bool(const std::string &name, bool default_value);
  
  std::string get_str(const std::string &name, std::string &default_value);
  
  std::string get_str(const std::string &name, std::string default_value);
  
  Strings get_strs(const std::string &name, Strings default_value);
  
  uint8_t get_i8(const std::string &name, uint8_t default_value);

  uint16_t get_i16(const std::string &name, uint16_t default_value);
  
  int32_t get_i32(const std::string &name, int32_t default_value);
  
  int64_t get_i64(const std::string &name, int64_t default_value);
  
  Int64s get_i64s(const std::string &name, Int64s &default_value);
  
  double get_f64(const std::string &name, double default_value); 
  
  Doubles get_f64s(const std::string &name, Doubles default_value);


  void print(std::ostream &out, bool include_default = false) const;
  
  const std::string to_string(bool include_default = false) const;
  
  private:

  Map       m_map;
  AliasMap  m_alias_map;
};



}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Properties.cc"
#endif 

#endif // swc_core_config_Properties_h
