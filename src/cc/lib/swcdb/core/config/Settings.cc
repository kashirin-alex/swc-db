/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
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

  bool verbose = properties.get<gBool>("verbose");
  if(verbose && properties.get_bool("quiet")) {
    verbose = false;
    properties.set("verbose", (gBool)false);
  }

  gEnumExtPtr loglevel = properties.get_ptr<gEnumExt>("logging-level");
  if(properties.get_bool("debug"))
    loglevel->set_value(LOG_DEBUG);

  if(loglevel->get() == -1){
    SWC_LOG_OUT(LOG_ERROR) << "unknown logging level: "<< loglevel->str() << SWC_LOG_OUT_END;
    std::quick_exit(EXIT_SUCCESS);
  }

  Logger::logger.set_level(loglevel->get());
  loglevel->set_cb_on_chg([](int value){Logger::logger.set_level(value);});
}

void Settings::init_options() {
  gEnumExt logging_level(LOG_INFO);
  logging_level.set_from_string(Logger::logger.from_string)
               .set_repr(Logger::logger.repr);

  cmdline_desc.add_options()
    ("help,h", "Show this help message and exit")
    ("help-config", "Show help message for config properties")
    ("version,v", "Show version information and exit")

    ("daemon ", boo()->zero_token(), "Start process in background mode")
    ("verbose", g_boo(false)->zero_token(), "Show more verbose output")
    ("debug", boo(false)->zero_token(), "Shortcut to --logging-level debug")
    ("quiet", boo(false)->zero_token(), "Negate verbose")

    ("logging-level,l", g_enum_ext(logging_level), 
     "Logging level: debug|info|notice|warn|error|crit|alert|fatal")
    
    ("config", str(install_path + "/etc/swcdb/swc.cfg"), "Configuration file.")
    ("swc.logging.path", str(install_path + "/var/log/swcdb/"), "Path of log files")

    //("induce-failure", str(), "Arguments for inducing failure")
    /* Interactive-Shell options
    ("silent", boo()->zero_token(),
     "as Not Interactive or Show as little output as possible") 
    */
    ;
    
  alias("logging-level", "swc.logging.level");

  file_desc.add_options()
    ("swc.logging.level", g_enum_ext(logging_level), 
     "Logging level: debug|info|notice|warn|error|crit|alert|fatal");
}

void Settings::parse_args(int argc, char *argv[]) {

  Parser prs(false);
  prs.config.add(cmdline_desc);
  prs.config.add(file_desc);
  prs.parse_cmdline(argc, argv);
 
  prs.own_options(m_cmd_args);
  
  properties.load_from(prs.get_options());
    
  // some built-in behavior
  if (has("help")) {
    std::cout << cmdline_desc << std::flush;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (has("help-config")) {
    std::cout << file_desc << std::flush;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (has("version")) {
    std::cout << "Version: " << SWC::VERSION << std::endl;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (!has("config")) 
    return;

  cfg_filename = properties.get_str("config");

  // Only try to parse config file if it exists or not default
  if (FileUtils::exists(cfg_filename)) 
    parse_file(cfg_filename, "swc.OnFileChange.cfg");  
    
  else if (!defaulted("config"))
    SWC_THROWF(Error::FS_FILE_NOT_FOUND, 
              "cfg file=%s not found", cfg_filename.c_str());
}

void Settings::parse_file(const std::string &fname, const std::string &onchg) {
  if(fname.empty())
    return;
  if(!FileUtils::exists(fname))
    SWC_THROWF(Error::FS_FILE_NOT_FOUND, 
              "cfg file=%s not found", fname.c_str());
    
  properties.load(fname, file_desc, cmdline_desc, false);
  if(!onchg.empty())
    properties.load_files_by(onchg, file_desc, cmdline_desc, false);
    
  properties.load_from(m_cmd_args);  // Inforce cmdline properties 
}


void Settings::init_process() {
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

const bool Settings::has(const std::string &name) const {
  return properties.has(name);
}

const bool Settings::defaulted(const std::string &name) {
  return properties.defaulted(name);
}

void Settings::alias(const std::string &minor, const std::string &major) {
  properties.alias(minor, major);
} 

std::string Settings::usage_str(const char *usage) {
  if (!usage)
    usage = "Usage: %s [options]\n\nOptions:";

  if (strstr(usage, "%s"))
    return format(usage, executable.c_str());

  return usage;
}

}} // namespace SWC::Config
