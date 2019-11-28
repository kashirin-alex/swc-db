/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#ifndef swc_core_config_Properties_h
#define swc_core_config_Properties_h

#include <map>

#include "Property.h"
#include "PropertiesParser.h"


// convenience/abbreviated accessors
#define HT_PROPERTIES_ABBR_ACCESSORS() \
  inline bool get_bool(const std::string &name)  { \
    return get<bool>(name); } \
  inline std::string get_str(const std::string &name)  { \
    return get<std::string>(name); } \
  inline Strings get_strs(const std::string &name)  { \
    return get<Strings>(name); } \
  inline uint16_t get_i16(const std::string &name)  { \
    return get<uint16_t>(name); } \
  inline int32_t get_i32(const std::string &name)  { \
    return get<int32_t>(name); } \
  inline int64_t get_i64(const std::string &name)  { \
    return get<int64_t>(name); } \
  inline Int64s get_i64s(const std::string &name)  { \
    return get<Int64s>(name); } \
  inline double get_f64(const std::string &name)  { \
    return get<double>(name); } \
  inline Doubles get_f64s(const std::string &name)  { \
    return get<Doubles>(name); } \
  inline bool get_bool(const std::string &name, bool default_value) \
     { return get<bool>(name, default_value); } \
  inline std::string get_str(const std::string &name, std::string &default_value) \
     { return get<std::string>(name, static_cast<std::string>(default_value)); } \
  inline std::string get_str(const std::string &name, std::string default_value) \
     { return get<std::string>(name, default_value); } \
  inline Strings get_strs(const std::string &name, Strings default_value) \
     { return get<Strings>(name, default_value); } \
  inline uint16_t get_i16(const std::string &name, uint16_t default_value) \
     { return get<uint16_t>(name, default_value); } \
  inline int32_t get_i32(const std::string &name, int32_t default_value) \
     { return get<int32_t>(name, default_value); } \
  inline int64_t get_i64(const std::string &name, int64_t default_value) \
     { return get<int64_t>(name, default_value); } \
  inline Int64s get_i64s(const std::string &name, Int64s &default_value) \
     { return get<Int64s>(name, default_value); } \
  inline double get_f64(const std::string &name, double default_value) \
     { return get<double>(name, default_value); } \
  inline Doubles get_f64s(const std::string &name, Doubles default_value) \
     { return get<Doubles>(name, default_value); }
  /*     \
  inline bool get_bool(const std::string &name, bool default_value) \
    const { return get<bool>(name, default_value); } \
  inline std::string get_str(const std::string &name, std::string default_value) \
    const { return get<std::string>(name, default_value); } \
  inline Strings get_strs(const std::string &name, Strings &default_value) \
    const { return get<Strings>(name, default_value); } \
  inline uint16_t get_i16(const std::string &name, uint16_t default_value) \
    const { return get<uint16_t>(name, default_value); } \
  inline int32_t get_i32(const std::string &name, int32_t default_value) \
    const { return get<int32_t>(name, default_value); } \
  inline int64_t get_i64(const std::string &name, int64_t default_value)  \
    const { return get<int64_t>(name, default_value); } \
  inline Int64s get_i64s(const std::string &name, Int64s default_value) \
    const { return get<Int64s>(name, default_value); } \
  inline double get_f64(const std::string &name, double default_value) \
    const { return get<double>(name, default_value); } \
  inline Doubles get_f64s(const std::string &name, Doubles default_value) \
    const { return get<Doubles>(name, default_value); }
*/


namespace SWC {

class Properties {

  typedef std::map<std::string, Property::Value::Ptr>   Map;
  typedef std::pair<std::string, Property::Value::Ptr>  MapPair;
  typedef std::pair<Map::iterator, bool>      InsRet;
  typedef std::map<std::string, std::string>  AliasMap;
  
  public:

  Properties() { }
  
  virtual ~Properties() { 
    reset();
  }

  void reset() {
    for (const auto &kv : m_map)
      delete kv.second;
    m_map.clear();
  }

  void load(const std::string &fname, 
            const Config::ParserConfig &filedesc,
            const Config::ParserConfig &cmddesc,
            bool allow_unregistered=false) {
    std::ifstream in(fname.c_str());
    Config::Parser prs(in, filedesc, cmddesc, allow_unregistered);
    load_from(prs.get_options());
  } 

