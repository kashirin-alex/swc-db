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

  typedef std::shared_ptr<Settings> Ptr;
  typedef void init_option_t(Settings*);

  ParserConfig    cmdline_desc;
  ParserConfig    file_desc;

  std::string     install_path;
  std::string     executable;

  Settings();

  ~Settings() noexcept;

  void init(int argc, char *argv[],
            init_option_t app, init_option_t post_cmd_args);

  void init_options();

  void parse_args(int argc, char *argv[]);

  void load_files_by(const char* fileprop, bool allow_unregistered);

  void parse_file(const std::string& fname, const char* onchg);

  void init_process(bool with_pid_file, const char* port_cfg = nullptr);

  std::string usage_str(const char *usage = nullptr);

  void check_dynamic_files() noexcept;

  private:

  Parser::Options      m_cmd_args;

  struct DynFile {
    const std::string filename;
    time_t            modified;
    SWC_CAN_INLINE
    DynFile(std::string&& a_filename) noexcept
            : filename(std::move(a_filename)), modified(0) {
    }
    SWC_CAN_INLINE
    ~DynFile() noexcept { }
    SWC_CAN_INLINE
    bool operator==(const DynFile& other) const noexcept {
      return other == filename;
    }
    SWC_CAN_INLINE
    bool operator==(const std::string& other) const noexcept {
      return Condition::str_eq(filename, other);
    }
  };
  Core::Vector<DynFile> m_dyn_files;
};


}


//! The SWC-DB Environments C++ namespace 'SWC::Env'
namespace Env {

class Config final {

  public:

  typedef std::shared_ptr<Config> Ptr;

  SWC_SHOULD_NOT_INLINE
  static void init(
        int argc,
        char** argv,
        SWC::Config::Settings::init_option_t init_app_options,
        SWC::Config::Settings::init_option_t init_post_cmd_args) {
    get()->m_settings->init(argc, argv, init_app_options, init_post_cmd_args);
  }

  static void set(Ptr env) noexcept {
    m_env = env;
  }

  static Ptr& get() {
    if(!m_env)
      m_env.reset(new Config());
    return m_env;
  }

  static SWC::Config::Settings::Ptr& settings() {
    //SWC_ASSERT(m_env);
    return m_env->m_settings;
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  Config() : m_settings(new SWC::Config::Settings()) { }

  ~Config() noexcept { }

  private:
  SWC::Config::Settings::Ptr m_settings;
  inline static Ptr          m_env = nullptr;

};
}

}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/config/Settings.cc"
#endif

#endif // swcdb_core_config_Config_h
