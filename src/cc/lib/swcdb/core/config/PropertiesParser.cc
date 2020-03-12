/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include <iostream>
#include <iomanip>
#include <fstream>

#include "swcdb/core/Error.h"
#include "swcdb/core/config/PropertiesParser.h"

namespace SWC {


/* cfg methods for types
*  @param v The default Value and a Type
*/
Property::V_BOOL::Ptr boo(const bool& v) {
  return new Property::V_BOOL(v);
}
Property::V_UINT8::Ptr i8(const uint8_t& v) {
  return new Property::V_UINT8(v);
}
Property::V_UINT16::Ptr i16(const uint16_t& v) {
  return new Property::V_UINT16(v);
}
Property::V_INT32::Ptr i32(const int32_t& v) {
  return new Property::V_INT32(v);
}
Property::V_INT64::Ptr i64(const int64_t& v) {
  return new Property::V_INT64(v);
}
Property::V_DOUBLE::Ptr f64(const double& v) {
  return new Property::V_DOUBLE(v);
}
Property::V_STRING::Ptr str(const std::string& v) {
  return new Property::V_STRING(v);
}
Property::V_STRINGS::Ptr strs(const Strings& v) {
  return new Property::V_STRINGS(v);
}
Property::V_INT64S::Ptr i64s(const Int64s& v) {
  return new Property::V_INT64S(v);
}
Property::V_DOUBLES::Ptr f64s(const Doubles& v) {
  return new Property::V_DOUBLES(v);
}

/* cfg methods for guarded types
*  @param v The default Value and a Type
*/
Property::V_GBOOL::Ptr g_boo(const bool& v) {
  return new Property::V_GBOOL(v, 0);
}
Property::V_GUINT8::Ptr g_i8(const uint8_t& v) {
  return new Property::V_GUINT8(v, 0);
}
Property::V_GINT32::Ptr g_i32(const int32_t& v) {
  return new Property::V_GINT32(v, 0);
}
Property::V_GSTRINGS::Ptr g_strs(const Strings& v) {
  return new Property::V_GSTRINGS(v, 0);
}
Property::V_GENUM::Ptr g_enum(const int32_t& v, 
                              const Property::V_GENUM::OnChg_t& cb, 
                              const Property::V_GENUM::FromString_t& from_string,
                              const Property::V_GENUM::Repr_t& repr) {
  return new Property::V_GENUM(v, cb, from_string, repr);
}

/* cfg methods for types, a skippable option
*  if no option parsed it is skipped
*/
Property::V_BOOL::Ptr boo() {
  return new Property::V_BOOL(true, Property::Value::SKIPPABLE);
}
Property::V_UINT8::Ptr i8() {
  return new Property::V_UINT8(0, Property::Value::SKIPPABLE);
}
Property::V_UINT16::Ptr i16() {
  return new Property::V_UINT16(0, Property::Value::SKIPPABLE);
}
Property::V_INT32::Ptr i32() {
  return new Property::V_INT32(0, Property::Value::SKIPPABLE);
}
Property::V_INT64::Ptr i64() {
  return new Property::V_INT64(0, Property::Value::SKIPPABLE);
}
Property::V_DOUBLE::Ptr f64() {
  return new Property::V_DOUBLE(0, Property::Value::SKIPPABLE);
}
Property::V_STRING::Ptr str() {
  return new Property::V_STRING(std::string(), Property::Value::SKIPPABLE);
}
Property::V_STRINGS::Ptr strs() {
  return new Property::V_STRINGS(Strings(), Property::Value::SKIPPABLE);
}
Property::V_INT64S::Ptr i64s() {
  return new Property::V_INT64S(Int64s(), Property::Value::SKIPPABLE);
}
Property::V_DOUBLES::Ptr f64s() {
  return new Property::V_DOUBLES(Doubles(), Property::Value::SKIPPABLE);
}



namespace Config {

  
ParserConfig::ParserConfig(const std::string& usage, 
                            int line_len, bool own)
                          : usage(usage), line_length(line_len), own(own) {
}

ParserConfig::ParserConfig(const ParserConfig& other) {
  add(other);
}

ParserConfig::~ParserConfig() {
  free();
}

void ParserConfig::free() {
  if(own) {
    for(auto& opt : options)
      delete opt.second.value;
    options.clear();
  }
}
  
ParserConfig& ParserConfig::definition(const std::string& u) {
  usage = u;
  return *this;
}

/* populate from other Parser Config */
ParserConfig& ParserConfig::add(const ParserConfig& other_cfg) {
  usage.append(other_cfg.usage);
      
  for (auto& pos : other_cfg.positions) 
    add_pos(pos.second, pos.first);

  for(const auto &kv : other_cfg.options)
    options.emplace(kv.first, kv.second);
    //auto r= if(r.second && !kv.second.aliases.empty()) { } // ?merge alias
  return *this;
}

/* Method to add option */
ParserConfig& ParserConfig::add(const std::string& names, 
                                Property::Value::Ptr vptr, 
                                const std::string& description) {
  Strings aliases;
  std::istringstream f(names);
  std::string s;    
  while (getline(f, s, ',')) {
    s.erase(std::remove_if(s.begin(), s.end(),
    [](unsigned char x){return std::isspace(x);}), s.end());
    aliases.push_back(s);
  }  
  ParserOpt& opt = options[aliases.front()];
  opt.value = vptr;
  opt.desc = description;
  opt.aliases.assign(aliases.begin()+1, aliases.end());
  return *this;
}

ParserConfig& ParserConfig::operator()(const std::string& name, 
                                       Property::Value::Ptr vptr, 
                                       const std::string& description) {
  return add(name, vptr, description);
}

ParserConfig& ParserConfig::add_options(){
  return *this;
}

ParserConfig& ParserConfig::add_options(const std::string &name, 
                                        Property::Value::Ptr vptr, 
                                        const std::string& description){
  return add(name, vptr, description);
}

ParserConfig& ParserConfig::add(const std::string &name, 
                                const std::string& description){
   return add(name, boo()->zero_token(), description);
}

ParserConfig& ParserConfig::operator()(const std::string &name, 
                                       const std::string&  description) {
  return add(name, description);
}
    
/* Method to add_pos option */
ParserConfig& ParserConfig::add_pos(const std::string s, int pos){
  positions.emplace_back(pos, s);
  return *this;
}

ParserConfig& ParserConfig::operator()(const std::string s, int pos) {
  return add_pos(s, pos);
}

const std::string ParserConfig::position_name(int n){
  for (auto & pos : positions) {
    if(pos.first == n) 
      return pos.second;
  }
  return "";
}

const bool ParserConfig::has(const std::string& name) const {
  for(const MapPair& info : options) {
    if(name.compare(info.first) == 0)
      return true;
    for(const std::string& alias : info.second.aliases)
      if(name.compare(alias) == 0)
        return true;
    }
  return false;
}

const bool ParserConfig::has(const std::string& name, 
                             std::string& alias_to) const {
  for(const MapPair& info : options) {
    if(name.compare(info.first) == 0)
      return true;
    for(const std::string& alias : info.second.aliases)
      if(name.compare(alias) == 0) {
        alias_to = info.first;
        return true;
      }
  }
  return false;    
}

Property::Value::Ptr ParserConfig::get_default(const std::string& name){
  for(const MapPair& info : options) {
    if(name.compare(info.first) == 0)
      return info.second.value;
    for(const std::string& alias : info.second.aliases)
      if(name.compare(alias) == 0)
        return info.second.value;
  }
  SWC_THROWF(Error::CONFIG_GET_ERROR, "ParserConfig, getting value of '%s'",
            name.c_str());
}

void ParserConfig::remove(const std::string& name) {
  auto it = options.find(name);
  if(it != options.end())
    options.erase(it);
}

void ParserConfig::print(std::ostream& os) const {
  os << usage << "\n";
  
  size_t tmp;
  size_t len_name=5;
  size_t len_desc=5;
  for (const auto &kv : options) {
    tmp = (!kv.second.aliases.empty()? 
            format_list(kv.second.aliases).length()+2 : 0);
    if(len_name < tmp+kv.first.length())
      len_name = tmp+kv.first.length();
    if(len_desc < kv.second.desc.length())
      len_desc = kv.second.desc.length();
  }
  int offset_name = static_cast<int>(len_name)+2;
  int offset_desc = static_cast<int>(len_desc);

  for (const auto &kv : options) {
    os  << std::left << std::setw(2) << " "
        << std::left << std::setw(offset_name+2) << 
          format("--%s %s", kv.first.c_str(),
                (!kv.second.aliases.empty()? 
                  format("-%s",format_list(kv.second.aliases).c_str()).c_str()
                  : ""))
        << std::left << std::setw(offset_desc+2) << kv.second.desc
        << std::left << std::setw(2) << kv.second.value->to_string()
        << "\n";
  }
}


std::ostream& operator<<(std::ostream& os, const ParserConfig& cfg) {
  cfg.print(os);
  return os;
}




Strings Parser::args_to_strings(int argc, char *argv[]) {
  Strings raw_strings;
  for(int n=1; n<argc; ++n)  
    raw_strings.emplace_back(argv[n]);
  return raw_strings;
}

Parser::Options::Options(bool own) : own(own) { }

Parser::Options::~Options() {
  free();
}

void Parser::Options::free() {
    if(own) {
      for(const auto &kv : map)
        delete kv.second;
      map.clear();
    }
  }


Parser::Parser(bool unregistered) 
              : m_unregistered(unregistered), config("", 0, false) { 
}
  
Parser::~Parser() {
  free();
}

void Parser::free() {
  m_opts.free();
  config.free();
}
  
void Parser::parse_filedata(std::ifstream &in) {
  size_t at;
  std::string group = "";
  std::string line, g_tmp;
  while(getline (in, line)){
    at = line.find_first_of("[");
    if(at == (size_t)0){

      at = line.find_first_of("]");      
      if(at != std::string::npos) {

        g_tmp = line.substr(1, at-1);  
        if(!group.empty()){
          group.pop_back(); // remove a dot      
          if(g_tmp.compare(group+"=end") == 0) { 
            // an end of group  "[groupname=end]"
            group.clear();
            line.clear();
            continue;
          }
        }
        group = g_tmp+".";               
        // a start of group "[groupname]"
        line.clear();
        continue;
      }
    }
    parse_line(group+line);

  }
  make_options();
}

void Parser::parse_cmdline(int argc, char *argv[]) { 
  parse_cmdline(args_to_strings(argc, argv));
}

void Parser::parse_cmdline(const Strings& raw_strings) { 

  if(raw_strings.empty()) {
    make_options();
    return;
  }

  bool cfg_name;
  bool fill = false;
  std::string name, alias, opt;
  int len_o = raw_strings.size();

  int n = 0;
  for(const std::string& raw_opt: raw_strings) {
    ++n;

    // if arg is a --name / -name
    if(!fill && raw_opt.find_first_of("-", 0, 1) != std::string::npos) {
      name = raw_opt.substr(1);
      if(name.find_first_of("-", 0, 1) != std::string::npos)
        name = name.substr(1);
      opt.append(name);

      // if not arg with name=Value 
      if(name.find_first_of("=") == std::string::npos){
        opt.append("=");

        if(!config.has(name) || !config.get_default(name)->is_zero_token()) {
          if(len_o > n) {
            fill = true;
            continue;
          }
        }
        opt.append("1"); // zero-token true default 
      }
    } else {
      // position based name with position's value
      name = config.position_name(n);
      if(!name.empty()) {
        set_pos_parse(name, raw_opt);
        continue;
      }
      opt.append(raw_opt);
    }
    
    // SWC_LOGF(LOG_INFO, "parsing: %s", opt.c_str());
    cfg_name = parse_opt(opt); // << input need to be NAME=VALUE else false
    if(!cfg_name) {
      name = config.position_name(-1);
      if(!name.empty())
        set_pos_parse(name, raw_opt);
      else if(!m_unregistered) 
        // if no pos -1  if(n!=0) ignore app-file, unregistered arg-positions
        SWC_THROWF(Error::CONFIG_GET_ERROR, 
                    "unknown cfg  with %s", raw_opt.c_str());
    }

    opt.clear();
    fill = false;
  }
  
  make_options();
}

void Parser::parse_line(const std::string& line) {
  size_t at = line.find_first_of("="); // is a cfg type
  if(at == std::string::npos) 
    return;
      
  std::string name = line.substr(0, at);
  size_t cmt = name.find_first_of("#"); // is comment in name
  if(cmt != std::string::npos) 
    return;
      
  name.erase(std::remove_if(name.begin(), name.end(),
                [](unsigned char x){return std::isspace(x);}),
            name.end());

  std::string value = line.substr(at+1);
  cmt = value.find_first_of("#"); // remove followup comment
  if(cmt != std::string::npos)
    value = value.substr(0, cmt);

  at = value.find_first_of("\"");
  if(at != std::string::npos) {   
    // quoted value
    value = value.substr(at+1);
    at = value.find_first_of("\"");
    if(at != std::string::npos)
      value = value.substr(0, at);
  } else                 
    // remove spaces
    value.erase(std::remove_if(value.begin(), value.end(),
                  [](unsigned char x){return std::isspace(x);}),
                value.end());

  parse_opt(name+"="+value); // << input must be NAME=VALUE !

  //cfg_name = parse_opt(line);
  // if(!cfg_name)
  //  SWC_LOGF(LOG_WARN, "unknown cfg: %s", line.c_str());
  // line.clear();
}

void Parser::set_pos_parse(const std::string& name, const std::string& value) {
  raw_opts[name].push_back(value);
}

const bool Parser::parse_opt(const std::string& s){
  size_t at = s.find_first_of("=");
  if(at == std::string::npos) 
    return false;

  std::string name = s.substr(0, at);
  std::string alias_to;
  // SWC_LOGF(LOG_INFO, "looking: %s", name.c_str());
  if(!config.has(name, alias_to) && !m_unregistered)
    return false;
    
  auto& r = raw_opts[alias_to.length() ? alias_to : name];
  std::string value = s.substr(at+1);
  // SWC_LOGF(LOG_INFO, "value: %s", value.c_str());
  if(value.empty()) 
    return true;
  r.push_back(value);
  return true;
}

void Parser::make_options() {
  for(const auto &kv : raw_opts) {
    if(!config.has(kv.first) && m_unregistered)
      add_opt(kv.first, nullptr, kv.second);  // unregistered cfg
    else
      add_opt(kv.first, config.get_default(kv.first), kv.second);
  }
  for (const auto &kv : config.options) {
    if(raw_opts.count(kv.first) || kv.second.value->is_skippable())
      continue;
    // add default kv
    add_opt(kv.first, kv.second.value, {});
  }
}

// convert, validate and add property to options
void Parser::add_opt(const std::string& name, Property::Value::Ptr p, 
                     const Strings& raw_opt) {
  auto tmp = p ? p : str();
  auto& p_set = m_opts.map.emplace(name, tmp->make_new(raw_opt)).first->second;
  if(!p)
    delete tmp;
  
  if(raw_opt.empty())
    p_set->default_value(true);
}
      
void Parser::own_options(Parser::Options& opts) {
  opts = m_opts;
  m_opts.own = false;
}

const Parser::Options& Parser::get_options(){
  return m_opts;
}

void Parser::print(std::ostream& os) const {
  os << std::string("*** Raw Parsed Options:") << "\n";
  for (const auto &kv : raw_opts)
    os << kv.first << "=" << format_list(kv.second) << "\n";
}

void Parser::print_options(std::ostream& os) const {
  os << std::string("*** Parsed Options:") << "\n";
  for(const auto &kv : m_opts.map)
    os << kv.first << "=" << kv.second->to_string() << "\n";
}

std::ostream& operator<<(std::ostream& os, const Parser& prs) {
  prs.print(os);
  return os;
}


}} // namespace SWC::Config
