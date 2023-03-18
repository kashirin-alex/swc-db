/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/Version.h"
#include "swcdb/core/config/Settings.h"

#include <fstream>
#include <filesystem>


#if defined(SWC_PATH_ETC)
#define USE_SWC_PATH_ETC(_) SWC_PATH_ETC
#else
#define USE_SWC_PATH_ETC(_default_) _default_
#endif

#if defined(SWC_PATH_LOG)
#define USE_SWC_PATH_LOG(_) SWC_PATH_LOG
#else
#define USE_SWC_PATH_LOG(_default_) _default_
#endif

#if defined(SWC_PATH_RUN)
#define USE_SWC_PATH_RUN(_) SWC_PATH_RUN
#else
#define USE_SWC_PATH_RUN(_default_) _default_
#endif



namespace SWC { namespace Config {

SWC_SHOULD_NOT_INLINE
Settings::Settings()
    : cmdline_desc(),
      file_desc(), 
      install_path(), 
      executable(), 
      m_cmd_args(),
      m_dyn_files()  {
  cmdline_desc.definition(usage_str());
}

Settings::~Settings() noexcept { }

SWC_SHOULD_NOT_INLINE
void Settings::init(int argc, char *argv[],
                    Settings::init_option_t init_app_options,
                    Settings::init_option_t init_post_cmd_args) {
  {
    std::error_code ec;
    auto path = std::filesystem::read_symlink("/proc/self/exe", ec);
    if(ec) {
      std::cerr << "Problem reading file='/proc/self/exe' ";
      Error::print(std::cerr, ec.value());
      std::cerr << std::endl;
      SWC_QUICK_EXIT(EXIT_FAILURE);
    }

    executable = path.filename().string();
    install_path = (path.parent_path().has_parent_path()
                    ? path.parent_path() : path).parent_path().string();
  }

  init_options();

  if(init_app_options)
    init_app_options(this);

  parse_args(argc, argv);

  if(init_post_cmd_args)
    init_post_cmd_args(this);

  {
    auto verbose = get<Property::Value_bool_g>("verbose");
    if(verbose->get() && get_bool("quiet")) {
      verbose->set(false);
    }
  }

  if(!get_bool("quiet") && !has("daemon")) {
    SWC_PRINT << swcdb_copyrights() << SWC_PRINT_CLOSE;
  }

  {
    auto loglevel = get<Property::Value_enum_g>("swc.logging.level");
    if(get_bool("debug")) {
      loglevel->set(LOG_DEBUG);
    } else if(loglevel->get() == -1) {
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM
        << "unknown logging level: "<< loglevel->to_string();
      );
      SWC_QUICK_EXIT(EXIT_SUCCESS);
    }
  }
}

SWC_SHOULD_NOT_INLINE
void Settings::init_options() {

  cmdline_desc.add_options()
    ("help,h", "Show this help message and exit")
    ("help-config", "Show help message for config properties")
    ("version,v", "Show version information and exit")

    ("verbose", g_boo(false)->zero_token(), "Show more verbose output")
    ("debug", boo(false)->zero_token(), "Shortcut to --swc.logging.level debug")
    ("quiet", boo(false)->zero_token(), "Negate verbose")
    ("daemon ", boo()->zero_token(), "Start process in background mode")

    ("swc.cfg.path",
      str(USE_SWC_PATH_ETC(install_path+"/etc/swcdb/")),
      "Path of configuration files")

    ("swc.cfg", str("swc.cfg"), "Main configuration file")
    ("swc.cfg.dyn", strs(), "Main dynamic configuration file")

    ("swc.logging.level,l",
      g_enum(
        LOG_INFO,
        [](int value) noexcept { Core::logger.set_level(value); },
        Core::logger.from_string,
        Core::logger.repr
      ),
     "Logging level: debug|info|notice|warn|error|crit|alert|fatal")
    ("swc.logging.path",
      str(USE_SWC_PATH_LOG(install_path + "/var/log/swcdb/")),
      "Path of log files")

    //("induce-failure", str(), "Arguments for inducing failure")
    /* Interactive-Shell options
    ("silent", boo()->zero_token(),
     "as Not Interactive or Show as little output as possible")
    */
    ;
}

SWC_SHOULD_NOT_INLINE
void Settings::parse_args(int argc, char *argv[]) {
  {
    Parser prs(false);
    prs.config.add(cmdline_desc);
    prs.config.add(file_desc);
    prs.parse_cmdline(argc, argv);

    prs.own_options(m_cmd_args);

    load_from(prs.get_options());
  }

  // some built-in behavior
  if (has("help")) {
    SWC_PRINT << cmdline_desc << SWC_PRINT_CLOSE;
    SWC_QUICK_EXIT(EXIT_SUCCESS);
  }

  if (has("help-config")) {
    SWC_PRINT << file_desc << SWC_PRINT_CLOSE;
    SWC_QUICK_EXIT(EXIT_SUCCESS);
  }

  if (has("version")) {
    SWC_PRINT << swcdb_version() << '\n'
              << swcdb_copyrights() << SWC_PRINT_CLOSE;
    SWC_QUICK_EXIT(EXIT_SUCCESS);
  }

  if(has("swc.cfg"))
    parse_file(get_str("swc.cfg"), "swc.cfg.dyn");
}

SWC_SHOULD_NOT_INLINE
void Settings::parse_file(const std::string& name, const char* onchg) {
  if(name.empty())
    return;

  std::filesystem::path fname;
  if(name.front() != '/' && name.front() != '.')
    fname = get_str("swc.cfg.path");
  fname.concat(name);
  {
    std::error_code ec;
    if(!std::filesystem::exists(fname, ec))
      SWC_THROWF(ENOENT, "cfg file=%s not found", fname.string().c_str());
  }
  load(fname.string(), file_desc, cmdline_desc, false);
  load_files_by(onchg, false);
  load_from(m_cmd_args);  // Inforce cmdline properties
}

SWC_SHOULD_NOT_INLINE
void Settings::load_files_by(const char* fileprop, bool allow_unregistered) {
  if(!fileprop || !has(fileprop))
    return;
  Strings files;
  {
    auto ptr = get<Property::Value_strings>(fileprop);
    if(!ptr)
      return;
    files = ptr->get();
  }
  for(auto it=files.cbegin(); it != files.cend(); ++it) {
    std::string fname;
    if(it->front() != '/' && it->front() != '.')
      fname.append(get_str("swc.cfg.path"));
    fname.append(*it);

    try {
      load(fname, file_desc, cmdline_desc, allow_unregistered);

      Core::MutexSptd::scope lock(mutex);
      auto it_found = std::find(
        m_dyn_files.cbegin(), m_dyn_files.cend(), fname);
      if(it_found == m_dyn_files.cend())
        m_dyn_files.emplace_back(std::move(fname));
    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_WARN, SWC_LOG_OSTREAM
        << fileprop << " has bad cfg file='" << *it << "': " << e;
      );
    }
  }
}

