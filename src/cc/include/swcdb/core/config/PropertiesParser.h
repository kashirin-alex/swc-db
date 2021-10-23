/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_config_PropertiesParser_h
#define swcdb_core_config_PropertiesParser_h


#include "swcdb/core/config/Property.h"


namespace SWC { namespace Config {


/* cfg methods for types
*  @param v The default Value and a Type
*/
Property::Value_bool::Ptr     boo(const bool& v);
Property::Value_uint8::Ptr    i8(const uint8_t& v);
Property::Value_uint16::Ptr   i16(const uint16_t& v);
Property::Value_int32::Ptr    i32(const int32_t& v);
Property::Value_int64::Ptr    i64(const int64_t& v);
Property::Value_double::Ptr   f64(const double& v);
Property::Value_string::Ptr   str(std::string&& v);
Property::Value_strings::Ptr  strs(Strings&& v);
Property::Value_int64s::Ptr   i64s(Int64s&& v);
Property::Value_doubles::Ptr  f64s(Doubles&& v);

/* cfg methods for guarded types
*  @param v The default Value and a Type
*/
Property::Value_bool_g::Ptr     g_boo(const bool& v);
Property::Value_uint8_g::Ptr    g_i8(const uint8_t& v);
Property::Value_uint16_g::Ptr   g_i16(const uint16_t& v);
Property::Value_int32_g::Ptr    g_i32(const int32_t& v);
Property::Value_uint64_g::Ptr   g_i64(const uint64_t& v);
Property::Value_strings_g::Ptr  g_strs(Strings&& v);
Property::Value_enum_g::Ptr     g_enum(
                          const int32_t& v,
                          Property::Value_enum_g::OnChg_t&& cb,
                          Property::Value_enum_g::FromString_t&& from_string,
                          Property::Value_enum_g::Repr_t&& repr);

/* cfg methods for types, a skippable option
*  if no option parsed it is skipped
*/
Property::Value_bool::Ptr     boo();
Property::Value_uint8::Ptr    i8();
Property::Value_uint16::Ptr   i16();
Property::Value_int32::Ptr    i32();
Property::Value_int64::Ptr    i64();
Property::Value_double::Ptr   f64();
Property::Value_string::Ptr   str();
Property::Value_strings::Ptr  strs();
Property::Value_int64s::Ptr   i64s();
Property::Value_doubles::Ptr  f64s();





class ParserConfig final {

  struct ParserOpt final {
    Property::Value::Ptr  value;
    Strings               aliases;
    std::string           desc;
    ~ParserOpt() noexcept { }
  };

  typedef std::map<std::string, ParserOpt>          Map;
  typedef Core::Vector<std::pair<int, std::string>> Positions;

  public:

  std::string   usage;
  Positions     positions;
  Map           options;
  int           line_length;
  bool          own;

  SWC_SHOULD_NOT_INLINE
  explicit ParserConfig(int line_len=0, bool own=true) noexcept;

  SWC_SHOULD_NOT_INLINE
  explicit ParserConfig(const char* usage, int line_len=0, bool own=true);

  SWC_SHOULD_NOT_INLINE
  explicit ParserConfig(const ParserConfig& other);

  ~ParserConfig() noexcept;

  void free() noexcept;

  ParserConfig& definition(const char* u);

  ParserConfig& definition(std::string&& u);

  /* populate from other Parser Config */
  ParserConfig& add(const ParserConfig& other_cfg);

  /* Method to add option */
  ParserConfig& add(const char* names, Property::Value::Ptr vptr,
                    const char* description);

  ParserConfig& operator()(const char* name, Property::Value::Ptr vptr,
                           const char*description);

  ParserConfig& SWC_CONST_FUNC add_options();

  ParserConfig& add_options(const char* name, Property::Value::Ptr vptr,
                            const char* description);

  ParserConfig& add(const char* name, const char* description);

  ParserConfig& operator()(const char* name, const char* description);

  /* Method to add_pos option */
  ParserConfig& add_pos(const char* s, int pos);

  ParserConfig& operator()(const char* s, int pos);

  std::string position_name(int n);

  bool SWC_PURE_FUNC has(const std::string& name) const noexcept;

  bool has(const std::string& name, std::string& alias_to) const noexcept;

  Property::Value::Ptr get_default(const std::string& name);

  void remove(const std::string& name);

  void print(std::ostream& os) const;

};

std::ostream& operator<<(std::ostream& os, const ParserConfig& cfg);




class Parser final {
  public:

  ParserConfig config;

  static Strings args_to_strings(int argc, char *argv[]);

  class Options final {
    public:
    bool  own;
    std::map<std::string, Property::Value::Ptr> map;

    SWC_SHOULD_NOT_INLINE
    Options(bool own=true) noexcept;

    ~Options() noexcept;

    void free() noexcept;
  };

  SWC_SHOULD_NOT_INLINE
  explicit Parser(bool unregistered=false) noexcept;

  ~Parser() noexcept;

  void free() noexcept;

  void parse_filedata(std::ifstream& in);

  void parse_cmdline(int argc, char *argv[]);

  void parse_cmdline(const Strings& raw_strings);

  void parse_line(const std::string& line);

  void set_pos_parse(const std::string& name, const std::string& value);

  bool parse_opt(const std::string& s);

  void make_options();

  // convert, validate and add property to options
  void add_opt(const std::string& name, Property::Value::Ptr p,
               const Strings& raw_opt);

  void own_options(Options& opts);

  const Options& SWC_CONST_FUNC get_options() const noexcept;

  void print(std::ostream& os) const;

  void print_options(std::ostream& os) const;

  private:
  typedef std::pair<std::string, Strings> Pair;
  typedef std::map<std::string, Strings> Map;

  Map           raw_opts;
  bool          m_unregistered;
  Options       m_opts;

};

std::ostream& operator<<(std::ostream& os, const Parser& prs);


}} // namespace SWC::Config


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/PropertiesParser.cc"
#endif

#endif // swcdb_core_config_PropertiesParser_h
