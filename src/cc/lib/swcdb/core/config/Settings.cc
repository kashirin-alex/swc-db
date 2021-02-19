/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/Version.h"
#include "swcdb/core/config/Settings.h"

#include "swcdb/core/FileUtils.h"
#include <fstream>


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

Settings::Settings() {
  cmdline_desc.definition(usage_str());
}

Settings::~Settings() { }

void Settings::init(int argc, char *argv[]) {

  char path[1024];
  errno = 0;
  ssize_t r = readlink("/proc/self/exe", path, 1024);
  if(r == -1 || errno)
    throw;

  install_path = std::string(path, r); // install_path = std::string(argv[0]);

  auto at = install_path.find_last_of("/");
  if(at == std::string::npos) {
    executable = install_path;
    install_path = ".";
  } else {
    executable = install_path.substr(at?at+1:at, install_path.length());
    install_path = install_path.substr(0, at);
    at = install_path.find_last_of("/");
    install_path = at == std::string::npos ? "." : install_path.substr(0, at);
  }

  init_options();

  init_app_options();

  parse_args(argc, argv);

  init_post_cmd_args();

  auto verbose = get<Property::V_GBOOL>("verbose");
  if(verbose->get() && get_bool("quiet")) {
    verbose->set(false);
  }

  if(!get_bool("quiet") && !has("daemon"))
    SWC_PRINT << swcdb_copyrights() << SWC_PRINT_CLOSE;

  auto loglevel = get<Property::V_GENUM>("swc.logging.level");
  if(get_bool("debug")) {
    loglevel->set(LOG_DEBUG);

  } else if(loglevel->get() == -1) {
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM
      << "unknown logging level: "<< loglevel->to_string();
    );
    std::quick_exit(EXIT_SUCCESS);
  }
}

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

