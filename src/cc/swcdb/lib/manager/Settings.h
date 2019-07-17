/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_Settings_h
#define swc_app_manager_Settings_h

#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"

namespace SWC{ namespace Config {


void Settings::init_app_options(){
  init_comm_options();
  file_desc().add_options()
    ("swc.mngr.reactors", i32(8), "Number of Communication Reactors")
    ("swc.mngr.workers", i32(32), "Number of Workers a Reactor")
    ("swc.mngr.port", i32(15000), "RS-Manager port")

    ("swc.mngr.RoleState.connection.probes", g_i32(3), 
     "Number of tries Mngr tries to connect to other manager")
    ("swc.mngr.RoleState.connection.timeout", g_i32(1000), 
     "Timeout in milliseconds for each connection probe")
    ("swc.mngr.RoleState.request.timeout", g_i32(20000), 
     "Timeout in milliseconds for Mngr-Hosts state request, "
     "total=(connection.probes x connection.timeout + request.timeout) x hosts")

    ("swc.mngr.RoleState.check.interval", g_i32(10000), 
     "Check Interval for Mngr-Hosts-Status changes")

    ("swc.mngr.RoleState.check.delay.fallback", g_i32(30000), 
     "Delay of a Check in milliseconds on Mngr-Host disconnected")
    ("swc.mngr.RoleState.check.delay.updated", g_i32(200), 
     "Delay in milliseconds on Mngr-Hosts-Status changes");
    
}

}}

#endif // swc_app_manager_Settings_h