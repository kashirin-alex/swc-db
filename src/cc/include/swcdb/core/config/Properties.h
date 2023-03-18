/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_config_Properties_h
#define swcdb_core_config_Properties_h


#include "swcdb/core/Exception.h"
#include "swcdb/core/MutexSptd.h"
#include "swcdb/core/config/Property.h"
#include "swcdb/core/config/PropertiesParser.h"


namespace SWC { namespace Config {

class Properties {

  typedef std::map<std::string, Property::Value::Ptr> Map;
  typedef std::map<std::string, std::string>          AliasMap;

  public:

  Core::MutexSptd    mutex;

  SWC_CAN_INLINE
  Properties() noexcept : mutex(), m_map(), m_alias_map()  { }

  SWC_CAN_INLINE
  ~Properties() noexcept {
    reset();
  }

  void reset() noexcept;

  void load_from(const Config::Parser::Options& opts,
                 bool only_guarded=false);

  void load(const std::string& fname,
            const Config::ParserConfig& filedesc,
            const Config::ParserConfig& cmddesc,
            bool allow_unregistered=false, bool only_guarded=false);

  void reload(const std::string& fname,
              const Config::ParserConfig& filedesc,
              const Config::ParserConfig& cmddesc);

  void alias(const char* primary, const char* secondary);

  void set(const char* name, Property::Value::Ptr p);

  bool SWC_PURE_FUNC has(const char* name) const noexcept;

  bool defaulted(const char* name) const;

  std::string to_string(const char* name) const;

  void get_names(Strings& names) const;

  void remove(const char* name);

  Property::Value::Ptr get_ptr(const char* name, bool null_ok=false) const;

  template <typename T>
  SWC_SHOULD_NOT_INLINE
  T* get(const char* name) const {
    return Property::Value::get_pointer<T>(get_ptr(name));
  }

  std::string get_str(const char* name) const {
    return get<Property::Value_string>(name)->get();
  }

  std::string get_str(const char* name, const std::string& v) const {
    return has(name) ? get_str(name) : v;
  }

  Strings get_strs(const char* name) const {
    return get<Property::Value_strings>(name)->get();
  }

  bool get_bool(const char* name) const {
    return get<Property::Value_bool>(name)->get();
  }

  bool get_bool(const char* name, bool v) const {
    return has(name) ? get_bool(name) : v;
  }

  bool get_gbool(const char* name) const {
    return get<Property::Value_bool_g>(name)->get();
  }

  int32_t get_enum(const char* name) const {
    return get<Property::Value_enum>(name)->get();
  }

  int32_t get_genum(const char* name) const {
    return get<Property::Value_enum_g>(name)->get();
  }

  uint8_t get_i8(const char* name) const {
    return get<Property::Value_uint8>(name)->get();
  }

  uint16_t get_i16(const char* name) const {
    return get<Property::Value_uint16>(name)->get();
  }

  uint16_t get_i16(const char* name, uint16_t v) const {
    return has(name) ? get_i16(name) : v;
  }

  int32_t get_i32(const char* name) const {
    return get<Property::Value_int32>(name)->get();
  }

  int32_t get_i32(const char* name, int32_t v) const {
    return has(name) ? get_i32(name) : v;
  }

  int64_t get_i64(const char* name) const {
    return get<Property::Value_int64>(name)->get();
  }


  void print(std::ostream& out, bool include_default = false) const;

  private:

  Map       m_map;
  AliasMap  m_alias_map;
};



}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Properties.cc"
#endif

#endif // swcdb_core_config_Properties_h
