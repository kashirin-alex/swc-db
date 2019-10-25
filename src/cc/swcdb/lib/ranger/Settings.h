/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_ranger_Settings_h
#define swc_app_ranger_Settings_h

#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"
#include "swcdb/lib/fs/Settings.h"

namespace SWC{ namespace Config {


void Settings::init_app_options(){
  init_comm_options();
  init_fs_options();
  file_desc().add_options()
    ("swc.rgr.cfg", str(), "Specific cfg-file for Ranger")
    ("swc.rgr.OnFileChange.cfg", str(), "Specific dyn. cfg-file for Ranger")

    ("swc.rgr.reactors", i32(8), "Number of Communication Reactors")
    ("swc.rgr.workers", i32(32), "Number of Workers a Reactor")
    ("swc.rgr.port", i32(16000), "Ranger port")
    ("swc.rgr.handlers", i32(8), "Number of App Handlers")
    ("swc.rgr.maintenance.handlers", i32(2), "Number of Maintenance Handlers")

    ("swc.rgr.id.validation.interval", g_i32(120000), 
    "Validation of Ranger-ID against Mngr(root)")

    ("swc.rgr.compaction.check.interval", g_i32(300000), 
    "Interval in ms for Compaction ")
  ;

}

void Settings::init_post_cmd_args(){ }

}}

#endif // swc_app_ranger_Settings_h