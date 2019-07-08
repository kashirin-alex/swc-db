/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_rangeserver_Settings_h
#define swc_app_rangeserver_Settings_h

#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"

namespace SWC{ namespace Config {


void Settings::init_app_options(){
  init_comm_options();
  file_desc().add_options()
    ("swc.rs.reactors", i32(8), "Number of Communication Reactors")
    ("swc.rs.workers", i32(32), "Number of Workers a Reactor")
    ("swc.rs.port", i32(15001), "RangeServer port");

}


}}

#endif // swc_app_rangeserver_Settings_h