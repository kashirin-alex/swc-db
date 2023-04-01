/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/config/PropertiesParser.h"
#include <sstream>
#include <fstream>
#include <iomanip>


namespace SWC {


/* cfg methods for types
*  @param v The default Value and a Type
*/
Config::Property::Value_bool::Ptr
Config::boo(const bool& v) {
  return new Config::Property::Value_bool(v);
}
Config::Property::Value_uint8::Ptr
Config::i8(const uint8_t& v) {
  return new Config::Property::Value_uint8(v);
}
Config::Property::Value_uint16::Ptr
Config::i16(const uint16_t& v) {
  return new Config::Property::Value_uint16(v);
}
Config::Property::Value_int32::Ptr
Config::i32(const int32_t& v) {
  return new Config::Property::Value_int32(v);
}
Config::Property::Value_int64::Ptr
Config::i64(const int64_t& v) {
  return new Config::Property::Value_int64(v);
}
Config::Property::Value_double::Ptr
Config::f64(const double& v) {
  return new Config::Property::Value_double(v);
}
Config::Property::Value_string::Ptr
Config::str(std::string&& v) {
  return new Config::Property::Value_string(std::move(v));
}
Config::Property::Value_strings::Ptr
Config::strs(Strings&& v) {
  return new Config::Property::Value_strings(std::move(v));
}
Config::Property::Value_int64s::Ptr
Config::i64s(Int64s&& v) {
  return new Config::Property::Value_int64s(std::move(v));
}
Config::Property::Value_doubles::Ptr
Config::f64s(Doubles&& v) {
  return new Config::Property::Value_doubles(std::move(v));
}

/* cfg methods for guarded types
*  @param v The default Value and a Type
*/
Config::Property::Value_bool_g::Ptr
Config::g_boo(const bool& v) {
  return new Config::Property::Value_bool_g(v, nullptr);
}
Config::Property::Value_uint8_g::Ptr
Config::g_i8(const uint8_t& v) {
  return new Config::Property::Value_uint8_g(v, nullptr);
}
Config::Property::Value_uint16_g::Ptr
Config::g_i16(const uint16_t& v) {
  return new Config::Property::Value_uint16_g(v, nullptr);
}
Config::Property::Value_int32_g::Ptr
Config::g_i32(const int32_t& v) {
  return new Config::Property::Value_int32_g(v, nullptr);
}
Config::Property::Value_uint64_g::Ptr
Config::g_i64(const uint64_t& v) {
  return new Config::Property::Value_uint64_g(v, nullptr);
}
Config::Property::Value_strings_g::Ptr
Config::g_strs(Strings&& v) {
  return new Config::Property::Value_strings_g(std::move(v), nullptr);
}
Config::Property::Value_enum_g::Ptr
Config::g_enum(const int32_t& v,
               Config::Property::Value_enum_g::OnChg_t&& cb,
               Config::Property::Value_enum_g::FromString_t&& from_string,
               Config::Property::Value_enum_g::Repr_t&& repr) {
  return new Config::Property::Value_enum_g(
    v, std::move(cb), std::move(from_string), std::move(repr));
}

/* cfg methods for types, a skippable option
*  if no option parsed it is skipped
*/
Config::Property::Value_bool::Ptr Config::boo() {
  return new Config::Property::Value_bool(
    true, Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_uint8::Ptr Config::i8() {
  return new Config::Property::Value_uint8(
    0, Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_uint16::Ptr Config::i16() {
  return new Config::Property::Value_uint16(
    0, Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_int32::Ptr Config::i32() {
  return new Config::Property::Value_int32(
    0, Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_int64::Ptr Config::i64() {
  return new Config::Property::Value_int64(
    0, Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_double::Ptr Config::f64() {
  return new Config::Property::Value_double(
    0, Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_string::Ptr Config::str() {
  return new Config::Property::Value_string(
    std::string(), Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_strings::Ptr Config::strs() {
  return new Config::Property::Value_strings(
    Strings(), Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_int64s::Ptr Config::i64s() {
  return new Config::Property::Value_int64s(
    Int64s(), Config::Property::Value::SKIPPABLE);
}
Config::Property::Value_doubles::Ptr Config::f64s() {
  return new Config::Property::Value_doubles(
    Doubles(), Config::Property::Value::SKIPPABLE);
}



namespace Config {


ParserConfig::ParserConfig(int line_len, bool a_own) noexcept
                          : usage(), positions(), options(),
                            line_length(line_len), own(a_own) {
}

ParserConfig::ParserConfig(const char* a_usage, int line_len, bool a_own)
                          : usage(a_usage), positions(), options(),
                            line_length(line_len), own(a_own) {
}

ParserConfig::ParserConfig(const ParserConfig& other)
                          : usage(), positions(), options(),
                            line_length(0), own(true) {
  add(other);
}

ParserConfig::~ParserConfig() noexcept {
  free();
}

void ParserConfig::free() noexcept {
  if(own) {
    for(auto& opt : options)
      delete opt.second.value;
    options.clear();
  }
}

ParserConfig& ParserConfig::definition(const char* u) {
  usage = u;
  return *this;
}

ParserConfig& ParserConfig::definition(std::string&& u) {
  usage = std::move(u);
  return *this;
}

/* populate from other Parser Config */
SWC_SHOULD_NOT_INLINE
ParserConfig& ParserConfig::add(const ParserConfig& other_cfg) {
  usage.append(other_cfg.usage);

  for (auto& pos : other_cfg.positions)
    add_pos(pos.second.c_str(), pos.first);

  for(const auto& kv : other_cfg.options)
    options.emplace(kv.first, kv.second);
    //auto r= if(r.second && !kv.second.aliases.empty()) { } // ?merge alias
  return *this;
}

/* Method to add option */
SWC_SHOULD_NOT_INLINE
ParserConfig& ParserConfig::add(const char* names,
                                Property::Value::Ptr vptr,
                                const char* description) {
  Strings aliases;
  {
    std::string s;
    for(; *names; ++names) {
      if(std::isspace(*names))
        continue;
      if(*names == ',') {
        if(!s.empty())
          aliases.emplace_back(std::move(s));
      } else {
        s += *names;
      }
    }
    if(aliases.empty() || !s.empty())
      aliases.emplace_back(std::move(s));
  }
  ParserOpt& opt = options[aliases.front()];
  opt.value = vptr;
  opt.desc = description;
  opt.aliases.resize(aliases.size() - 1);
  for(size_t n=0; n<opt.aliases.size(); ++n)
    opt.aliases[n] = std::move(aliases[n + 1]);
  return *this;
}

ParserConfig& ParserConfig::operator()(const char* name,
                                       Property::Value::Ptr vptr,
                                       const char* description) {
  return add(name, vptr, description);
}

ParserConfig& ParserConfig::add_options() {
  return *this;
}

ParserConfig& ParserConfig::add_options(const char* name,
                                        Property::Value::Ptr vptr,
                                        const char* description) {
  return add(name, vptr, description);
}

ParserConfig& ParserConfig::add(const char* name, const char* description) {
   return add(name, boo()->zero_token(), description);
}

ParserConfig& ParserConfig::operator()(const char* name,
                                       const char* description) {
  return add(name, description);
}

/* Method to add_pos option */
ParserConfig& ParserConfig::add_pos(const char* s, int pos) {
  positions.emplace_back(pos, s);
  return *this;
}

ParserConfig& ParserConfig::operator()(const char* s, int pos) {
  return add_pos(s, pos);
}

std::string ParserConfig::position_name(int n) {
  for (auto& pos : positions) {
    if(pos.first == n)
      return pos.second;
  }
  return "";
}

SWC_SHOULD_NOT_INLINE
bool ParserConfig::has(const std::string& name) const noexcept {
  for(const auto& info : options) {
    if(Condition::str_eq(name, info.first))
      return true;
    for(const std::string& alias : info.second.aliases) {
      if(Condition::str_eq(name, alias))
        return true;
    }
  }
  return false;
}

SWC_SHOULD_NOT_INLINE
bool ParserConfig::has(const std::string& name,
                       std::string& alias_to) const noexcept {
  for(const auto& info : options) {
    if(Condition::str_eq(name, info.first))
      return true;
    for(const std::string& alias : info.second.aliases) {
      if(Condition::str_eq(name, alias)) {
        alias_to = info.first;
        return true;
      }
    }
  }
  return false;
}

SWC_SHOULD_NOT_INLINE
Property::Value::Ptr ParserConfig::get_default(const std::string& name) {
  for(const auto& info : options) {
    if(Condition::str_eq(name, info.first))
      return info.second.value;
    for(const std::string& alias : info.second.aliases) {
      if(Condition::str_eq(name, alias))
        return info.second.value;
    }
  }
  SWC_THROWF(Error::CONFIG_GET_ERROR, "ParserConfig, getting value of '%s'",
            name.c_str());
}

void ParserConfig::remove(const std::string& name) {
  auto it = options.find(name);
  if(it != options.cend()) {
    if(own)
      delete it->second.value;
    options.erase(it);
  }
}

void ParserConfig::print(std::ostream& os) const {
  os << usage << '\n';

  size_t offset_name = 5;
  size_t offset_desc = 5;
  for (const auto& kv : options) {
    size_t tmp = (!kv.second.aliases.empty()?
               format_list(kv.second.aliases).length()+2 : 0);
    if(offset_name < (tmp += kv.first.length()))
      offset_name = tmp;
    if(offset_desc < kv.second.desc.length())
      offset_desc = kv.second.desc.length();
  }
  offset_name += 2;

  for (const auto& kv : options) {
    os  << std::left << std::setw(2) << ' '
        << std::left << std::setw(offset_name + 2) <<
          format("--%s %s", kv.first.c_str(),
                (!kv.second.aliases.empty()?
                  format("-%s",format_list(kv.second.aliases).c_str()).c_str()
                  : ""))
        << std::left << std::setw(offset_desc+2) << kv.second.desc
        << std::left << std::setw(2) << kv.second.value->to_string()
        << '\n';
  }
}


std::ostream& operator<<(std::ostream& os, const ParserConfig& cfg) {
  cfg.print(os);
  return os;
}




Strings Parser::args_to_strings(int argc, char *argv[]) {
  Strings raw_strings;
  raw_strings.resize(--argc);
  for(int n=0; n<argc; ++n)
    raw_strings[n].append(argv[n + 1]);
  return raw_strings;
}

Parser::Options::Options() noexcept : map() { }

Parser::Options&
Parser::Options::operator=(Parser::Options&& other) noexcept {
  map = std::move(other.map);
  return *this;
}

Parser::Options::~Options() noexcept {
  for(const auto& kv : map)
    delete kv.second;
  map.clear();
}

Parser::Parser(bool unregistered) noexcept
              : config(0, false),
                raw_opts(), m_unregistered(unregistered), m_opts() {
}

Parser::~Parser() noexcept { }

void Parser::free() noexcept {
  config.free();
}

SWC_SHOULD_NOT_INLINE
void Parser::parse_filedata(std::ifstream& in) {
  {
  std::string line;
  for(std::string group; std::getline(in, line); ) {
    if(line.find_first_of("[") == 0) {

      size_t at = line.find_first_of("]");
      if(at != std::string::npos) {

        std::string g_tmp = line.substr(1, at-1);
        if(!group.empty()){
          group.pop_back(); // remove a dot
          if(Condition::str_eq(g_tmp, group+"=end")) {
            // an end of group  "[groupname=end]"
            group.clear();
            line.clear();
            continue;
          }
        }
        group = g_tmp + ".";
        // a start of group "[groupname]"
        line.clear();
        continue;
      }
    }
    parse_line(group + line);
  }
  }
  make_options();
}

void Parser::parse_cmdline(int argc, char *argv[]) {
  parse_cmdline(args_to_strings(argc, argv));
}

SWC_SHOULD_NOT_INLINE
void Parser::parse_cmdline(const Strings& raw_strings) {

  if(raw_strings.empty()) {
    make_options();
    return;
  }

  bool cfg_name;
  bool fill = false;
  std::string name, opt;
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
        opt += '=';

        if(!config.has(name) || !config.get_default(name)->is_zero_token()) {
          if(len_o > n) {
            fill = true;
            continue;
          }
        }
        opt += '1'; // zero-token true default
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
        // if no pos -1  if(n) ignore app-file, unregistered arg-positions
        SWC_THROWF(Error::CONFIG_GET_ERROR,
                    "unknown cfg  with %s", raw_opt.c_str());
    }

    opt.clear();
    fill = false;
  }

  make_options();
}

SWC_SHOULD_NOT_INLINE
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
            name.cend());

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
  } else {
    // remove spaces
    value.erase(std::remove_if(value.begin(), value.end(),
                  [](unsigned char x){return std::isspace(x);}),
                value.cend());
  }
  parse_opt(name+"="+value); // << input must be NAME=VALUE !

  //cfg_name = parse_opt(line);
  // if(!cfg_name)
  //  SWC_LOGF(LOG_WARN, "unknown cfg: %s", line.c_str());
  // line.clear();
}

void Parser::set_pos_parse(const std::string& name, const std::string& value) {
  raw_opts[name].emplace_back(value);
}

SWC_SHOULD_NOT_INLINE
bool Parser::parse_opt(const std::string& s){
  size_t at = s.find_first_of("=");
  if(at == std::string::npos)
    return false;

  std::string name = s.substr(0, at);
  std::string alias_to;
  // SWC_LOGF(LOG_INFO, "looking: %s", name.c_str());
  if(!config.has(name, alias_to) && !m_unregistered)
    return false;

  auto& r = raw_opts[alias_to.length() ? alias_to : name];
  if(s.size() > ++at)
    r.emplace_back(s.c_str() + at, s.size() - at);
  return true;
}

SWC_SHOULD_NOT_INLINE
void Parser::make_options() {
  for(const auto& kv : raw_opts) {
    if(!config.has(kv.first) && m_unregistered)
      add_opt(kv.first, nullptr, kv.second);  // unregistered cfg
    else
      add_opt(kv.first, config.get_default(kv.first), kv.second);
  }
  for (const auto& kv : config.options) {
    if(raw_opts.count(kv.first) || kv.second.value->is_skippable())
      continue;
    // add default kv
    add_opt(kv.first, kv.second.value, {});
  }
}

// convert, validate and add property to options
SWC_SHOULD_NOT_INLINE
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
  opts = std::move(m_opts);
}

const Parser::Options& Parser::get_options() const noexcept {
  return m_opts;
}

void Parser::print(std::ostream& os) const {
  os << "*** Raw Parsed Options:" << '\n';
  for (const auto& kv : raw_opts)
    os << kv.first << '=' << format_list(kv.second) << '\n';
}

void Parser::print_options(std::ostream& os) const {
  os << "*** Parsed Options:" << '\n';
  for(const auto& kv : m_opts.map)
    os << kv.first << '=' << kv.second->to_string() << '\n';
}

std::ostream& operator<<(std::ostream& os, const Parser& prs) {
  prs.print(os);
  return os;
}


}} // namespace SWC::Config
