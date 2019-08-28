/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_Settings_h
#define swc_app_manager_Settings_h

#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"
#include "swcdb/lib/fs/Settings.h"

namespace SWC{ namespace Config {


void Settings::init_app_options(){
  init_comm_options();
  init_fs_options();
  file_desc().add_options()
    ("swc.mngr.cfg", str(), "Specific cfg-file for RS-Manager")
    ("swc.mngr.OnFileChange.cfg", str(), "Specific dyn. cfg-file for RS-Manager")

    ("swc.mngr.reactors", i32(8), "Number of Communication Reactors")
    ("swc.mngr.workers", i32(32), "Number of Workers a Reactor")
    ("swc.mngr.port", i32(15000), "RS-Manager port")
    ("swc.mngr.handlers", i32(8), "Number of App Handlers")


    ("swc.mngr.role.connection.probes", g_i32(3), 
     "Number of tries Mngr tries to connect to other manager")
    ("swc.mngr.role.connection.timeout", g_i32(1000), 
     "Timeout in milliseconds for each connection probe")
    ("swc.mngr.role.connection.fallback.failures", g_i32(10), 
     "Fallback only after this number of retries to connect")
     
    ("swc.mngr.role.request.timeout", g_i32(20000), 
     "Timeout in milliseconds for Mngr-Hosts state request, "
     "total=(connection.probes x connection.timeout + request.timeout) x hosts")

    ("swc.mngr.role.check.interval", g_i32(10000), 
     "Check Interval for Mngr-Hosts-Status changes")

    ("swc.mngr.role.check.delay.fallback", g_i32(30000), 
     "Delay of a Check in milliseconds on Mngr-Host disconnected")
    ("swc.mngr.role.check.delay.updated", g_i32(200), 
     "Delay in milliseconds on Mngr-Hosts-Status changes")


    ("swc.mngr.ranges.assign.RS.connection.timeout", g_i32(30000), 
     "Timeout for connection establishing with RangeServer")
    ("swc.mngr.ranges.assign.RS.connection.probes", g_i32(3), 
     "Number of connection probes")
    ("swc.mngr.ranges.assign.RS.remove.failures", g_i32(255), 
     "Number of failures(establishing conn.x probes) after which the RangeServer is removed")

    ("swc.mngr.ranges.assign.delay.onRangeServerChange", g_i32(30000), 
     "Delay of Ranges Assignment Check in milliseconds on RS(on/off)")
    ("swc.mngr.ranges.assign.delay.afterColumnsInit", g_i32(30000), 
     "Delay of Ranges Assignment Check in milliseconds follow columns init")
    ("swc.mngr.ranges.assign.interval.check", g_i32(60000), 
     "Ranges assignment interval in milliseconds between checks")

  ;
  
}

void Settings::init_post_cmd_args(){ }

}}

#endif // swc_app_manager_Settings_h