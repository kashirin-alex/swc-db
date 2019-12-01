/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_fsbroker_Settings_h
#define swc_app_fsbroker_Settings_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"
#include "swcdb/fs/Settings.h"

namespace SWC{ namespace Config {


void Settings::init_client_options() { }

void Settings::init_app_options() {
  init_comm_options();
  init_fs_options();
  
  file_desc.add_options()
    ("swc.fs.broker.host", str(), "FsBroker host (default resolve by hostname)") 
    ("swc.fs.broker.port", i32(17000), "FsBroker port")
    
    ("swc.FsBroker.cfg", str(), "Specific cfg-file for FsBroker")
    ("swc.FsBroker.OnFileChange.cfg", str(), "Specific dyn. cfg-file for FsBroker")

    ("swc.FsBroker.reactors", i32(8), "Number of Communication Reactors")
    ("swc.FsBroker.workers", i32(32), "Number of Workers a Reactor")
    ("swc.FsBroker.handlers", i32(8), "Number of App Handlers")
  ;
  alias("host", "swc.fs.broker.host");
}

void Settings::init_post_cmd_args(){
  parse_file(get<std::string>("swc.fs.broker.cfg", ""), "");
  parse_file(get<std::string>("swc.FsBroker.cfg", ""), 
             get<std::string>("swc.FsBroker.OnFileChange.cfg", ""));
}


}}

#endif // swc_app_fsbroker_Settings_h