void Settings::parse_args(int argc, char *argv[]) {

  Parser prs(false);
  prs.config.add(cmdline_desc);
  prs.config.add(file_desc);
  prs.parse_cmdline(argc, argv);

  prs.own_options(m_cmd_args);

  load_from(prs.get_options());

  // some built-in behavior
  if (has("help")) {
    SWC_PRINT << cmdline_desc << SWC_PRINT_CLOSE;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (has("help-config")) {
    SWC_PRINT << file_desc << SWC_PRINT_CLOSE;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (has("version")) {
    SWC_PRINT << swcdb_version() << '\n'
              << swcdb_copyrights() << SWC_PRINT_CLOSE;
    std::quick_exit(EXIT_SUCCESS);
  }

  if(has("swc.cfg"))
    parse_file(get_str("swc.cfg"), "swc.cfg.dyn");
}

void Settings::parse_file(const std::string& name, const std::string& onchg) {
  if(name.empty())
    return;

  std::string fname;
  if(name.front() != '/' && name.front() != '.')
    fname = get_str("swc.cfg.path");
  fname.append(name);

  if(!FileUtils::exists(fname))
    SWC_THROWF(ENOENT, "cfg file=%s not found", fname.c_str());

  load(fname, file_desc, cmdline_desc, false);
  load_files_by(onchg, false);
  load_from(m_cmd_args);  // Inforce cmdline properties
}

void Settings::load_files_by(const std::string& fileprop,
                             bool allow_unregistered) {
  if(fileprop.empty() || !has(fileprop))
    return;
  auto ptr = get<Property::V_STRINGS>(fileprop);
  if(!ptr)
    return;
  std::string fname;
  Strings files = ptr->get();
  for(auto it=files.cbegin(); it != files.cend(); ++it) {
    if(it->front() != '/' && it->front() != '.')
      fname.append(get_str("swc.cfg.path"));
    fname.append(*it);

    try {
      load(fname, file_desc, cmdline_desc, allow_unregistered);

      Core::MutexSptd::scope lock(mutex);
      auto it = std::find(m_dyn_files.begin(), m_dyn_files.end(), fname);
      if(it == m_dyn_files.end())
        m_dyn_files.emplace_back(std::move(fname));
    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_WARN, SWC_LOG_OSTREAM
        << fileprop << " has bad cfg file " << *it << ": " << e;
      );
    }
    fname.clear();
  }
}

void Settings::init_process(bool with_pid_file, const std::string& port_cfg) {
  bool daemon = has("daemon");

  if(daemon) {
    if(fork())
      exit(0);
  }

  std::string pid_file;
  if(with_pid_file) {
    pid_file = USE_SWC_PATH_RUN(install_path + "/run/") + executable;
    if(!port_cfg.empty() && !defaulted(port_cfg)) {
      pid_file.append(".");
      pid_file.append(std::to_string(get_i16(port_cfg)));
    }
    pid_file.append(".pid");

    if(FileUtils::exists(pid_file)) {
      errno = 0;
      std::string old_pid;
      std::ifstream buffer(pid_file);
      if(buffer.is_open()) {
        buffer >> old_pid;
        buffer.close();
      }
      if(old_pid.empty()) {
        std::cerr << "Problem reading pid-file='" << pid_file << '\'';
        if(errno)
          Error::print(std::cerr << ' ', errno);
        std::cerr << std::endl;
        quick_exit(EXIT_FAILURE);
      }

      char path[1024];
      std::string pid_exe("/proc/" + old_pid + "/exe");
      errno = 0;
      ssize_t r = readlink(pid_exe.c_str(), path, 1024);
      if(errno != ENOENT) {
        if(r == -1 || errno) {
          std::cerr << "Problem reading running-pid='" << pid_exe << '\'';
          if(errno)
            Error::print(std::cerr << ' ', errno);
          std::cerr << std::endl;
          quick_exit(EXIT_FAILURE);
        }
        std::string prog_path(install_path + "/bin/" + executable);
        if((size_t(r) == prog_path.size() ||
            (size_t(r) > prog_path.size() && path[prog_path.size()] == ' ')
           ) && !strncmp(prog_path.c_str(), path, prog_path.size())) {
          std::cerr << "Problem executing '" << executable
                    << "', process already running-pid=" << old_pid
                    << ", stop it first" << std::endl;
          quick_exit(EXIT_FAILURE);
        }
      }

      FileUtils::unlink(pid_file);
    }
  }

  auto pid = getpid();

  if(with_pid_file) {
    errno = 0;
    std::ofstream pid_file_buff(pid_file, std::ios::out);
    if(pid_file_buff.is_open()) {
      pid_file_buff << std::to_string(size_t(pid));
      pid_file_buff.close();
    }
    if(errno) {
      std::cerr << "Problem writing pid-file='" << pid_file << '\'';
      if(errno)
        Error::print(std::cerr << ' ', errno);
      std::cerr << std::endl;
      quick_exit(EXIT_FAILURE);
    }

    std::cout << "Executing '"<< executable
              << "' with pid-file='" << pid_file << '\'' << std::endl;
  }

  executable.append(".");
  executable.append(std::to_string(size_t(pid)));

  Core::logger.initialize(executable);
  if(daemon)
    Core::logger.daemon(get_str("swc.logging.path"));

  if(daemon || get_gbool("verbose")) {
    SWC_LOG_OUT(LOG_NOTICE, SWC_LOG_OSTREAM
      << "Initialized " << executable << " "
      << swcdb_version() << "\n" << swcdb_copyrights() << '\n'
      << "Process Settings: \n" << to_string_all();
    );
  }
}

std::string Settings::usage_str(const char *usage) {
  std::string tmp(swcdb_copyrights());
  tmp += '\n';
  tmp.append(usage ? usage : "Usage: %s [options]\n\nOptions:");

  if(strstr(tmp.c_str(), "%s"))
    return format(tmp.c_str(), executable.c_str());
  return tmp;
}


void Settings::check_dynamic_files() {
  time_t ts;
  Core::MutexSptd::scope lock(mutex);
  for(auto& dyn : m_dyn_files) {
    errno = 0;
    ts = FileUtils::modification(dyn.filename);
    if(errno > 0) {
      SWC_LOGF(LOG_WARN, "cfg-file '%s' err(%s)",
               dyn.filename.c_str(), Error::get_text(errno));
      continue;
    }
    if(ts == dyn.modified)
      continue;
    dyn.modified = ts;
    reload(dyn.filename, file_desc, cmdline_desc);
    SWC_LOGF(LOG_DEBUG, "dyn-cfg-file '%s' checked", dyn.filename.c_str());
  }
}


Settings::DynFile::DynFile(std::string&& filename) noexcept
                          : filename(std::move(filename)),
                            modified(0) {
}

bool Settings::DynFile::operator==(const DynFile& other) const noexcept {
  return other == filename;
}
bool Settings::DynFile::operator==(const std::string& other) const noexcept {
  return !filename.compare(other);
}


}} // namespace SWC::Config
