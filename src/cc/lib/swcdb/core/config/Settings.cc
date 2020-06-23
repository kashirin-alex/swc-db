/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/config/Settings.h"

#include "swcdb/core/FileUtils.h"


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
    SWC_PRINT << SWC::COPYRIGHT << SWC_PRINT_CLOSE;

  auto loglevel = get<Property::V_GENUM>("swc.logging.level");
  if(get_bool("debug")) {
    loglevel->set(LOG_DEBUG);

  } else if(loglevel->get() == -1) {
    SWC_LOG_OUT(LOG_ERROR) 
      << "unknown logging level: "<< loglevel->to_string() << SWC_LOG_OUT_END;
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

    ("swc.cfg.path", str(install_path+"/etc/swcdb/"), "Path of configuration files")
    ("swc.cfg", str("swc.cfg"), "Main configuration file")
    ("swc.cfg.dyn", strs(), "Main dynamic configuration file")

    ("swc.logging.level,l", 
      g_enum(
        LOG_INFO,
        [](int value){ Logger::logger.set_level(value); },
        Logger::logger.from_string,
        Logger::logger.repr
      ), 
     "Logging level: debug|info|notice|warn|error|crit|alert|fatal")
    ("swc.logging.path", str(install_path + "/var/log/swcdb/"), "Path of log files")

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
    SWC_PRINT << SWC::VERSION << '\n' 
              << SWC::COPYRIGHT << SWC_PRINT_CLOSE;
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
    fname.append(get_str("swc.cfg.path"));
  fname.append(name);
  
  if(!FileUtils::exists(fname))
    SWC_THROWF(Error::FS_FILE_NOT_FOUND, 
              "cfg file=%s not found", fname.c_str());
    
  load(fname, file_desc, cmdline_desc, false);
  load_files_by(onchg, false);
  load_from(m_cmd_args);  // Inforce cmdline properties 
}

void Settings::load_files_by(const std::string& fileprop,  
                             bool allow_unregistered) {
  if(fileprop.empty() || !has(fileprop))
    return;

  std::string fname;
  Strings files = get<Property::V_STRINGS>(fileprop)->get();
  for (auto it=files.begin(); it<files.end(); ++it) {
    fname.clear();
    if(it->front() != '/' && it->front() != '.')
      fname.append(get_str("swc.cfg.path"));
    fname.append(*it);

    try {
      load(fname, file_desc, cmdline_desc, allow_unregistered);

      std::lock_guard lock(mutex);
      auto it = std::find(m_dyn_files.begin(), m_dyn_files.end(), fname);
      if(it == m_dyn_files.end())
        m_dyn_files.push_back({.filename=fname, .modified=0});
    } catch (std::exception &e) {
      SWC_LOGF(LOG_WARN, "%s has bad cfg file %s: %s",
                fileprop.c_str(), it->c_str(), e.what());
    }
  }
}

void Settings::init_process() {
  bool daemon = has("daemon");

  if(daemon) {
    if(pid_t p = fork())
      exit(0);
  }
  auto pid = getpid();

  executable.append(".");
  executable.append(std::to_string((size_t)pid));

  Logger::logger.initialize(executable);
  if(daemon)
    Logger::logger.daemon(get_str("swc.logging.path"));
    
  if(daemon || get_gbool("verbose")) {
    SWC_LOG_OUT(LOG_NOTICE) 
      << "Initialized " << executable << " "
      << SWC::VERSION << "\n"
      << SWC::COPYRIGHT << '\n' 
      << "Process Settings: \n" << to_string_all() << SWC_LOG_OUT_END;
  }
}

std::string Settings::usage_str(const char *usage) {
  std::string tmp(SWC::COPYRIGHT);
  tmp += '\n';
  tmp.append(usage ? usage : "Usage: %s [options]\n\nOptions:");

  if(strstr(tmp.c_str(), "%s"))
    return format(tmp.c_str(), executable.c_str());
  return tmp;
}


void Settings::check_dynamic_files() {
  time_t ts;
  std::lock_guard lock(mutex);
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

bool Settings::DynFile::operator==(const DynFile& other) const {
  return other == filename;
}
bool Settings::DynFile::operator==(const std::string& other) const {
  return filename.compare(other) == 0;
}


}} // namespace SWC::Config
