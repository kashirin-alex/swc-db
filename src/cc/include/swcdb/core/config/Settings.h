/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_config_Config_h
#define swc_core_config_Config_h

#include <memory>
#include "swcdb/core/Error.h"

#include "swcdb/core/config/Properties.h"
#include "swcdb/core/config/PropertiesParser.h"


namespace SWC { namespace Config {

class Settings {

  public:

  ParserConfig    cmdline_desc;
  ParserConfig    file_desc;

  std::string     install_path;
  std::string     executable;

  Properties      properties;

  Settings();

  virtual ~Settings();

  void init(int argc, char *argv[]);

  void init_options();

  void init_app_options();

  void init_comm_options();
  
  void init_fs_options();

  void init_client_options();
  
  void init_post_cmd_args();
 
  void parse_args(int argc, char *argv[]);

  void parse_file(const std::string &fname, const std::string &onchg);

  void init_process();

  const bool has(const std::string &name) const;

  const bool defaulted(const std::string &name);

  template <typename T>
  T get(const std::string &name) {
    return properties.get<T>(name);
  }

  template <typename T>
  T* get_ptr(const std::string &name) {
    return properties.get_ptr<T>(name);
  }

  template <typename T>
  T get(const std::string &name, T default_value) {
    return properties.get<T>(name, default_value);
  }

  void alias(const std::string &minor, const std::string &major);
  
  std::string usage_str(const char *usage = 0);

  private:


  std::string          cfg_filename;
  Parser::Options      m_cmd_args;
};


}


namespace Env {

class Config final {
  
  public:

  typedef std::shared_ptr<Config> Ptr;

  static void init(int argc, char** argv) {
    get()->m_settings->init(argc, argv);
  }

  static void set(Ptr env){
    m_env = env;
  }

  static Ptr get(){
    if(m_env == nullptr)
      m_env = std::make_shared<Config>();
    return m_env;
  }

  static SWC::Config::Settings* settings() {
    HT_ASSERT(m_env != nullptr);
    return m_env->m_settings;
  }

  Config() : m_settings(new SWC::Config::Settings()){}

  ~Config() {
    if(m_settings != nullptr)
      delete m_settings;
  }

  private:
  SWC::Config::Settings*  m_settings = nullptr;
  inline static Ptr       m_env = nullptr;

};
}

}

#ifdef SWC_IMPL_SOURCE
#include "../../../../lib/swcdb/core/config/Settings.cc"
#endif 

#endif // swc_core_config_Config_h
