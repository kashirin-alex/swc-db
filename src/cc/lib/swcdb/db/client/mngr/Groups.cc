/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/mngr/Groups.h"
#include "swcdb/db/client/Settings.h"


namespace SWC { namespace client { namespace Mngr {


Group::Group(uint8_t role, cid_t cid_begin, cid_t cid_end, 
             const EndPoints& endpoints)
            : Hosts({endpoints}), 
              role(role), cid_begin(cid_begin), cid_end(cid_end) {
}

Group::Group(uint8_t role, cid_t cid_begin, cid_t cid_end, 
             const Hosts& hosts) 
            : Hosts(hosts), 
              role(role), cid_begin(cid_begin), cid_end(cid_end) {
}
  
Group::~Group() { }
  
Group::Ptr Group::copy() {
  return std::make_shared<Group>(role, cid_begin, cid_end, get_hosts());
}

void Group::add_host(EndPoints& new_endpoints) {
  std::lock_guard lock(m_mutex);

  EndPoints* found_host = nullptr;
  for(auto& endpoint : new_endpoints) {
    _get_host(endpoint, found_host);
    if(found_host)
      return found_host->swap(new_endpoints);
  }
  push_back(new_endpoints);
}

Hosts Group::get_hosts() {
  std::lock_guard lock(m_mutex);
  return Hosts(begin(), end());
}

bool Group::is_in_group(const EndPoint& endpoint) {
  std::lock_guard lock(m_mutex);

  EndPoints* found_host;
  _get_host(endpoint, found_host);
  return found_host != nullptr;
}

void Group::print(std::ostream& out) {
  std::lock_guard lock(m_mutex);

  out << "group:\n"
      << " role=" << Types::MngrRole::to_string(role)
      << " column=" << cid_begin << '-' << cid_end << '\n';
  for(auto& endpoints : *this) {
    out << " host=\n";
    for(auto& endpoint : endpoints)
      out << "  " << endpoint << '\n';
  }
}

void Group::apply_endpoints(EndPoints& to_endpoints) {
  std::lock_guard lock(m_mutex);
    
  for(auto& endpoints : *this) {
    for(auto& endpoint : endpoints) {      
      auto it = std::find_if(
        to_endpoints.begin(), to_endpoints.end(), 
        [endpoint](const EndPoint& e2) {
          return endpoint.address() == e2.address() 
                 && endpoint.port() == e2.port();});

      if(it == to_endpoints.end())
        to_endpoints.push_back(endpoint);
    }
  }
}

void Group::_get_host(const EndPoint& point, EndPoints*& found_host) {
  for(auto& endpoints : *this) {
    auto it = std::find_if(
      endpoints.begin(), endpoints.end(), 
      [point](const EndPoint& e2) {
        return point.address() == e2.address() 
               && point.port() == e2.port();});

    if(it != endpoints.end()) {
      found_host = &endpoints;
      return;
    }
  }
  found_host = nullptr;
}


Groups::Groups() { 
  asio::error_code ec;
  Resolver::get_networks(
    Env::Config::settings()->get_strs("swc.comm.network.priority"),
    m_nets, ec
  );
  if(ec)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "swc.comm.network.priority error(%s)",
              ec.message().c_str());
}


Groups::Groups(const Groups::Vec& groups, const std::vector<Network>& nets)
               : Vec(groups), m_nets(nets) {
}

Groups::~Groups() { }

Groups::Ptr Groups::init() {
  Env::Config::settings()->get<Config::Property::V_GSTRINGS>("swc.mngr.host")
    ->set_cb_on_chg([cb=shared_from_this()]{cb->on_cfg_update();});
    
  on_cfg_update();
  return shared_from_this();
}

Groups::Ptr Groups::copy() {
  Vec groups;
  std::lock_guard lock(m_mutex);
  for(auto& group : *this)
    groups.push_back(group->copy());
  return std::make_shared<Groups>(groups, m_nets);
}

void Groups::on_cfg_update() {
  SWC_LOG(LOG_DEBUG, "update_cfg()");

  Config::Property::V_GSTRINGS::Ptr cfg_mngr_hosts
    = Env::Config::settings()->get<Config::Property::V_GSTRINGS>(
      "swc.mngr.host");
  uint16_t default_port = Env::Config::settings()->get_i16("swc.mngr.port");
  
  uint8_t role;
  int64_t col_begin;
  int64_t col_end;
  std::string host_or_ips;
  uint16_t port;

  size_t at;
  size_t at_chk;
  size_t at_offset;

  int c = cfg_mngr_hosts->size();
  std::string cfg;
  std::string cfg_chk;
  
  m_mutex.lock();
  clear();
  for(int n=0; n<c; ++n) {
    cfg = cfg_mngr_hosts->get_item(n);
    SWC_LOGF(LOG_DEBUG, "cfg=%d swc.mngr.host=%s", n, cfg.c_str());
      
    if((at = cfg.find_first_of('|')) == std::string::npos) {
      _add_host(Types::MngrRole::ALL, 0, 0, default_port, cfg);
      continue;
    }

    role = Types::MngrRole::COLUMNS;
    cfg_chk = cfg.substr(at_offset = 0, at);
    if((at_chk = cfg_chk.find_first_of('{')) != std::string::npos) {
      cfg_chk = cfg_chk.substr(++at_chk, cfg_chk.find_first_of('}')-1);
      for(;;) {
        if(strncasecmp(cfg_chk.data(), "schemas", 7) == 0)
          role |= Types::MngrRole::SCHEMAS;
        else if(strncasecmp(cfg_chk.data(), "rangers", 7) == 0)
          role |= Types::MngrRole::RANGERS;
        if((at_chk = cfg_chk.find_first_of(',')) == std::string::npos)
          break;
        cfg_chk = cfg_chk.substr(++at_chk);
      }
      at = cfg.find_first_of('|', at_offset = ++at);
    }


    col_begin = 0;
    col_end = 0;
    cfg_chk = cfg.substr(at_offset, at-at_offset);
    if((at_chk = cfg_chk.find_first_of('[')) != std::string::npos) {
      cfg_chk = cfg_chk.substr(++at_chk, cfg_chk.find_first_of(']')-1);
      auto col_range_at = cfg_chk.find_first_of('-');
      auto b = cfg_chk.substr(0, col_range_at);
      auto e = cfg_chk.substr(col_range_at+1);
      if(!b.empty())
        Config::Property::from_string(b, &col_begin);
      if(!e.empty())
        Config::Property::from_string(e, &col_end);

      at = cfg.find_first_of('|', at_offset = ++at);
    } else if(role != Types::MngrRole::COLUMNS) {
      role -= Types::MngrRole::COLUMNS;
      role |= Types::MngrRole::NO_COLUMNS;
    }

    if(role == Types::MngrRole::COLUMNS && !col_begin && !col_end)
      role = Types::MngrRole::ALL;

    cfg_chk = cfg.substr(at_offset);
    if((at_chk = cfg_chk.find_first_of('|')) == std::string::npos) {
      host_or_ips = cfg_chk;
      port = default_port;
    } else {
      host_or_ips = cfg_chk.substr(0, at_chk);
      Config::Property::from_string(cfg_chk.substr(++at_chk), &port);
    }

    _add_host(role, col_begin, col_end, port, host_or_ips);

  }
  m_mutex.unlock();

  SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM); );
}

