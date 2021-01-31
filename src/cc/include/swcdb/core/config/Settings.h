/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_config_Config_h
#define swcdb_core_config_Config_h

#include "swcdb/core/Exception.h"
#include "swcdb/core/config/Properties.h"
#include "swcdb/core/config/PropertiesParser.h"


namespace SWC {


/**
 * @brief The SWC-DB Configurations C++ namespace 'SWC::Config'
 *
 * \ingroup Core
 */
namespace Config {

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

  void init_process(bool with_pid_file,
                    const std::string& port_cfg = nullptr);

  std::string usage_str(const char *usage = nullptr);

  void check_dynamic_files();

  private:

  Parser::Options      m_cmd_args;

  struct DynFile {
    const std::string filename;
    time_t            modified;
    bool operator==(const DynFile& other) const noexcept;
    bool operator==(const std::string& other) const noexcept;
  };
  std::vector<DynFile> m_dyn_files;
};


}


//! The SWC-DB Environments C++ namespace 'SWC::Env'
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
    if(!m_env)
      m_env = std::make_shared<Config>();
    return m_env;
  }

  static SWC::Config::Settings* settings() {
    SWC_ASSERT(m_env);
    return m_env->m_settings;
  }

  Config() : m_settings(new SWC::Config::Settings()){}

  ~Config() {
    if(m_settings)
      delete m_settings;
  }

  private:
  SWC::Config::Settings*  m_settings;
  inline static Ptr       m_env = nullptr;

};
}

}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Settings.cc"
#endif

#endif // swcdb_core_config_Config_h
