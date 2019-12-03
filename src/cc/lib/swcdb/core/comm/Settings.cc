/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/core/config/Settings.h"

namespace SWC{ namespace Config {

void Settings::init_comm_options() {
  file_desc.add_options()
    ("addr", strs(), "IP-port, addr to listen on else resolved(hostname):swc.ServiceName.port")
    ("host", str(),  "host:port to listen on IPv4+IPv6, if port not specified swc.ServiceName.port is used");
}


}}