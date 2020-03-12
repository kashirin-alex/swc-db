/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_lib_utils_ShellRanger_h
#define swc_lib_utils_ShellRanger_h


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/AppContext.h"
#include "swcdb/db/Protocol/Rgr/req/Report.h"

namespace SWC { namespace Utils { namespace shell {


class Rgr : public Interface {
  public:
  Rgr() 
    : Interface("\033[32mSWC-DB(\033[36mrgr\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-ranger-history") {
    options.push_back(
      new Option(
        "report", 
        {"report Ranger state, the loaded columns and ranges",
        "report '[endpoint,]|hostname'|'port';"},
        [ptr=this](std::string& cmd){return ptr->report(cmd);}, 
        new re2::RE2(
          "(?i)^(report)")
      )
    );
  
    Env::Clients::init(
      std::make_shared<client::Clients>(
        nullptr,
        std::make_shared<client::AppContext>()
      )
    );
  }

  const bool report(std::string& cmd) {
    std::string message;
    
    size_t at = cmd.find_first_of(" ");
    std::string host_or_ips = cmd.substr(at+1);
    
    host_or_ips.erase(
      std::remove_if(host_or_ips.begin(), host_or_ips.end(), 
                    [](unsigned char x){return std::isspace(x);} ), 
      host_or_ips.end()
    );
    
    uint32_t port;
    at = host_or_ips.find_first_of("|");
    if(at != std::string::npos) {
      std::string port_str = host_or_ips.substr(at+1);
      host_or_ips = host_or_ips.substr(0, at);
      try {
        if((port = std::stol(port_str)) > UINT16_MAX )
          err = ERANGE;
      } catch(std::exception e ) {
        err = ERANGE;
      }
      if(err) {
        message.append("Bad value='"+port_str+ "' for port\n");
        return error(message);
      }
    } else {
      port = Env::Config::settings()->get_i16("swc.rgr.port");
    }
    
    std::vector<std::string> ips;
    std::string host;
    do{
      auto addr = host_or_ips;
      at = addr.find_first_of(",");
      if(at != std::string::npos) { 
        addr = host_or_ips.substr(0, at);
        host_or_ips = host_or_ips.substr(at+1, host_or_ips.length());
      }
      if(Resolver::is_ipv4_address(addr) || Resolver::is_ipv6_address(addr))
        ips.push_back(addr);
      else
        host = addr;
  
    }while(at != std::string::npos);

    EndPoints endpoints;
    try {
      endpoints = Resolver::get_endpoints(port, ips, host, false);
      if(endpoints.empty()) {
        message.append("Empty endpoints\n");
        err = EINVAL;
      }
    } catch(Exception& e) {
      err = e.code();
      message.append(e.what());
      message.append("\n");
    }
    if(err)
      return error(message);
    
    std::promise<void>  r_promise;
    Protocol::Rgr::Req::Report::request(
      Protocol::Rgr::Params::ReportReq(), 
      endpoints, 
      [this, await=&r_promise]() {
        await->set_value();
        err = Error::COMM_CONNECT_ERROR;
      },
      [this, await=&r_promise] 
      (client::ConnQueue::ReqBase::Ptr req, 
       const Protocol::Rgr::Params::ReportRsp& rsp) {
        if(!(err = rsp.err)) {
          std::scoped_lock lock(Logger::logger.mutex);
          rsp.display(std::cout);
        }
        await->set_value();
      }
    );
    r_promise.get_future().wait();

    if(err) {
      message.append(Error::get_text(err));
      message.append("\n");
      return error(message);
    }
    return true;
  }
};


}}} // namespace Utils::shell

#endif // swc_lib_utils_ShellRanger_h