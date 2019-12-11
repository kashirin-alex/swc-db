/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/core/config/Properties.h"

namespace SWC {

  Properties::Properties() { }
  
  Properties::~Properties() { 
    reset();
  }

  void Properties::reset() {
    for (const auto &kv : m_map)
      delete kv.second;
    m_map.clear();
  }

  void Properties::load_from(const Config::Parser::Options& opts, 
                             bool only_guarded) {
    for(const auto &kv : opts.map) {
      if(has(kv.first) && (kv.second->is_default() || 
                           (only_guarded && !kv.second->is_guarded())))
      continue;
      set(kv.first, kv.second);
    }
  }

  void Properties::load(const std::string &fname, 
                        const Config::ParserConfig &filedesc,
                        const Config::ParserConfig &cmddesc,
                        bool allow_unregistered, 
                        bool only_guarded) {
    Config::Parser prs(false);
    prs.config.add(filedesc);
    prs.config.add(cmddesc);
    
    std::ifstream in(fname.c_str());
    prs.parse_filedata(in);

    load_from(prs.get_options(), only_guarded);
  } 
  
  void Properties::load_files_by(const std::string &fileprop, 
                                 const Config::ParserConfig &filedesc,
                                 const Config::ParserConfig &cmddesc,
	                               bool allow_unregistered) {
    if(fileprop.empty() || !has(fileprop)) 
      return;

    Strings files = get_strs(fileprop);
    for (auto it=files.begin(); it<files.end(); it++){
	    try {
        load(*it, filedesc, cmddesc, allow_unregistered);
      } catch (std::exception &e) {
		    SWC_LOGF(LOG_WARN, "%s has bad cfg file %s: %s", 
                  fileprop.c_str(), it->c_str(), e.what());
      }
    }
  }
    
  std::string Properties::reload(const std::string &fname, 
                                 const Config::ParserConfig &filedesc,
                                 const Config::ParserConfig &cmddesc,
	                               bool allow_unregistered) {
    std::string out;
	  try {
      out.append("\n\nCurrent Configurations:\n");
      out.append(to_string());

      load(fname, filedesc, cmddesc, allow_unregistered, true);

      out.append("\n\nNew Configurations:\n");
      out.append(to_string());
      return out;
	  }
	  catch (std::exception &e) {
		  SWC_LOGF(LOG_WARN, "Error::CONFIG_BAD_CFG_FILE %s: %s", fname.c_str(), e.what());
      return format("Error::CONFIG_BAD_CFG_FILE %s: %s \noutput:\n%s", 
                      fname.c_str(), e.what(), out.c_str());
	  }
  }

  void Properties::alias(const std::string &primary, 
                         const std::string &secondary) {
    m_alias_map[primary] = secondary;
    m_alias_map[secondary] = primary;
  }

  Property::Value::Ptr Properties::get_value_ptr(const std::string &name) {
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

  const bool Properties::has(const std::string &name) const {
    if(m_map.count(name))
      return true;
      
    auto alias = m_alias_map.find(name);
    if(alias == m_alias_map.end()) 
      return false;
    return m_map.count(alias->second);
  }

  const bool Properties::defaulted(const std::string &name) {
    return get_value_ptr(name)->is_default();
  }

  const std::string Properties::str(const std::string &name) {
    return get_value_ptr(name)->str();
  }
  
  void Properties::get_names(std::vector<std::string> &names) const {
    for(auto it = m_map.begin(); it != m_map.end(); it++)
      names.push_back(it->first);
  }

  void Properties::remove(const std::string &name) {
    auto it = m_map.find(name);
    if(it != m_map.end()) {
      delete it->second;
      m_map.erase(it);
    }
  }

  void Properties::set(const std::string &name, Property::Value::Ptr p) {
    Property::Value::Ptr p_set;
    if(!m_map.count(name)) {
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

  void Properties::print(std::ostream &out, bool include_default) const {
    out << to_string(include_default);
  }

  const std::string Properties::to_string(bool include_default) const {
    std::string out;
    bool isdefault;
    for(const auto &kv : m_map) {
      isdefault = kv.second->is_default();
      if(include_default || !isdefault) {
        out.append(format("%s=%s", kv.first.c_str(), kv.second->str().c_str()));
        if(isdefault)
          out.append(" (default)");
        out.append("\n");
      }
    }
    return out;
  }

  bool Properties::get_bool(const std::string &name) {
    return get<bool>(name); 
  }
  std::string Properties::get_str(const std::string &name) {
    return get<std::string>(name); 
  }
  Strings Properties::get_strs(const std::string &name) {
    return get<Strings>(name); 
  }
  uint16_t Properties::get_i16(const std::string &name) {
    return get<uint16_t>(name); 
  }
  int32_t Properties::get_i32(const std::string &name) {
    return get<int32_t>(name); 
  }
  int64_t Properties::get_i64(const std::string &name) {
    return get<int64_t>(name); 
  }
  Int64s Properties::get_i64s(const std::string &name) {
    return get<Int64s>(name); 
  }
  double Properties::get_f64(const std::string &name) {
    return get<double>(name); 
  }
  Doubles Properties::get_f64s(const std::string &name) {
    return get<Doubles>(name); 
  }
  bool Properties::get_bool(const std::string &name, bool default_value) {
    return get<bool>(name, default_value); 
  }
  std::string Properties::get_str(const std::string &name, std::string &default_value) {
    return get<std::string>(name, static_cast<std::string>(default_value)); 
  }
  std::string Properties::get_str(const std::string &name, std::string default_value) {
    return get<std::string>(name, default_value); 
  }
  Strings Properties::get_strs(const std::string &name, Strings default_value) {
    return get<Strings>(name, default_value); 
  }
  uint16_t Properties::get_i16(const std::string &name, uint16_t default_value) {
    return get<uint16_t>(name, default_value); 
  }
  int32_t Properties::get_i32(const std::string &name, int32_t default_value) {
    return get<int32_t>(name, default_value); 
  }
  int64_t Properties::get_i64(const std::string &name, int64_t default_value) {
    return get<int64_t>(name, default_value); 
  }
  Int64s Properties::get_i64s(const std::string &name, Int64s &default_value) {
    return get<Int64s>(name, default_value); 
  }
  double Properties::get_f64(const std::string &name, double default_value) {
    return get<double>(name, default_value); 
  }
  Doubles Properties::get_f64s(const std::string &name, Doubles default_value) {
    return get<Doubles>(name, default_value); 
  }



}

