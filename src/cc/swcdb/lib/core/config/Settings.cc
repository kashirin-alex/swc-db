/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "Settings.h"

#include "../Version.h"
#include "../FileUtils.h"

#include <filesystem>


namespace SWC { namespace Config {


void Settings::parse_args(int argc, char *argv[]) {

  Parser prs(false);
  prs.config.add(cmdline_desc);
  prs.config.add(file_desc);
  prs.parse_cmdline(argc, argv);

  m_cmd_args = prs.get_options();
  properties.load_from(m_cmd_args);
    
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
  gEnumExt logging_level(Logger::Priority::INFO);
  logging_level.set_from_string(Logger::cfg::from_string).set_repr(Logger::cfg::repr);
        
  cmdline_desc.add_options()
    ("help,h", "Show this help message and exit")
    ("help-config", "Show help message for config properties")
    ("version", "Show version information and exit")
    ("verbose,v", g_boo(false)->zero_token(), "Show more verbose output")
    ("debug", boo(false)->zero_token(), "Shortcut to --logging-level debug")
    ("quiet", boo(false)->zero_token(), "Negate verbose")
    ("swc.logging.level,logging-level,l", g_enum_ext(logging_level), 
     "Logging level: debug|info|notice|warn|error|crit|alert|fatal")
    ("config", str(install_path + "/conf/swc.cfg"), "Configuration file.")
    ("induce-failure", str(), "Arguments for inducing failure")
    
    /* Interactive-Shell options
    ("silent", boo()->zero_token(),
     "as Not Interactive or Show as little output as possible") 
    */
    ;
  alias("logging-level", "swc.logging.level");
}

void Settings::init_client_options() {

  file_desc.add_options()
    ("swc.mngr.host", g_strs(gStrings()), 
     "Manager Host: \"[cols range]|(hostname or ips-csv)|port\"")
    ("swc.mngr.port", i32(15000), 
     "Manager default port if not defined in swc.mngr.host")
     
    ("swc.client.Rgr.connection.timeout", g_i32(10000), 
     "Ranger client connect timeout")
    ("swc.client.Rgr.connection.probes", g_i32(1), 
     "Ranger client connect probes")
    ("swc.client.Rgr.connection.keepalive", g_i32(30000), 
     "Ranger client connection keepalive for ms since last action")

    ("swc.client.Mngr.connection.timeout", g_i32(10000), 
     "Manager client connect timeout")
    ("swc.client.Mngr.connection.probes", g_i32(1), 
     "Manager client connect probes")
    ("swc.client.Mngr.connection.keepalive", g_i32(30000), 
     "Manager client connection keepalive for ms since last action")
    ("swc.client.schema.expiry", g_i32(1800000), 
     "Schemas expiry in ms")
    ;
} 

void Settings::init(int argc, char *argv[]) {
  executable = std::string(argv[0]);
   
  auto at = executable.find_last_of("/");
  Logger::initialize(executable.substr(at?++at:at, executable.length()));

  install_path = std::filesystem::absolute(argv[0]).parent_path().parent_path();

  init_options();

  init_app_options();

  parse_args(argc, argv);

  init_post_cmd_args();
  
  gEnumExtPtr loglevel = properties.get_ptr<gEnumExt>("logging-level");
  bool verbose = properties.get<gBool>("verbose");

  if (verbose && properties.get_bool("quiet")) {
    verbose = false;
    properties.set("verbose", (gBool)false);
  }
  if (properties.get_bool("debug")) {
    loglevel->set_value(Logger::Priority::DEBUG);
  }
  if(loglevel->get()==-1){
    HT_ERROR_OUT << "unknown logging level: "<< loglevel->str() << HT_END;
    std::quick_exit(EXIT_SUCCESS);
  }
  
  Logger::get()->set_level(loglevel->get());
  loglevel->set_cb_on_chg([](int value){Logger::get()->set_level(value);});

  if (verbose) {
    HT_NOTICE_OUT << "Initializing " << executable << " (SWC-DB "
        << version_string() << ")..." << HT_END;
  }

}


}} // namespace SWC::Config
