/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_PropertiesParser_h
#define swc_core_config_PropertiesParser_h

#include "swcdb/core/config/Property.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>

namespace SWC {

// Config::cfg(Int32tSafe default_value, true)
template<typename T>
Property::Value::Ptr cfg(T v = 0, bool skippable=false, bool guarded=false);

/* cfg methods for types
*  @param v The default Value and a Type
*/
Property::Value::Ptr boo(bool v);
Property::Value::Ptr i16(uint16_t v);
Property::Value::Ptr i32(int32_t v);
Property::Value::Ptr i64(int64_t v);
Property::Value::Ptr f64(double v);
Property::Value::Ptr str(std::string v);
Property::Value::Ptr strs(Strings v);
Property::Value::Ptr i64s(Int64s v);
Property::Value::Ptr f64s(Doubles v);
Property::Value::Ptr enum_ext(EnumExt v);

/* cfg methods for guarded types
*  @param v The default Value and a Type
*/
Property::Value::Ptr g_boo(bool v);
Property::Value::Ptr g_i32(int32_t v);
Property::Value::Ptr g_strs(Strings v);
Property::Value::Ptr g_enum_ext(gEnumExt v);

/* cfg methods for types, a skippable option
*  if no option parsed it is skipped
*/
Property::Value::Ptr boo();
Property::Value::Ptr i16();
Property::Value::Ptr i32();
Property::Value::Ptr i64();
Property::Value::Ptr f64();
Property::Value::Ptr str();
Property::Value::Ptr strs();
Property::Value::Ptr i64s();
Property::Value::Ptr f64s();



namespace Config {

  
class ParserConfig {
  
  struct ParserOpt {
    Property::Value::Ptr  value;
    Strings               aliases;
    std::string           desc;
  };

  typedef std::map<std::string, ParserOpt>          Map;
  typedef std::pair<std::string, ParserOpt>         MapPair;
  typedef std::vector<std::pair<int, std::string>>  Positions;

  public:

  std::string   usage;
  Positions     positions;
  Map           options;
  int           line_length;
  bool          own;


  explicit ParserConfig(const std::string& usage = "", 
                        int line_len=0, bool own=true);

  explicit ParserConfig(const ParserConfig& other);

  virtual ~ParserConfig();

  void free();
  
  ParserConfig& definition(const std::string& u);

  /* populate from other Parser Config */
  ParserConfig& add(const ParserConfig& other_cfg);

  /* Method to add option */
  ParserConfig& add(const std::string& names, Property::Value::Ptr vptr, 
                    const std::string& description);

  ParserConfig& operator()(const std::string& name, Property::Value::Ptr vptr,
                           const std::string& description);

  ParserConfig& add_options();

  ParserConfig& add_options(const std::string &name, Property::Value::Ptr vptr,
                            const std::string& description);

  ParserConfig& add(const std::string &name, const std::string& description);

  ParserConfig& operator()(const std::string &name, 
                           const std::string& description);
    
  /* Method to add_pos option */
  ParserConfig& add_pos(const std::string s, int pos);

  ParserConfig& operator()(const std::string s, int pos);

  const std::string position_name(int n);

  const bool has(const std::string& name) const;

  const bool has(const std::string& name, std::string& alias_to) const;

  Property::Value::Ptr get_default(const std::string& name);

  void print(std::ostream& os) const;

};

std::ostream& operator<<(std::ostream& os, const ParserConfig& cfg);




class Parser {
  public:

  ParserConfig config;

  static Strings args_to_strings(int argc, char *argv[]);

  class Options {
    public:
    bool  own;
    std::map<std::string, Property::Value::Ptr> map;

    Options(bool own=true);

    virtual ~Options();

    void free();
  };

  explicit Parser(bool unregistered=false);
  
  virtual ~Parser();

  void free();
  
  void parse_filedata(std::ifstream &in);

  void parse_cmdline(int argc, char *argv[]);

  void parse_cmdline(const Strings& raw_strings);

  void parse_line(const std::string& line);

  void set_pos_parse(const std::string& name, const std::string& value);

  const bool parse_opt(const std::string& s);

  void make_options();

  // convert, validate and add property to options
  void add_opt(const std::string& name, Property::Value::Ptr p, 
               const Strings& raw_opt);
      
  void own_options(Options& opts);

  const Options& get_options();

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
#include "../../../../lib/swcdb/core/config/PropertiesParser.cc"
#endif 

#endif // swc_core_config_PropertiesParser_h
