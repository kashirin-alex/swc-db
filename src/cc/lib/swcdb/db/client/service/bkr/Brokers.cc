/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/service/bkr/Brokers.h"

namespace SWC { namespace client {


Brokers::Brokers(const Config::Settings& settings,
                 Comm::IoContextPtr ioctx,
                 const ContextBroker::Ptr& bkr_ctx)
    : queues(std::make_shared<Comm::client::ConnQueues>(
        std::make_shared<Comm::client::Serialized>(
          "BROKER",
          ioctx,
          bkr_ctx ? bkr_ctx : std::make_shared<ContextBroker>(settings)
        ),
         settings.get<Config::Property::V_GINT32>(
          "swc.client.Bkr.connection.timeout"),
         settings.get<Config::Property::V_GINT32>(
          "swc.client.Bkr.connection.probes"),
         settings.get<Config::Property::V_GINT32>(
          "swc.client.Bkr.connection.keepalive"),
         settings.get<Config::Property::V_GINT32>(
          "swc.client.request.again.delay")
      )),
      cfg_hosts(
        settings.get<Config::Property::V_GSTRINGS>("swc.bkr.host")),
      cfg_port(settings.get_i16("swc.bkr.port")) {
  on_cfg_update();
}

void Brokers::on_cfg_update() noexcept {
  try {
    Config::Strings hosts(cfg_hosts->get());
    if(hosts.empty())
      return;
    Comm::EndPoints endpoints;
    endpoints.reserve(hosts.size());

    uint16_t port;
    size_t at;
    for(auto& host_str : hosts) {
      if((at = host_str.find_first_of('|')) == std::string::npos) {
        port = cfg_port;
      } else {
        Config::Property::from_string(host_str.substr(++at), &port);
        host_str = host_str.substr(0, at);
      }
      std::vector<std::string> ips;
      std::string host;
      do {
        auto addr = host_str;
        at = host_str.find_first_of(',');
        if(at != std::string::npos) {
          addr = host_str.substr(0, at);
          host_str = host_str.substr(at+1, host_str.length());
        }
        if(Comm::Resolver::is_ipv4_address(addr) ||
          Comm::Resolver::is_ipv6_address(addr))
          ips.push_back(addr);
        else
          host = addr;
      } while(at != std::string::npos);

      auto tmp = Comm::Resolver::get_endpoints(port, ips, host, {});
      if(!tmp.empty())
        endpoints.insert(endpoints.end(), tmp.begin(), tmp.end());
    }

    if(endpoints.empty())
      return;

    Core::MutexSptd::scope lock(m_mutex);
    m_endpoints = std::move(endpoints);
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
}

Comm::EndPoints Brokers::get_endpoints() {
  Core::MutexSptd::scope lock(m_mutex);
  return m_endpoints;
}

bool Brokers::has_endpoints() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return !m_endpoints.empty();
}


}} //namespace SWC::client