  void load_from(const Config::Parser::Options& opts, 
                 bool only_guarded=false) {
    for(const auto &kv : opts.map) {
      if(has(kv.first) && (kv.second->is_default() || 
                           (only_guarded && !kv.second->is_guarded())))
      continue;
      set(kv.first, kv.second);
    }
  }
  
  void load_files_by(const std::string &fileprop, 
                     const Config::ParserConfig &filedesc,
                     const Config::ParserConfig &cmddesc,
	                   bool allow_unregistered = false) {
    if(fileprop.empty() || !has(fileprop)) 
      return;

    Strings files = get_strs(fileprop);
    for (auto it=files.begin(); it<files.end(); it++){
	    try {
        load(*it, filedesc, cmddesc, allow_unregistered);
      } catch (std::exception &e) {
		    HT_WARNF("%s has bad cfg file %s: %s", 
                  fileprop.c_str(), it->c_str(), e.what());
      }
    }
  }
    
  std::string reload(const std::string &fname, 
                     const Config::ParserConfig &filedesc,
                     const Config::ParserConfig &cmddesc,
	                   bool allow_unregistered = false) {
    std::string out;
	  try {
		  std::ifstream in(fname.c_str());

      HT_INFOF("Reloading Configuration File %s", fname.c_str());
		  if (!in) throw;
    
      out.append("\n\nCurrent Configurations:\n");
      append_to_string(out);

      Config::Parser prs(in, filedesc, cmddesc, allow_unregistered);
      load_from(prs.get_options(), true);

      out.append("\n\nNew Configurations:\n");
      append_to_string(out);
      return out;
	  }
	  catch (std::exception &e) {
		  HT_WARNF("Error::CONFIG_BAD_CFG_FILE %s: %s", fname.c_str(), e.what());
      return format("Error::CONFIG_BAD_CFG_FILE %s: %s \noutput:\n%s", 
                      fname.c_str(), e.what(), out.c_str());
	  }
  }

  void alias(const std::string &primary, const std::string &secondary) {
    m_alias_map[primary] = secondary;
    m_alias_map[secondary] = primary;
  }

  Property::Value::Ptr get_value_ptr(const std::string &name) {
    auto it = m_map.find(name);
    if (it != m_map.end())
      return it->second;

    auto alias = m_alias_map.find(name);
    if(alias != m_alias_map.end()) {
      it = m_map.find(alias->second);
      if (it != m_map.end())
        return it->second;
    }
    
    HT_THROWF(Error::CONFIG_GET_ERROR, 
              "getting value of '%s' - missing",
              name.c_str());
  }

  const bool has(const std::string &name) const {
    if(m_map.count(name))
      return true;
      
    auto alias = m_alias_map.find(name);
    if(alias == m_alias_map.end()) 
      return false;
    return m_map.count(alias->second);
  }

  const bool defaulted(const std::string &name) {
    return get_value_ptr(name)->is_default();
  }

  const std::string str(const std::string &name) {
    return get_value_ptr(name)->str();
  }
  
  void get_names(std::vector<std::string> &names) const {
    for(auto it = m_map.begin(); it != m_map.end(); it++)
      names.push_back(it->first);
  }

  void remove(const std::string &name) {
    auto it = m_map.find(name);
    if(it != m_map.end()) {
      delete it->second;
      m_map.erase(it);
    }
  }

  /**
   * Get the ptr of property value type. Throws if name is not defined.
   * Property::ValueDef<int32_t>* threads = props->get_type_ptr<int32_t>("threads");
   * 
   * @param name The name of the property
   * @throw Error::CONFIG_GET_ERROR if the requested property is not defined
   * @return Property::ValueDef<T>*
   */
  template <typename T>
  Property::ValueDef<T>* get_type_ptr(const std::string &name) {
    return (Property::ValueDef<T>*)get_value_ptr(name)->get_type_ptr();
  }

  /**
   * Get the property value ptr
   *
   * @param name The name of the property
   * @throw Error::CONFIG_GET_ERROR if the requested property is not defined
   * @return T*
   */
  template <typename T>
  T* get_ptr(const std::string &name) {
    return get_value_ptr(name)->get_ptr<T>();
  }
  
