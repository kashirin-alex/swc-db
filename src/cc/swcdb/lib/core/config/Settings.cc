/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "Settings.h"



namespace SWC { namespace Config {


void Settings::parse_args(int argc, char *argv[]) {

  Parser prs(false);
  prs.config.add(cmdline_desc);
  prs.config.add(file_desc);
  prs.parse_cmdline(argc, argv);

  prs.own_options(m_cmd_args);
  
  properties.load_from(prs.get_options());
    
  // some built-in behavior
  if (has("help")) {
    std::cout << usage_str(0) << cmdline_desc << std::flush;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (has("help-config")) {
    std::cout << usage_str(0) << file_desc << std::flush;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (has("version")) {
    std::cout << version_string() << std::endl;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (!has("config")) 
    return;

  cfg_filename = properties.get_str("config");

  // Only try to parse config file if it exists or not default
  if (FileUtils::exists(cfg_filename)) 
    parse_file(cfg_filename, "swc.OnFileChange.cfg");  
    
  else if (!defaulted("config"))
    HT_THROWF(Error::FS_FILE_NOT_FOUND, 
              "cfg file=%s not found", cfg_filename.c_str());
}



void Settings::init_options() {
  gEnumExt logging_level(LOG_INFO);
  logging_level.set_from_string(Logger::logger.from_string)
               .set_repr(Logger::logger.repr);

  cmdline_desc.add_options()
    ("help,h", "Show this help message and exit")
    ("help-config", "Show help message for config properties")
    ("version", "Show version information and exit")

    ("daemon ", boo()->zero_token(), "Start process in background mode")
    ("verbose,v", g_boo(false)->zero_token(), "Show more verbose output")
    ("debug", boo(false)->zero_token(), "Shortcut to --logging-level debug")
    ("quiet", boo(false)->zero_token(), "Negate verbose")

    ("logging-level,l", g_enum_ext(logging_level), 
     "Logging level: debug|info|notice|warn|error|crit|alert|fatal")
    
    ("config", str(install_path + "/conf/swc.cfg"), "Configuration file.")
    ("swc.logging.path", str(install_path + "/log/"), "Path of log files")

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


void Settings::init(int argc, char *argv[]) {
  install_path = std::string(argv[0]);
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


}} // namespace SWC::Config
