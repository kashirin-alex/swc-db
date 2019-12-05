/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_client_Settings_h
#define swc_client_Settings_h

#include "swcdb/core/config/Settings.h"
namespace SWC{ namespace Config {

void Settings::init_client_options() {
  file_desc.add_options()
    ("swc.mngr.host", g_strs(gStrings()), 
     "Manager Host: \"[cols range]|(hostname or ips-csv)|port\"")
    ("swc.mngr.port", i16(15000), 
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
}}

#endif // swc_app_manager_Settings_h