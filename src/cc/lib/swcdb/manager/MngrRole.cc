/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/manager/MngrRole.h"


namespace SWC { namespace Manager {


MngrRole::MngrRole(const EndPoints& endpoints)
    : m_local_endpoints(endpoints),
      m_local_token(endpoints_hash(m_local_endpoints)),
      m_check_timer(asio::high_resolution_timer(*Env::IoCtx::io()->ptr())),
      cfg_conn_probes(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.role.connection.probes")),
      cfg_conn_timeout(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.role.connection.timeout")),
      cfg_conn_fb_failures(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.role.connection.fallback.failures")),
      cfg_req_timeout(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.role.request.timeout")),
      cfg_check_interval(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.role.check.interval")),
      cfg_delay_updated(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.role.check.delay.updated")),
      cfg_delay_fallback(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.role.check.delay.fallback")),
      m_checkin(false),
      m_mngr_inchain(
        std::make_shared<client::ConnQueue>(Env::IoCtx::io()->shared())) {
  schedule_checkin(3000);
}

MngrRole::~MngrRole() { }


void MngrRole::schedule_checkin(uint32_t t_ms) {
  std::lock_guard lock(m_mutex_timer);
  if(!m_run)
    return;

  auto set_in = std::chrono::milliseconds(t_ms);
  auto set_on = m_check_timer.expires_from_now();
  if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
    return;

  m_check_timer.cancel();
  m_check_timer.expires_from_now(set_in);

  m_check_timer.async_wait(
    [this](const asio::error_code ec) {
      if(ec != asio::error::operation_aborted) {
        managers_checkin();
      }
  }); 
  SWC_LOGF(LOG_DEBUG, "MngrRole managers_checkin scheduled in ms=%d", t_ms);
}

bool MngrRole::is_active(size_t cid) {
  auto host = active_mngr(cid, cid);
  return host != nullptr && has_endpoint(host->endpoints, m_local_endpoints);
}

MngrStatus::Ptr MngrRole::active_mngr(size_t begin, size_t end) {
  std::shared_lock lock(m_mutex);
  for(auto& host : m_states) {
    if(host->state == Types::MngrState::ACTIVE 
      && host->col_begin <= begin 
      && (!host->col_end || host->col_end >= end)) {
      return host;
    }
  }
  return nullptr;
}

void MngrRole::req_mngr_inchain(client::ConnQueue::ReqBase::Ptr req) {
  m_mngr_inchain->put(req);
}

void MngrRole::fill_states(MngrsStatus states, uint64_t token, 
                           ResponseCallback::Ptr cb) {
  bool new_recs = false;
  bool turn_around = token == m_local_token;

  for(auto& host : states) {

    bool local = has_endpoint(host->endpoints, m_local_endpoints);

    if(local && !token
       && (int)host->state < (int)Types::MngrState::STANDBY) {
      update_state(host->endpoints, Types::MngrState::STANDBY);
      continue;
    }
      
    MngrStatus::Ptr host_set = get_host(host->endpoints);
      
    if(host_set->state == Types::MngrState::OFF 
       && host->state > Types::MngrState::OFF) {
      //m_major_updates = true;
      schedule_checkin(500);
    }

    if((int)host->state < (int)Types::MngrState::STANDBY) {
      if(host->state != host_set->state) {
        update_state(host->endpoints, 
          host->state != Types::MngrState::NOTSET?
          host->state : host_set->state);  
        new_recs = true;
      }
      continue;
    }

    MngrStatus::Ptr high_set = 
      get_highest_state_host(host->col_begin, host->col_end);

    if(high_set->state == Types::MngrState::ACTIVE) {
      if(host->state != host_set->state
         && host_set->priority != high_set->priority) {
        update_state(host->endpoints, Types::MngrState::STANDBY);
        new_recs = true;
      }
      continue;
    }
    if(host->state == Types::MngrState::ACTIVE) {
      update_state(host->endpoints, host->state);
      new_recs = true;
      // m_major_updates = true;
      continue;
    }
      
    
    if(host->priority > high_set->priority) {
      if((int)host->state > (int)Types::MngrState::STANDBY) {
        update_state(host->endpoints, (Types::MngrState)(host->state-1));
        new_recs = true;
      }
    } else {
      if((int)host->state < (int)Types::MngrState::ACTIVE) {
        update_state(host->endpoints, (Types::MngrState)(host->state+1));
        new_recs = true;
      }
    }
  }
    
  for(auto& host : states) {
    if(!is_off(host->col_begin, host->col_end))
      continue;
    auto hosts_pr_group = 
        Env::Clients::get()->mngrs_groups->get_endpoints(
        host->col_begin, host->col_end);
    for(auto& h : hosts_pr_group) {
      if(has_endpoint(h, host->endpoints) 
        || !has_endpoint(h, m_local_endpoints))
        continue;

      MngrStatus::Ptr l_host = get_host(m_local_endpoints);
      MngrStatus::Ptr l_hight = get_highest_state_host(
        l_host->col_begin, l_host->col_end);
      if(l_hight->state < Types::MngrState::WANT) {
        update_state(m_local_endpoints, Types::MngrState::WANT);
        new_recs = true;
      }
      break;
    }
  }

  {
    std::shared_lock lock(m_mutex);
    if(!token || !turn_around) {
      if(!token)
        token = m_local_token;

      req_mngr_inchain(std::make_shared<Protocol::Mngr::Req::MngrState>(
        cb, m_states, token, m_local_endpoints[0], 
        (cfg_conn_probes->get() * cfg_conn_timeout->get()
        + cfg_req_timeout->get()) * m_states.size()
        ));
      //  schedule_checkin(3000);
      return;
    }
  }
    
  if(cb != nullptr)
    cb->response_ok();

  schedule_checkin(
    new_recs ? cfg_delay_updated->get() : cfg_check_interval->get());
    
  SWC_LOGF(LOG_DEBUG, "%s", to_string().c_str());
  set_active_columns();
}

void MngrRole::update_manager_addr(uint64_t hash, const EndPoint& mngr_host) {
  std::scoped_lock lock(m_mutex);

  bool new_srv = m_mngrs_client_srv.emplace(hash, mngr_host).second;
  if(new_srv) {
    //m_major_updates = true;
    schedule_checkin(500);
  }
}
  
bool MngrRole::disconnection(const EndPoint& endpoint_server, 
                             const EndPoint& endpoint_client, 
                             bool srv) {
  EndPoints endpoints;
  {
    std::scoped_lock lock(m_mutex);
    
    auto it = m_mngrs_client_srv.find(endpoint_hash(endpoint_server));
    if(it != m_mngrs_client_srv.end()) {
      endpoints.push_back(it->second);
      m_mngrs_client_srv.erase(it);
    } else 
      endpoints.push_back(endpoint_server);
  }
  MngrStatus::Ptr host_set = get_host(endpoints);
  if(host_set == nullptr)
    return false;

  schedule_checkin(
    host_set->state == Types::MngrState::ACTIVE ? 
    cfg_delay_fallback->get() : cfg_check_interval->get());

  SWC_LOGF(LOG_DEBUG, "disconnection, srv=%d, server=[%s]:%d, client=[%s]:%d", 
            (int)srv,
            endpoint_server.address().to_string().c_str(), 
            endpoint_server.port(), 
            endpoint_client.address().to_string().c_str(), 
            endpoint_client.port());
  if(host_set->state != Types::MngrState::ACTIVE)
    update_state(endpoint_server, Types::MngrState::OFF);
  // m_major_updates = true;
  return true;
}
  
bool MngrRole::require_sync() {
  std::scoped_lock lock(m_mutex);
  bool current = m_major_updates;
  m_major_updates = false;
  return current;
}

void MngrRole::stop() {
  {
    std::lock_guard lock(m_mutex_timer);
    m_check_timer.cancel();
    m_run = false;
  }

  m_mngr_inchain->stop();

  {
    std::shared_lock lock(m_mutex);
    for(auto& host : m_states) {
      if(host->conn != nullptr && host->conn->is_open())
        asio::post(
          *Env::IoCtx::io()->ptr(), [conn=host->conn]() {conn->do_close();});
    }
  }
}

std::string MngrRole::to_string() {
  std::string s("Mngrs Role:");
    
  for(auto& h : m_states) {
    s.append("\n ");
    s.append(h->to_string());
  }
  s.append("\nMngrInchain ");
  s.append(m_mngr_inchain->to_string());

  s.append("\nLocal-Endpoints: ");
  for(auto& endpoint : m_local_endpoints) {
    s.append(" [");
    s.append(endpoint.address().to_string());
    s.append("]:");
    s.append(std::to_string(endpoint.port()));
    s.append(",");
  }
  return s;
}
 
void MngrRole::apply_cfg() {
  client::Mngr::Groups::Selected groups = 
    Env::Clients::get()->mngrs_groups->get_groups();
    
  for(auto& g : groups) {
    // SWC_LOG(LOG_DEBUG,  g->to_string().c_str());
    uint32_t pr = 0;
    for(auto& endpoints : g->get_hosts()) {

      bool found = false;
      for(auto& host : m_states) {
        found = has_endpoint(endpoints, host->endpoints);
        if(found)break;
      }
      if(found)continue;

      m_states.emplace_back(
        new MngrStatus(g->col_begin, g->col_end, endpoints, nullptr, ++pr));
    }
  }
    
  m_local_groups = Env::Clients::get()->mngrs_groups->get_groups(
    m_local_endpoints);
}

void MngrRole::managers_checkin() {
  if(m_checkin)
    return;
  m_checkin = true;

  //SWC_LOG(LOG_DEBUG, "managers_checkin");
  size_t sz;
  {
    std::scoped_lock lock(m_mutex);
    apply_cfg();
    sz = m_states.size();
  }
  managers_checker(0, sz, false);
}

void MngrRole::fill_states() {
  MngrsStatus states;
  {
    std::shared_lock lock(m_mutex);
    states.assign(m_states.begin(), m_states.end());
  }
  fill_states(states, 0, nullptr);
}

void MngrRole::managers_checker(int next, size_t total, bool flw) {
    // set manager followed(in-chain) local manager, incl. last's is first
  if(!total) {
    m_checkin=false;
    schedule_checkin(cfg_check_interval->get());
    return;
  }

  MngrStatus::Ptr host_chk;
  {
    std::shared_lock lock(m_mutex);
    if(!m_run)
      return;
    if(next == m_states.size())
      next = 0;
    host_chk = m_states.at(next);
    ++next;
  }

  if(has_endpoint(host_chk->endpoints, m_local_endpoints) && total >= 1) {
    if(flw) {
      m_checkin=false;
      schedule_checkin(cfg_check_interval->get());
      return;
    }
    flw = true;
    if(total > 1) {
      managers_checker(next, total, flw);
      return;
    }
  }
  if(!flw) {
    managers_checker(next, total, flw);
    return;
  }
        
  if(host_chk->conn != nullptr && host_chk->conn->is_open()) {
    set_mngr_inchain(host_chk->conn);
    return;
  }

  Env::Clients::get()->mngr_service->get_connection(
    host_chk->endpoints, 
    [this, host_chk, next, total, flw]
    (ConnHandlerPtr conn) {
      manager_checker(host_chk, next, total, flw, conn);
    },
    std::chrono::milliseconds(cfg_conn_timeout->get()), 
    cfg_conn_probes->get()
  );
}

void MngrRole::manager_checker(MngrStatus::Ptr host, int next, size_t total, 
                               bool flw, ConnHandlerPtr conn) {
  if(conn == nullptr || !conn->is_open()) {
    if(host->state == Types::MngrState::ACTIVE
       && ++host->failures <= cfg_conn_fb_failures->get()) {   
      m_checkin=false;
      schedule_checkin(cfg_delay_fallback->get());
      SWC_LOGF(LOG_DEBUG, "Allowed conn Failure=%d before fallback", 
                host->failures);
      return;
    }
    host->state = Types::MngrState::OFF;
    managers_checker(next, total-1, flw);
    return;
  }
  host->conn = conn;
  host->failures = 0;
  //conn->accept_requests();
  m_major_updates = true;
  set_mngr_inchain(host->conn);
}
  
void MngrRole::update_state(EndPoint endpoint, Types::MngrState state) {
  std::scoped_lock lock(m_mutex);

  for(auto& host : m_states) {
    if(has_endpoint(endpoint, host->endpoints)) {
      host->state = state;
    }
  }
}

void MngrRole::update_state(const EndPoints& endpoints, Types::MngrState state) {
  std::scoped_lock lock(m_mutex);

  for(auto& host : m_states) {
    if(has_endpoint(endpoints, host->endpoints)) {
      host->state = state;
    }
  }
}
  
MngrStatus::Ptr MngrRole::get_host(const EndPoints& endpoints) {
  std::shared_lock lock(m_mutex);

  for(auto& host : m_states) {
    if(has_endpoint(endpoints, host->endpoints))
      return host;
  }
  return nullptr;
}

MngrStatus::Ptr MngrRole::get_highest_state_host(uint64_t begin, uint64_t end) {
  std::shared_lock lock(m_mutex);

  MngrStatus::Ptr h = nullptr;
  for(auto& host : m_states) {
    if(host->col_begin == begin && host->col_end == end 
      && (h == nullptr || h->state < host->state)) {
      h = host;
    }
  }
  return h;
}
  
bool MngrRole::is_off(uint64_t begin, uint64_t end) {
  std::shared_lock lock(m_mutex);

  bool offline = true;
  for(auto& host : m_states) {
    if(host->col_begin == begin 
      && host->col_end == end && host->state != Types::MngrState::OFF)
      offline = false;
  }
  return offline;
}

void MngrRole::set_active_columns() {
  client::Mngr::Groups::Selected groups;
  {
    std::shared_lock lock(m_mutex);
    groups = m_local_groups;
  }

  std::vector<int64_t> active;
  for(auto& group : groups) {
    int64_t cid =   !group->col_begin ?  1  : group->col_begin;
    int64_t cid_end = !group->col_end ? cid : group->col_end;

    if(!group->col_end && is_active(cid))
      active.push_back(group->col_end);
      
    for(;cid <= cid_end; ++cid) { 
      auto c_it = std::find_if(active.begin(), active.end(),  
                              [cid](const int64_t& cid_set) 
                              {return cid_set == cid;});
      if(c_it == active.end() && is_active(cid))
        active.push_back(cid);
    }
  }
  
  Env::Mngr::mngd_columns()->active(active);
}
  
void MngrRole::set_mngr_inchain(ConnHandlerPtr mngr) {
  m_mngr_inchain->set(mngr);

  fill_states();
  m_checkin=false;
  schedule_checkin(cfg_check_interval->get());
}


} // namespace Manager


}


#include "swcdb/manager/AppContextClient.cc"
#include "swcdb/manager/Protocol/Mngr/req/MngrState.cc"