  /**
   * Get the value of option of type T. Throws if option is not defined.
   *
   * @param name The name of the property
   * @throw Error::CONFIG_GET_ERROR if the requested property is not defined
   */
  template <typename T>
  T get(const std::string &name) {
    try {
      return get_value_ptr(name)->get<T>();
    } catch (std::exception &e) {
      HT_THROWF(Error::CONFIG_GET_ERROR, "getting value of '%s': %s",
                name.c_str(), e.what());
    }
  }
  
  /**
   * Get the value of option of type T by preference. Throws if option is not defined.
   *
   * @param namee The names of preference for the property
   * @throw Error::CONFIG_GET_ERROR if the requested property is not defined
   */
  template <typename T>
  T get_pref(const Strings& names){
    for(const std::string& name : names)
      if (has(name))
        return get<T>(name);
    HT_THROWF(Error::CONFIG_GET_ERROR, "getting pref value by '%s'",
                                        format_list(names).c_str());
  }

  /**
   * Get the value of option of type T. Returns supplied default value if
   * not found. Try use the first form in usual cases and supply default
   * values in the config descriptions, as it validates the name and is less
   * error prone.
   * If no value, sets the default value to properties 
   *
   * @param name The name of the property
   * @param default_value The default value to return if not found
   */
  template <typename T>
  T get(const std::string &name, T default_value) {
    auto it = m_map.find(name);
    if (it != m_map.end())
      return it->second->get<T>();
    set(name, default_value);
    return get<T>(name);
  }

  HT_PROPERTIES_ABBR_ACCESSORS()

  /**
   * Add property to the map of type T
   *
   * @param name The name of the property
   * @param v The Property::ValueDef<T> the property
   * @return An iterator to the new item
   */
  template<typename T>
  InsRet add(const std::string &name, T v) {
    return m_map.insert(MapPair(name, new Property::Value(v)));
  }
  /**
   * Add property to the map
   *
   * @param name The name of the property
   * @param v The Property::Value::Ptr of the property
   * @return An iterator to the new item
   */
  InsRet add(const std::string &name, Property::Value::Ptr v) {
    return m_map.insert(MapPair(name, v));
  }

  /**
   * Set a property in the map, create or update, of T type
   *
   * @param name The name of the property
   * @param v The value of the property, name need to correspond to Value Type
   */
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

  /**
   * Set a property in the map, create or update, by Property::ValueDef<T> type
   *
   * @param name The name of the property
   * @param v The Property::ValueDef<T>
   */
  template<typename T>
  void set(const std::string &name, Property::ValueDef<T> v)  {
    InsRet r = add(name, v.get_value());
    if (!r.second)
      (*r.first).second = new Property::Value(v.get_value());
    else 
      (*r.first).second->set_value(v.get_value());
  }

  /**
   * Set a property in the map, create or update, from Property::Value::Ptr
   *
   * @param name The name of the property
   * @param v The Property::Value::Ptr
   */
  void set(const std::string &name, Property::Value::Ptr p) {
    Property::Value::Ptr p_set;
    if(m_map.count(name) == 0) {
      p_set = Property::Value::make_new(p);
      
      if(p->is_default())
        p_set->default_value();
      
      if(p->is_guarded())
        p_set->guarded(true);
        
      add(name, p_set);

    } else if(!p->is_default()) {
      p_set = get_value_ptr(name);
      p_set->set_value_from(p);
      p_set->default_value(false);
    }
  }


  /**
   * Prints keys and values of the configuration map
   *
   * @param out The output stream
   * @param include_default If true then default values are included
   */
  void print(std::ostream &out, bool include_default = false) const {
    for (const auto &kv : m_map) {
      bool isdefault = kv.second->is_default();

      if (include_default || !isdefault) {
        out << kv.first << '=' << kv.second->str();

        if (isdefault)
          out << " (default)";
        out << std::endl;
      }
    }
  }

  /**
   * Append to std::string keys and values of the configuration map for print
   *   
   * @param out The std::string to append 
   * @param include_default If true then default values are included
   */
  void append_to_string(std::string &out, bool include_default = false) const {
    bool isdefault;
    for (const auto &kv : m_map) {
      isdefault = kv.second->is_default();
      if (include_default || !isdefault) {
        out.append(format("%s=%s", kv.first.c_str(), kv.second->str().c_str()));
        if (isdefault)
          out.append(" (default)");
        out.append("\n");
      }
    }
  }

  private:

  Map       m_map;
  AliasMap  m_alias_map;
};



}
#endif // swc_core_config_Properties_h
