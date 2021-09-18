/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/service/bkr/Brokers.h"

namespace SWC { namespace client {


Brokers::Brokers(const Config::Settings& settings,
                 Comm::IoContextPtr ioctx,
                 const ContextBroker::Ptr& bkr_ctx)
    : queues(new Comm::client::ConnQueues(
        Comm::client::Serialized::Ptr(new Comm::client::Serialized(
          settings,
          "BROKER",
          ioctx,
          bkr_ctx
            ? bkr_ctx
            : ContextBroker::Ptr(new ContextBroker(settings))
        )),
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
    if(hosts.empty()) {
      set({});
      return;
    }
    BrokersEndPoints _brokers;
    _brokers.reserve(hosts.size());

    uint16_t port;
    size_t at;
    for(auto& host_str : hosts) {
      if(host_str.empty())
        continue;
      if((at = host_str.find_first_of('|')) == std::string::npos) {
        port = cfg_port;
      } else {
        Config::Property::from_string(host_str.c_str() + at + 1, &port);
        host_str = host_str.substr(0, at);
      }
      Config::Strings ips;
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
        _brokers.emplace_back(std::move(tmp));
    }

    SWC_LOG_OUT(LOG_DEBUG,
      SWC_LOG_OSTREAM << "Broker Hosts (size=" << _brokers.size() << "): ";
      for(size_t n=0; n<_brokers.size(); ++n) {
        SWC_LOG_OSTREAM  << "\n  " << n << ": ";
        for(auto& e : _brokers[n])
          SWC_LOG_OSTREAM  << e << ',';
      }
    );
    set(_brokers);

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }

}

size_t Brokers::size() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_brokers.size();
}

bool Brokers::get(Brokers::BrokerIdx& idx, Comm::EndPoints& endpoints) {
  Core::MutexSptd::scope lock(m_mutex);
  return m_brokers.empty()
    ? false
    : !(endpoints = m_brokers[idx.pos >= m_brokers.size()
                        ? (idx.pos = 0)
                        : idx.pos]).empty();
}

bool Brokers::has_endpoints() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return !m_brokers.empty() && !m_brokers.front().empty();
}

void Brokers::set(BrokersEndPoints&& _brokers) {
  Core::MutexSptd::scope lock(m_mutex);
  m_brokers = std::move(_brokers);
}

void Brokers::set(const BrokersEndPoints& _brokers) {
  Core::MutexSptd::scope lock(m_mutex);
  m_brokers = _brokers;
}

bool Brokers::put(const Comm::client::ConnQueue::ReqBase::Ptr& req,
                  Brokers::BrokerIdx& idx) {
  for(Comm::EndPoints endpoints;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(!m_brokers.empty()) {
        endpoints = m_brokers[idx.pos >= m_brokers.size()
          ? (idx.pos = 0)
          : idx.pos];
      }
    }
    if(!endpoints.empty()) {
      queues->get(endpoints)->put(req);
      return true;
    }
    if(!req->valid()) {
      req->handle_no_conn();
      return false;
    }
    SWC_LOG(LOG_ERROR,
      "Broker hosts cfg 'swc.bkr.host' is empty, waiting!");
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

bool Brokers::BrokerIdx::turn_around(Brokers& brks) noexcept {
  if(++pos < brks.size())
    return false;
  pos = 0;
  return true;
}



}} //namespace SWC::client

