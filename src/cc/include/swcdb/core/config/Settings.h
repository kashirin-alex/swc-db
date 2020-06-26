/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_config_Config_h
#define swc_core_config_Config_h

#include <memory>
#include "swcdb/core/Error.h"

#include "swcdb/core/config/Properties.h"
#include "swcdb/core/config/PropertiesParser.h"


namespace SWC { namespace Config {

class Settings final : public Properties {

  public:

  ParserConfig    cmdline_desc;
  ParserConfig    file_desc;

  std::string     install_path;
  std::string     executable;

  Settings();

  ~Settings();

  void init(int argc, char *argv[]);

  void init_options();

  void init_app_options();

  void init_comm_options();
  
  void init_fs_options();

  void init_client_options();
  
  void init_post_cmd_args();
 
  void parse_args(int argc, char *argv[]);

  void load_files_by(const std::string& fileprop, 
                     bool allow_unregistered);

  void parse_file(const std::string& fname, const std::string& onchg);

  void init_process();

  std::string usage_str(const char *usage = 0);

  void check_dynamic_files();

  private:

  Parser::Options      m_cmd_args;

  struct DynFile {
    const std::string filename;
    time_t            modified;
    bool operator==(const DynFile& other) const;
    bool operator==(const std::string& other) const;
  };
  std::vector<DynFile> m_dyn_files;
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
    SWC_ASSERT(m_env != nullptr);
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
#include "swcdb/core/config/Settings.cc"
#endif 

#endif // swc_core_config_Config_h
