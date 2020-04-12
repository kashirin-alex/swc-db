/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/client/mngr/Groups.h"
#include "swcdb/db/client/Settings.h"


namespace SWC { namespace client { namespace Mngr {


Group::Group(size_t cbegin, size_t cend, const EndPoints& endpoints)
            : col_begin(cbegin), col_end(cend){
  m_hosts.push_back(endpoints);
}

Group::Group(size_t cbegin, size_t cend, Hosts hosts) 
            : col_begin(cbegin), col_end(cend){
  m_hosts.swap(hosts);
}
  
Group::~Group() { }
  
Group::Ptr Group::copy(){
  return std::make_shared<Group>(col_begin, col_end, get_hosts());
}

void Group::add_host(EndPoints& new_endpoints) {
  std::lock_guard lock(m_mutex);

  EndPoints* found_host;
  for(auto& endpoint : new_endpoints){
    get_host(endpoint, found_host);
    if(found_host != nullptr){
      std::lock_guard lock(m_mutex);
      (*found_host).swap(new_endpoints);
      return;
    }
  }
  m_hosts.push_back(new_endpoints);
}

Hosts Group::get_hosts() {
  Hosts hosts;
  std::lock_guard lock(m_mutex);
  for(auto& endpoints : m_hosts){
    EndPoints host;
    for(auto& endpoint : endpoints){
      host.push_back(endpoint);
    }
    hosts.push_back(host);
  }
  return hosts;
}

bool Group::is_in_group(const EndPoint& endpoint) {
  std::lock_guard lock(m_mutex);

  EndPoints* found_host;
  get_host(endpoint, found_host);
  return found_host != nullptr;
}

std::string Group::to_string() {
  std::string s;
  std::lock_guard lock(m_mutex);

  s.append("group:\n");
  s.append(" column=");
  s.append(std::to_string(col_begin));
  s.append("-");
  s.append(std::to_string(col_end));
  s.append("\n");
    
  for(auto& endpoints : m_hosts){
    s.append(" host=\n");
    for(auto& endpoint : endpoints){
      s.append("  [");
      s.append(endpoint.address().to_string());
      s.append("]");
      s.append(":");
      s.append(std::to_string(endpoint.port()));
      s.append("\n");
    }
  }
  return s;
}

void Group::apply_endpoints(EndPoints& to_endpoints) {
  std::lock_guard lock(m_mutex);
    
  for(auto& endpoints : m_hosts){
    for(auto& endpoint : endpoints){      
      auto it = std::find_if(
        to_endpoints.begin(), to_endpoints.end(), 
        [endpoint](const EndPoint& e2){
          return endpoint.address() == e2.address() 
                 && endpoint.port() == e2.port();});

      if(it == to_endpoints.end())
        to_endpoints.push_back(endpoint);
    }
  }
}

void Group::get_host(const EndPoint& point, EndPoints*& found_host) {
  for(auto& endpoints : m_hosts){
    auto it = std::find_if(
      endpoints.begin(), endpoints.end(), 
      [point](const EndPoint& e2){
        return point.address() == e2.address() 
               && point.port() == e2.port();});

    if(it != endpoints.end()){
      found_host = &endpoints;
      return;
    }
  }
  found_host = nullptr;
  return;
}


Groups::Groups() { }

Groups::Groups(Selected groups) {
  m_groups.swap(groups);
}

Groups::~Groups() { }

Groups::Ptr Groups::init() {
  Env::Config::settings()->get<Property::V_GSTRINGS>("swc.mngr.host")
    ->set_cb_on_chg([cb=shared_from_this()]{cb->on_cfg_update();});
    
  on_cfg_update();
  return shared_from_this();
}

Groups::operator Groups::Ptr(){
  return shared_from_this();
}

Groups::Ptr Groups::copy() {
  Selected groups;
  std::lock_guard lock(m_mutex);
  for(auto& group : m_groups)
    groups.push_back(group->copy());
  return std::make_shared<Groups>(groups);
}

void Groups::on_cfg_update() {
  SWC_LOG(LOG_DEBUG, "update_cfg()");

  Property::V_GSTRINGS::Ptr cfg_mngr_hosts
    = Env::Config::settings()->get<Property::V_GSTRINGS>("swc.mngr.host");
  uint16_t default_port = Env::Config::settings()->get_i16("swc.mngr.port");
  uint16_t port;

  int c = cfg_mngr_hosts->size();
  for(int n=0; n<c; ++n){
    std::string cfg = cfg_mngr_hosts->get_item(n);
    SWC_LOGF(LOG_DEBUG, "cfg=%d swc.mngr.host=%s", n, cfg.c_str());
      
    auto at = cfg.find_first_of("|");
    if(at == std::string::npos) {
      add_host(0, 0, default_port, cfg);
      continue;
    }
      
    auto cols = cfg.substr(0, at);
    auto col_at = cols.find_first_of("[");
    if(col_at == std::string::npos) {
      Property::from_string(cfg.substr(at+1), &port);
      add_host(0, 0, port, cfg.substr(0, at));
      continue;
    }

    cols = cols.substr(col_at+1, cols.find_first_of("]")-1);
          
    auto host_and_ip = cfg.substr(at+1);
    std::string host_or_ips;
    auto addr_at = host_and_ip.find_first_of("|");
    if(addr_at == std::string::npos) {
      host_or_ips = host_and_ip;
      port = default_port;
    } else {
      host_or_ips = host_and_ip.substr(0, addr_at);
      Property::from_string(host_and_ip.substr(addr_at+1), &port);
    }

    int64_t col_begin, col_end;
    do {
      auto col_range = cols;
      col_at = cols.find_first_of(",");
      if(col_at != std::string::npos){
        col_range = cols.substr(0, col_at);
        cols = cols.substr(col_at+1);
      }
          
      auto col_range_at = col_range.substr(0, col_at).find_first_of("-");
      auto b = col_range.substr(0, col_range_at);
      auto e = col_range.substr(col_range_at+1);
      col_begin = 0;
      col_end = 0;
      if(!b.empty())
        Property::from_string(b, &col_begin);
      if(!e.empty())
        Property::from_string(e, &col_end);

      add_host(col_begin, col_end, port, host_or_ips);

    } while (col_at != std::string::npos);

  }

  SWC_LOG(LOG_DEBUG, to_string().c_str());
}

void Groups::add_host(size_t col_begin, size_t col_end, 
                      uint16_t port, std::string host_or_ips) {
  std::vector<std::string> ips;
  std::string host;
  size_t at;
  do {
    auto addr = host_or_ips;
    at = host_or_ips.find_first_of(",");
    if(at != std::string::npos){
      addr = host_or_ips.substr(0, at);
      host_or_ips = host_or_ips.substr(at+1, host_or_ips.length());
    }
    if(Resolver::is_ipv4_address(addr) || Resolver::is_ipv6_address(addr))
      ips.push_back(addr);
    else
      host = addr;
  
  } while(at != std::string::npos);

  EndPoints endpoints = Resolver::get_endpoints(port, ips, host);

  if(endpoints.empty())
    return;
  {
    std::lock_guard lock(m_mutex);
    for(auto& group : m_groups){
      if(group->col_begin == col_begin && group->col_end == col_end){
        group->add_host(endpoints);
        return;
      }
    }
    m_groups.push_back(
      std::make_shared<Group>(col_begin, col_end, endpoints));
  }
}

Groups::Selected Groups::get_groups() {
  Selected groups;
  std::lock_guard lock(m_mutex);
  for(auto& group : m_groups)
    groups.push_back(group);
  return groups;
}

void Groups::hosts(size_t cid, Hosts& hosts, Groups::GroupHost &group_host) {
  std::lock_guard lock(m_mutex);

  for(auto& group : m_groups) {
    if(group->col_begin <= cid 
      && (!group->col_end || group->col_end >= cid)) {
        hosts = group->get_hosts();
        group_host.col_begin = group->col_begin;
        group_host.col_end = group->col_end;
        break;
      }
  }
}

Groups::Selected Groups::get_groups(const EndPoints& endpoints) {
  Selected host_groups;
  std::lock_guard lock(m_mutex);
    
  for(auto& group : m_groups){
    for(auto& endpoint : endpoints){
      if(group->is_in_group(endpoint) 
        && std::find_if(host_groups.begin(), host_groups.end(), 
            [group](const Group::Ptr & g){return g == group;})
           == host_groups.end()
        )
        host_groups.push_back(group);
    }
  }
  return host_groups;
}

EndPoints Groups::get_endpoints(size_t col_begin, size_t col_end) {
  EndPoints endpoints;
  if(!col_end)
    col_end = col_begin;
  std::lock_guard lock(m_mutex);
    
  for(auto& group : m_groups){
    if(group->col_begin <= col_begin
      && (!group->col_end || (col_end && group->col_end >= col_end))) {
      group->apply_endpoints(endpoints);
    }
  }
  return endpoints;
}

std::string Groups::to_string() {
  std::string s("manager-groups:\n");
  std::lock_guard lock(m_mutex);

  for(auto& group : m_groups)
    s.append(group->to_string());
  return s;
}

void Groups::add(Groups::GroupHost& g_host) {
  std::lock_guard lock(m_mutex);

  for(auto it=m_active_g_host.begin(); it<m_active_g_host.end(); ++it) {
    if(has_endpoint(g_host.endpoints, it->endpoints))
      return;
    if(g_host.col_begin == it->col_begin && g_host.col_end == it->col_end){
      it->endpoints = g_host.endpoints;
      return;
    }
  }
  m_active_g_host.push_back(g_host);
}

void Groups::remove(EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);

  for(auto it=m_active_g_host.begin(); it<m_active_g_host.end(); ++it){
    if(has_endpoint(endpoints, it->endpoints)){
      m_active_g_host.erase(it);
    }
  }
}

void Groups::select(int64_t cid, EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);
    
  for(auto& host : m_active_g_host) {
    if(host.col_begin <= cid && (!host.col_end || host.col_end >= cid)) {
      endpoints = host.endpoints;
      return;
    }
  }
}


}}}