namespace {

SWC_SHOULD_NOT_INLINE
void write_pid(const std::filesystem::path& pid_file, size_t pid) {
  errno = 0;
  {
    std::ofstream pid_file_buff(pid_file, std::ios::out);
    if(pid_file_buff.is_open()) {
      pid_file_buff << std::to_string(pid);
      pid_file_buff.close();
    }
  }
  if(errno) {
    std::cerr << "Problem writing pid-file=" << pid_file;
    Error::print(std::cerr << ' ', errno);
    std::cerr << std::endl;
    SWC_QUICK_EXIT(EXIT_FAILURE);
  }
}

SWC_SHOULD_NOT_INLINE
std::string get_old_pid(const std::filesystem::path& pid_file) {
  std::string old_pid;
  std::error_code ec;
  if(std::filesystem::exists(pid_file, ec)) {
    errno = 0;
    std::ifstream buffer(pid_file);
    if(buffer.is_open()) {
      buffer >> old_pid;
      buffer.close();
    }
    if(old_pid.empty())
      ec = std::error_code(errno?errno:ECANCELED, std::generic_category());
  }
  if(ec) {
    std::cerr << "Problem reading pid-file=" << pid_file;
    Error::print(std::cerr << ' ', ec.value());
    std::cerr << std::endl;
    SWC_QUICK_EXIT(EXIT_FAILURE);
  }
  return old_pid;
}

SWC_SHOULD_NOT_INLINE
void check_pid(const std::filesystem::path& pid_file,
               const std::string& install_path,
               const std::string& executable) {
  auto old_pid = get_old_pid(pid_file);
  if(old_pid.empty())
    return;
  std::filesystem::path pid_exe("/proc/");
  pid_exe.concat(old_pid);
  pid_exe.concat("/exe");
  std::error_code ec;
  if(std::filesystem::exists(pid_exe, ec)) {
    auto path = std::filesystem::read_symlink(pid_exe, ec);
    if(!ec) {
      std::filesystem::path prog(install_path + "/bin/" + executable);
      if(!Condition::str_cmp(path.string().c_str(), prog.string().c_str(),
                             prog.string().size())) { // incl. " (deleted)"
        std::cerr << "Problem executing '" << executable
                  << "', process already running-pid=" << old_pid
                  << ", stop it first" << std::endl;
        SWC_QUICK_EXIT(EXIT_FAILURE);
      }
    }
  }
  if(ec) {
    std::cerr << "Problem reading running-pid='" << pid_exe << "' ";
    Error::print(std::cerr, ec.value());
    std::cerr << std::endl;
    SWC_QUICK_EXIT(EXIT_FAILURE);
  }
  std::filesystem::remove(pid_file, ec);
}

SWC_SHOULD_NOT_INLINE
void update_pid(const std::string& install_path,
                const std::string& executable,
                uint16_t port,
                size_t pid) {
  std::filesystem::path pid_file(USE_SWC_PATH_RUN(install_path + "/run/"));
  pid_file.concat(executable);
  if(port) {
    pid_file.concat(".");
    pid_file.concat(std::to_string(port));
  }
  pid_file.concat(".pid");

  check_pid(pid_file, install_path, executable);
  write_pid(pid_file, pid);

  std::cout << "Executing '"<< executable
            << "' with pid-file=" << pid_file << std::endl;
}

SWC_SHOULD_NOT_INLINE
void print_init(const Settings* settings) {
  SWC_LOG_OUT(
    LOG_NOTICE,
    settings->print(SWC_LOG_OSTREAM
      << "Initialized " << settings->executable << ' '
      << swcdb_version() << '\n'
      << swcdb_copyrights() << '\n'
      << "Process Settings: \n");
  );
}

}