void Groups::_add_host(uint8_t role, cid_t cid_begin, cid_t cid_end, 
                       uint16_t port, std::string host_or_ips) {
  std::vector<std::string> ips;
  std::string host;
  size_t at;
  do {
    auto addr = host_or_ips;
    at = host_or_ips.find_first_of(',');
    if(at != std::string::npos) {
      addr = host_or_ips.substr(0, at);
      host_or_ips = host_or_ips.substr(at+1, host_or_ips.length());
    }
    if(Resolver::is_ipv4_address(addr) || Resolver::is_ipv6_address(addr))
      ips.push_back(addr);
    else
      host = addr;
  
  } while(at != std::string::npos);

  EndPoints endpoints = Resolver::get_endpoints(port, ips, host, m_nets);

  if(endpoints.empty())
    return;
  for(auto& group : *this) {
    if(group->role == role && 
       group->cid_begin == cid_begin && 
       group->cid_end == cid_end)
      return group->add_host(endpoints); 
  }
  emplace_back(new Group(role, cid_begin, cid_end, endpoints));
}

Groups::Vec Groups::get_groups() {
  std::lock_guard lock(m_mutex);
  return Vec(begin(), end());
}

void Groups::hosts(uint8_t role, cid_t cid, Hosts& hosts, 
                   Groups::GroupHost &group_host) {
  std::lock_guard lock(m_mutex);

  for(auto& group : *this) {
    if(group->role & role && (
        !(role & Types::MngrRole::COLUMNS) ||
        (group->cid_begin <= cid && (!group->cid_end || group->cid_end >= cid))
      )) {
      hosts = group->get_hosts();
      group_host.role = group->role;
      group_host.cid_begin = group->cid_begin;
      group_host.cid_end = group->cid_end;
      break;
    }
  }
}

