/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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
  typedef std::map<std::string, std::string>  AliasMap;
  typedef std::pair<Map::iterator, bool>      InsRet;
  
  public:

  std::shared_mutex    mutex;

  Properties();

  ~Properties();

  void reset();

  void load_from(const Config::Parser::Options& opts, 
                 bool only_guarded=false);

  void load(const std::string& fname, 
            const Config::ParserConfig& filedesc,
            const Config::ParserConfig& cmddesc,
            bool allow_unregistered=false, bool only_guarded=false);
  
  void reload(const std::string& fname, 
              const Config::ParserConfig& filedesc,
              const Config::ParserConfig& cmddesc);

  void alias(const std::string& primary, const std::string& secondary);

  void set(const std::string& name, Property::Value::Ptr p);

  bool has(const std::string& name) const;

  bool defaulted(const std::string& name);

  std::string to_string(const std::string& name);
  
  void get_names(std::vector<std::string>& names) const;

  void remove(const std::string& name);

  Property::Value::Ptr get_ptr(const std::string& name, bool null_ok=false);

  template <typename T>
  T* get(const std::string& name) {
    return (T*)get_ptr(name);
  }
  

  std::string get_str(const std::string& name) {
    return get<Property::V_STRING>(name)->get();
  }

  std::string get_str(const std::string& name, const std::string& v) {
    return has(name) ? get_str(name) : v;
  }

  Strings get_strs(const std::string& name) {
    return get<Property::V_STRINGS>(name)->get();
  }

  bool get_bool(const std::string& name) {
    return get<Property::V_BOOL>(name)->get();
  }

  bool get_bool(const std::string& name, bool v) {
    return has(name) ? get_bool(name) : v;
  }

  bool get_gbool(const std::string& name) {
    return get<Property::V_GBOOL>(name)->get();
  }

  int32_t get_enum(const std::string& name) {
    return get<Property::V_ENUM>(name)->get();
  }

  int32_t get_genum(const std::string& name) {
    return get<Property::V_GENUM>(name)->get();
  }

  uint8_t get_i8(const std::string& name) {
    return get<Property::V_UINT8>(name)->get();
  }

  uint16_t get_i16(const std::string& name) {
    return get<Property::V_UINT16>(name)->get();
  }

  uint16_t get_i16(const std::string& name, uint16_t v) {
    return has(name) ? get_i16(name) : v;
  }

  int32_t get_i32(const std::string& name) {
    return get<Property::V_INT32>(name)->get();
  }

  int32_t get_i32(const std::string& name, int32_t v) {
    return has(name) ? get_i32(name) : v;
  }

  int64_t get_i64(const std::string& name) {
    return get<Property::V_INT64>(name)->get();
  }


  void print(std::ostream& out, bool include_default = false) const;
  
  std::string to_string_all(bool include_default = false) const;
  
  private:

  Map       m_map;
  AliasMap  m_alias_map;
};



}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Properties.cc"
#endif 

#endif // swc_core_config_Properties_h