SWC_SHOULD_NOT_INLINE
void Settings::init_process(bool with_pid_file, const char* port_cfg) {
  bool daemon = has("daemon");
  #if defined(__MINGW64__) || defined(_WIN32)
    daemon = false;
    #define fork() false
  #endif

  if(daemon && fork())
    SWC_QUICK_EXIT(EXIT_SUCCESS);

  size_t pid = getpid();
  if(with_pid_file) {
    update_pid(
      install_path,
      executable,
      port_cfg && !defaulted(port_cfg) ? get_i16(port_cfg) : 0,
      pid
    );
  }

  executable.append(".");
  executable.append(std::to_string(pid));

  Core::logger.initialize(executable);
  if(daemon)
    Core::logger.daemon(get_str("swc.logging.path"));

  if(daemon || get_gbool("verbose"))
    print_init(this);
}

std::string Settings::usage_str(const char* usage) {
  std::string tmp(swcdb_copyrights());
  tmp += '\n';
  if(!usage)
    tmp.append(format("Usage: %s [options]\n\nOptions:", executable.c_str()));
  else if(strstr(usage, "%s"))
    tmp.append(format_unsafe(usage, executable.c_str()));
  else
    tmp.append(usage);
  return tmp;
}


namespace {

SWC_SHOULD_NOT_INLINE
time_t file_mtime(const char* filename) {
  errno = 0;
  struct stat statbuf;
  if(::stat(filename, &statbuf)) {
    SWC_LOGF(LOG_WARN, "cfg-file '%s' err(%s)",
             filename, Error::get_text(errno));
    return 0;
  }
  return statbuf.st_mtime;
}
}

SWC_SHOULD_NOT_INLINE
void Settings::check_dynamic_files() noexcept {
  try {
    Core::MutexSptd::scope lock(mutex);
    for(auto& dyn : m_dyn_files) {
      time_t mtime = file_mtime(dyn.filename.c_str());
      if(mtime && dyn.modified != mtime) {
        dyn.modified = mtime;
        reload(dyn.filename, file_desc, cmdline_desc);
        SWC_LOGF(LOG_DEBUG, "dyn-cfg-file '%s' checked", dyn.filename.c_str());
      }
    }
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}


}} // namespace SWC::Config