Groups::Vec Groups::get_groups(const EndPoints& endpoints) {
  Vec host_groups;
  std::lock_guard lock(m_mutex);
    
  for(auto& group : *this) {
    for(auto& endpoint : endpoints) {
      if(group->is_in_group(endpoint) 
        && std::find_if(host_groups.begin(), host_groups.end(), 
            [group](const Group::Ptr& g) {return g == group;})
           == host_groups.end()
        )
        host_groups.push_back(group);
    }
  }
  return host_groups;
}

EndPoints Groups::get_endpoints(uint8_t role, cid_t cid_begin, 
                                              cid_t cid_end) {
  EndPoints endpoints;
  if(!cid_end)
    cid_end = cid_begin;
  std::lock_guard lock(m_mutex);
    
  for(auto& group : *this) {
    if((!role || group->role & role) && 
       (!cid_begin || group->cid_begin <= cid_begin) && 
       (!cid_end || group->cid_end || 
        (cid_end && group->cid_end >= cid_end))) {
      group->apply_endpoints(endpoints);
    }
  }
  return endpoints;
}

void Groups::print(std::ostream& out) {
  out << "manager-groups:\n";
  std::lock_guard lock(m_mutex);

  for(auto& group : *this)
    group->print(out);
}

void Groups::add(Groups::GroupHost& g_host) {
  std::lock_guard lock(m_mutex);

  for(auto it=m_active_g_host.begin(); it<m_active_g_host.end(); ++it) {
    if(has_endpoint(g_host.endpoints, it->endpoints))
      return;
    if(g_host.role == it->role && 
       g_host.cid_begin == it->cid_begin && 
       g_host.cid_end == it->cid_end) {
      it->endpoints = g_host.endpoints;
      return;
    }
  }
  m_active_g_host.push_back(g_host);
}

void Groups::remove(const EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);

  for(auto it=m_active_g_host.begin(); it<m_active_g_host.end(); ++it) {
    if(has_endpoint(endpoints, it->endpoints))
      m_active_g_host.erase(it);
  }
}

void Groups::select(const cid_t& cid, EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);
    
  for(auto& host : m_active_g_host) {
    if(host.role & Types::MngrRole::COLUMNS && 
       host.cid_begin <= cid && 
       (!host.cid_end || host.cid_end >= cid)) {
      endpoints = host.endpoints;
      return;
    }
  }
}

void Groups::select(const uint8_t& role, EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);
    
  for(auto& host : m_active_g_host) {
    if(host.role & role) {
      endpoints = host.endpoints;
      return;
    }
  }
}


}}}
