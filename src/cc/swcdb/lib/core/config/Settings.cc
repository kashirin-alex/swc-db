/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/** @file
 * Configuration settings.
 * This file contains the global configuration settings (read from file or
 * from a command line parameter).
 */

#include "Settings.h"

#include "swcdb/lib/core/Version.h"
#include "swcdb/lib/core/FileUtils.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>

#include <errno.h>

namespace SWC { namespace Config {


static String filename;

namespace{ //local
static int terminal_line_length() {
  // int n = System::term_info().num_cols;
  // return n > 0 ? n : 80;
  return 80;
}
}

PropertiesDesc& Settings::cmdline_desc(const char *usage) {
  std::lock_guard<std::recursive_mutex> lock(rec_mutex);

  if (!cmdline_descp)
    cmdline_descp = new PropertiesDesc(usage_str(usage), terminal_line_length());

  return *cmdline_descp;
}

void Settings::cmdline_desc( PropertiesDesc &desc) {
  std::lock_guard<std::recursive_mutex> lock(rec_mutex);

  if (cmdline_descp)
    delete cmdline_descp;

  cmdline_descp = new PropertiesDesc(desc);
}


PropertiesDesc& Settings::file_desc(const char *usage) {
  std::lock_guard<std::recursive_mutex> lock(rec_mutex);

  if (!file_descp)
    file_descp = new PropertiesDesc(usage ? usage : "Settings Properties",
            terminal_line_length());

  return *file_descp;
}

void Settings::file_desc( PropertiesDesc &desc) {
  std::lock_guard<std::recursive_mutex> lock(rec_mutex);

  if (file_descp)
    delete file_descp;

  file_descp = new PropertiesDesc(desc);
}

void Settings::parse_args(int argc, char *argv[]) {
  std::lock_guard<std::recursive_mutex> lock(rec_mutex);

  m_cmd_args = Parser(argc, argv, cmdline_desc(), file_desc(), false)
                    .get_options();
  properties->load_from(m_cmd_args);
    
  // some built-in behavior
  if (has("help")) {
    std::cout << cmdline_desc() << std::flush;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (has("help-config")) {
    std::cout << file_desc() << std::flush;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (has("version")) {
    std::cout << version_string() << std::endl;
    std::quick_exit(EXIT_SUCCESS);
  }

  if (!has("config")) 
    // continue with Policies do not set/require config
    return;

  filename = properties->get_str("config");

  // Only try to parse config file if it exists or not default
  if (FileUtils::exists(filename)) {
    parse_file(filename, "swc.OnFileChange.cfg");
    
  } else if (!defaulted("config"))
    HT_THROWF(Error::FS_FILE_NOT_FOUND, 
              "cfg file=%s not found", filename.c_str());
}

void Settings::parse_file(const String &fname, const String &onchg) {
  if(!fname.empty()){
    if(!FileUtils::exists(fname))
      HT_THROWF(Error::FS_FILE_NOT_FOUND, 
                "cfg file=%s not found", fname.c_str());

    properties->load(fname, file_desc(), false);
    if(!onchg.empty())
      properties->load_files_by(onchg, file_desc(), false);
    
    properties->load_from(m_cmd_args);  // Inforce cmdline properties 
  }
}

void Settings::alias(const String &cmdline_opt, const String &file_opt) {
  properties->alias(cmdline_opt, file_opt);
}

bool Settings::has(const String &name) {
  HT_ASSERT(properties);
  return properties->has(name);
}

bool Settings::defaulted(const String &name) {
  HT_ASSERT(properties);
  return properties->defaulted(name);
}


void Settings::init_options() {
  String default_config = install_path + "/conf/swc.cfg";
  String default_data_dir = install_path;
    
  gEnumExt logging_level(Logger::Priority::INFO);
  logging_level.set_from_string(Logger::cfg::from_string).set_repr(Logger::cfg::repr);
        
  cmdline_desc().add_options()
    ("help,h", "Show this help message and exit")
    ("help-config", "Show help message for config properties")
    ("version", "Show version information and exit")
    ("verbose,v", g_boo(false)->zero_token(), "Show more verbose output")
    ("debug", boo(false)->zero_token(), "Shortcut to --logging-level debug")
    ("quiet", boo(false)->zero_token(), "Negate verbose")
    ("silent", boo()->zero_token(),
     "as Not Interactive or Show as little output as possible")
    ("logging-level,l", g_enum_ext(logging_level), 
     "Logging level: debug, info, notice, warn, error, crit, alert, fatal")
    ("config", str(default_config), "Configuration file.")
    ("induce-failure", str(), "Arguments for inducing failure")
    ("timeout,t", i32(), "System wide timeout in milliseconds")
    ;
  alias("logging-level", "swc.logging.level");
  alias("timeout", "Hypertable.Request.Timeout");

  file_desc().add_options()
    ("swc.mngr.host", g_strs(gStrings()), 
     "RS-Manager Host: \"[cols range]|(hostname or ips-csv)|port\"")
    ("swc.mngr.port", i32(15000), 
     "RS-Manager default port if not defined in swc.mngr.host")
    ;
    
  /*
  alias("Hypertable.RangeServer.CommitLog.RollLimit",
        "Hypertable.CommitLog.RollLimit");
  */
}


void Settings::init(int argc, char *argv[]) {

  executable = std::string(argv[0]);
  install_path = std::filesystem::absolute(argv[0]).parent_path().parent_path();

  init_options();

  init_app_options();

  parse_args(argc, argv);

  init_post_cmd_args();
  
  gEnumExtPtr loglevel = properties->get_ptr<gEnumExt>("logging-level");
  bool verbose = properties->get<gBool>("verbose");

  if (verbose && properties->get_bool("quiet")) {
    verbose = false;
    properties->set("verbose", (gBool)false);
  }
  if (properties->get_bool("debug")) {
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
