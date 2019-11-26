/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_config_PropertiesParser_h
#define swc_core_config_PropertiesParser_h

#include <iostream>
#include <iomanip>
#include <fstream>

#include <map>

namespace SWC {


// Config::cfg(Int32tSafe default_value, true)
template<typename T>
inline Property::Value::Ptr cfg(T v = 0, bool skippable=false, bool guarded=false) {
  return new Property::Value(v, skippable, guarded);
}

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

/* cfg methods for guarded  types, a skippable option
*  if no option parsed it is skipped
*/
Property::Value::Ptr g_strs();


 namespace Config {

  /** @addtogroup Common
   *  @{
   */
  
struct ParserOpt {
  Property::Value::Ptr  value;
  Strings             aliases;
  std::string              desc;
};

class ParserConfig {
  typedef std::map<std::string, ParserOpt> Map;
  typedef std::pair<std::string, ParserOpt> MapPair;
  typedef std::pair<Map::iterator, bool> InsRet;

  typedef std::pair<int, std::string> PosPair;
  typedef std::vector<PosPair> Positions;

  public:

    ParserConfig();
    ParserConfig(const std::string& usage, int line_len=0);

    /* populate from other Parser Config */
    ParserConfig &add(ParserConfig other_cfg);

    virtual ~ParserConfig(){}

    /* Method to add option */
    ParserConfig &add(const std::string& names, Property::Value::Ptr vptr, 
                      const std::string& description);

    ParserConfig &operator()(const std::string& name, Property::Value::Ptr vptr, 
                            const std::string& description) {
      return add(name, vptr, description);
    }

    ParserConfig &add_options(){
      return *this;
    }

    ParserConfig &add_options(const std::string &name, Property::Value::Ptr vptr, 
                              const std::string& description){
      return add(name, vptr, description);
    }

    ParserConfig &add(const std::string &names, const std::string& description){
      return add(names, cfg(true, true)->zero_token(), description);
    }

    ParserConfig &operator()(const std::string &name, const std::string&  description) {
      return add(name, description);
    }
    
    /* Method to add_pos option */
    ParserConfig &add_pos(const std::string s, int pos);

    ParserConfig &operator()(const std::string s, int pos) {
      return add_pos(s, pos);
    }

    operator ParserConfig*(){
      return this;
    }

    Map get_map(){
      return m_cfg;
    }

    const std::string get_usage(){
      return m_usage;
    }

    Positions get_positions(){
      return m_poss;
    }

    bool has(const std::string& name);

    Property::Value::Ptr get_default(const std::string& name);

    void print(std::ostream& os);

    const std::string position_name(int n);

  private:
    std::string    m_usage;
    Map       m_cfg;
    int       m_line_length = 0;
    Positions m_poss;
};

std::ostream& operator<<(std::ostream& os,  ParserConfig& cfg);




class Parser{
  public:
    typedef std::map<std::string, Property::Value::Ptr> Options;
    typedef std::pair<std::string, Property::Value::Ptr> OptPair;

    Parser(std::ifstream &in, ParserConfig cfg, bool unregistered=false);

    Strings arsg_to_strings(int argc, char *argv[]){
      Strings raw_strings;
      for(int n=1;n<argc;n++)  raw_strings.push_back(std::string(argv[n]));
      return raw_strings;
    }
    Parser(int argc, char *argv[], ParserConfig *main, 
           ParserConfig *hidden=nullptr, bool unregistered=false)
      : Parser(arsg_to_strings(argc, argv), main, hidden, unregistered){      
    }

    Parser(const Strings& raw_strings, ParserConfig *main, 
           ParserConfig *hidden=nullptr, bool unregistered=false);

    void parse_line(const std::string& line);

    void set_pos_parse(const std::string& name, const std::string& value);

    bool parse_opt(const std::string& s);
    
    void print(std::ostream& os);

    void make_options();

    // convert, validate and add property to options
    void add_opt(const std::string& name, Property::Value::Ptr p, const Strings& raw_opt);
    
    void print_options(std::ostream& os);
    
    Options get_options(){
      return m_opts;
    }

    ParserConfig get_cfg(){
      return m_cfg;
    }
    virtual ~Parser(){}
  private:
    typedef std::pair<std::string, Strings> Pair;
    typedef std::map<std::string, Strings> Map;
    typedef std::pair<Map::iterator, bool> InsRet;
    Map raw_opts;

    ParserConfig m_cfg;
    bool m_unregistered;
    Options m_opts;
    
};

std::ostream& operator<<(std::ostream& os,  Parser& prs);


/** @} */
} // namespace Config
} // namespace SWC

#endif // Common_PropertiesParser_h
