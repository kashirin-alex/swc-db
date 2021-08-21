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
Property::V_BOOL::Ptr boo(const bool& v);
Property::V_UINT8::Ptr i8(const uint8_t& v);
Property::V_UINT16::Ptr i16(const uint16_t& v);
Property::V_INT32::Ptr i32(const int32_t& v);
Property::V_INT64::Ptr i64(const int64_t& v);
Property::V_DOUBLE::Ptr f64(const double& v);
Property::V_STRING::Ptr str(std::string&& v);
Property::V_STRINGS::Ptr strs(Strings&& v);
Property::V_INT64S::Ptr i64s(Int64s&& v);
Property::V_DOUBLES::Ptr f64s(Doubles&& v);

/* cfg methods for guarded types
*  @param v The default Value and a Type
*/
Property::V_GBOOL::Ptr g_boo(const bool& v);
Property::V_GUINT8::Ptr g_i8(const uint8_t& v);
Property::V_GUINT16::Ptr g_i16(const uint16_t& v);
Property::V_GINT32::Ptr g_i32(const int32_t& v);
Property::V_GUINT64::Ptr g_i64(const uint64_t& v);
Property::V_GSTRINGS::Ptr g_strs(Strings&& v);
Property::V_GENUM::Ptr g_enum(const int32_t& v,
                              Property::V_GENUM::OnChg_t&& cb,
                              Property::V_GENUM::FromString_t&& from_string,
                              Property::V_GENUM::Repr_t&& repr);

/* cfg methods for types, a skippable option
*  if no option parsed it is skipped
*/
Property::V_BOOL::Ptr boo();
Property::V_UINT8::Ptr i8();
Property::V_UINT16::Ptr i16();
Property::V_INT32::Ptr i32();
Property::V_INT64::Ptr i64();
Property::V_DOUBLE::Ptr f64();
Property::V_STRING::Ptr str();
Property::V_STRINGS::Ptr strs();
Property::V_INT64S::Ptr i64s();
Property::V_DOUBLES::Ptr f64s();





class ParserConfig final {

  struct ParserOpt final {
    Property::Value::Ptr  value;
    Strings               aliases;
    std::string           desc;
  };

  typedef std::map<std::string, ParserOpt>          Map;
  typedef Core::Vector<std::pair<int, std::string>> Positions;

  public:

  std::string   usage;
  Positions     positions;
  Map           options;
  int           line_length;
  bool          own;


  explicit ParserConfig(int line_len=0, bool own=true) noexcept;

  explicit ParserConfig(const char* usage, int line_len=0, bool own=true);

  explicit ParserConfig(const ParserConfig& other);

  ~ParserConfig();

  void free();

  ParserConfig& definition(const char* u);

  ParserConfig& definition(std::string&& u);

  /* populate from other Parser Config */
  ParserConfig& add(const ParserConfig& other_cfg);

  /* Method to add option */
  ParserConfig& add(const char* names, Property::Value::Ptr vptr,
                    const char* description);

  ParserConfig& operator()(const char* name, Property::Value::Ptr vptr,
                           const char*description);

  ParserConfig& add_options();

  ParserConfig& add_options(const char* name, Property::Value::Ptr vptr,
                            const char* description);

  ParserConfig& add(const char* name, const char* description);

  ParserConfig& operator()(const char* name, const char* description);

  /* Method to add_pos option */
  ParserConfig& add_pos(const char* s, int pos);

  ParserConfig& operator()(const char* s, int pos);

  std::string position_name(int n);

  bool has(const std::string& name) const noexcept;

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

    Options(bool own=true) noexcept;

    ~Options();

    void free();
  };

  explicit Parser(bool unregistered=false) noexcept;

  //~Parser() { }

  void free();

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

  const Options& get_options() const noexcept;

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
