/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_config_Config_h
#define swc_core_config_Config_h

#include "swcdb/core/FileUtils.h"

#include "swcdb/core/config/Properties.h"


namespace SWC { namespace Config {

class Settings {

  public:

  ParserConfig    cmdline_desc;
  ParserConfig    file_desc;

  std::string     install_path;
  std::string     executable;

  Properties      properties;

  Settings() { 
    cmdline_desc.definition(usage_str());
  }

  virtual ~Settings() { }

  void init(int argc, char *argv[]);

  void init_app_options();

  void init_comm_options();
  
  void init_fs_options();

  void init_client_options();
  
  void init_post_cmd_args();

  void init_process() {
    bool daemon = properties.has("daemon");

    if(daemon) {
      if(pid_t p = fork())
        exit(0);
    }
    auto pid = getpid();

    executable.append(".");
    executable.append(std::to_string((size_t)pid));

    Logger::logger.initialize(executable);
    if(daemon)
      Logger::logger.daemon(properties.get_str("swc.logging.path"));
    
    if(properties.get<gBool>("verbose")) {
      SWC_LOG_OUT(LOG_NOTICE) 
        << "Initialized " << executable << " (" << SWC::VERSION << ")\n"
        << "Process Settings: \n" << properties.to_string() << SWC_LOG_OUT_END;
    }
  }

  const bool has(const std::string &name) const {
    return properties.has(name);
  }

  const bool defaulted(const std::string &name) {
    return properties.defaulted(name);
  }

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

  void alias(const std::string &minor, const std::string &major) {
    properties.alias(minor, major);
  } 

  void parse_args(int argc, char *argv[]);

  void parse_file(const std::string &fname, const std::string &onchg) {
    if(fname.empty())
      return;
    if(!FileUtils::exists(fname))
      HT_THROWF(Error::FS_FILE_NOT_FOUND, 
                "cfg file=%s not found", fname.c_str());
    
    properties.load(fname, file_desc, cmdline_desc, false);
    if(!onchg.empty())
      properties.load_files_by(onchg, file_desc, cmdline_desc, false);
    
    properties.load_from(m_cmd_args);  // Inforce cmdline properties 
  }

  std::string usage_str(const char *usage = 0) {
    if (!usage)
      usage = "Usage: %s [options]\n\nOptions:";

    if (strstr(usage, "%s"))
      return format(usage, executable.c_str());

    return usage;
  }

  private:

  void init_options();

  std::string          cfg_filename;
  Parser::Options      m_cmd_args;
};


}


namespace Env {

class Config {
  
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

  virtual ~Config() {
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
#include "../../lib/swcdb/config/Settings.cc"
#endif 

#endif // swc_core_config_Config_h
