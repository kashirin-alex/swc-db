/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/config/Settings.h"

namespace SWC{ namespace Config {

void Settings::init_comm_options() {
  file_desc.add_options()
    ("addr", strs(), 
     "IP-port, addr to listen on else resolved(hostname):swc.ServiceName.port")
    ("host", str(),  
     "host:port to listen on IPv4+6, swc.ServiceName.port applied if port not specified")
    
    ("swc.comm.network.priority", strs({}), "Network Priority Access")

    ("swc.comm.ssl", boo(false), "Use SSL in comm layer")
    ("swc.comm.ssl.secure.network", strs({}),
     "Networks that do not require SSL")
    ("swc.comm.ssl.ciphers", str(), "Ciphers to use")
    ("swc.comm.ssl.subject_name", str(), 
      "CRT/Cluster's domain-name, if set SRV-CRT is verified")
    ("swc.comm.ssl.crt", str("cluster.crt"), "Cluster Certificate file")
    ("swc.comm.ssl.key", str("server.key"), "Server Private-Key file")
    ("swc.comm.ssl.ca", str(), "CA, used if set")
     
  ;
}


}}