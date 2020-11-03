/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/manager/MngrRole.h"


namespace SWC { namespace Manager {


MngrRole::MngrRole(const Comm::IoContext::Ptr& app_io,
                   const Comm::EndPoints& endpoints)
    : m_local_endpoints(endpoints),
      m_local_token(Comm::endpoints_hash(m_local_endpoints)),
      m_checkin(false), m_local_active_role(DB::Types::MngrRole::NONE),
      m_check_timer(asio::high_resolution_timer(*app_io->ptr())),
      m_mngr_inchain(
        std::make_shared<Comm::client::ConnQueue>(app_io->shared())),
      cfg_conn_probes(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.role.connection.probes")),
      cfg_conn_timeout(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.role.connection.timeout")),
      cfg_conn_fb_failures(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.role.connection.fallback.failures")),
      cfg_req_timeout(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.role.request.timeout")),
      cfg_delay_updated(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.role.check.delay.updated")),
      cfg_check_interval(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.role.check.interval")),
      cfg_delay_fallback(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.role.check.delay.fallback")) {
  schedule_checkin(3000);
}

MngrRole::~MngrRole() { }


void MngrRole::schedule_checkin(uint32_t t_ms) {
  Core::MutexAtomic::scope lock(m_mutex_timer);
  if(!m_run)
    return;

  auto set_in = std::chrono::milliseconds(t_ms);
  auto set_on = m_check_timer.expiry();
  auto now = asio::high_resolution_timer::clock_type::now();
  if(set_on > now && set_on < now + set_in)
    return;

  m_check_timer.expires_after(set_in);
  m_check_timer.async_wait(
    [this](const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted) {
        managers_checkin();
      }
  }); 
  SWC_LOGF(LOG_DEBUG, "MngrRole managers_checkin scheduled in ms=%d", t_ms);
}

bool MngrRole::is_active(cid_t cid) {
  auto host = active_mngr(cid);
  return host && Comm::has_endpoint(host->endpoints, m_local_endpoints);
}

bool MngrRole::is_active_role(uint8_t role) {
  return m_local_active_role.load() & role;
}

MngrStatus::Ptr MngrRole::active_mngr(cid_t cid) {
  std::shared_lock lock(m_mutex);
  for(auto& host : m_states) {
    if(host->state == DB::Types::MngrState::ACTIVE &&
       host->role & DB::Types::MngrRole::COLUMNS &&
       host->cid_begin <= cid && 
       (!host->cid_end || host->cid_end >= cid))
      return host;
  }
  return nullptr;
}

MngrStatus::Ptr MngrRole::active_mngr_role(uint8_t role) {
  std::shared_lock lock(m_mutex);
  for(auto& host : m_states) {
    if(host->state == DB::Types::MngrState::ACTIVE && 
       host->role & role)
      return host;
  }
  return nullptr;
}

void MngrRole::get_states(MngrsStatus& states) {
  std::shared_lock lock(m_mutex);
  states.assign(m_states.begin(), m_states.end());
}

Comm::EndPoint MngrRole::get_inchain_endpoint() const {
  return m_mngr_inchain->get_endpoint_remote();
}

void MngrRole::req_mngr_inchain(const Comm::client::ConnQueue::ReqBase::Ptr& req) {
  m_mngr_inchain->put(req);
}

void MngrRole::fill_states(const MngrsStatus& states, uint64_t token, 
                           const Comm::ResponseCallback::Ptr& cb) {
  bool new_recs = false;
  bool turn_around = token == m_local_token;

  for(auto& host : states) {

    bool local = Comm::has_endpoint(host->endpoints, m_local_endpoints);

    if(local && !token && 
       (uint8_t)host->state.load() < (uint8_t)DB::Types::MngrState::STANDBY) {
      update_state(host->endpoints, DB::Types::MngrState::STANDBY);
      continue;
    }
      
    MngrStatus::Ptr host_set = get_host(host->endpoints);
    if(!host_set)
      continue;

    if(host_set->state == DB::Types::MngrState::OFF 
       && host->state > DB::Types::MngrState::OFF) {
      //m_major_updates = true;
      schedule_checkin(500);
    }

    if((uint8_t)host->state.load() < (uint8_t)DB::Types::MngrState::STANDBY) {
      if(host->state != host_set->state) {
        update_state(host->endpoints, 
          host->state != DB::Types::MngrState::NOTSET?
          host->state : host_set->state);  
        new_recs = true;
      }
      continue;
    }

    MngrStatus::Ptr high_set = get_highest_state_host(host);
    if(!high_set)
      continue;

    if(high_set->state == DB::Types::MngrState::ACTIVE) {
      if(host->state != host_set->state
         && host_set->priority != high_set->priority) {
        update_state(host->endpoints, DB::Types::MngrState::STANDBY);
        new_recs = true;
      }
      continue;
    }
    if(host->state == DB::Types::MngrState::ACTIVE) {
      update_state(host->endpoints, host->state);
      new_recs = true;
      // m_major_updates = true;
      continue;
    }
      
    
    if(host->priority > high_set->priority) {
      if((uint8_t)host->state.load() 
          > (uint8_t)DB::Types::MngrState::STANDBY) {
        update_state(
          host->endpoints, 
          (DB::Types::MngrState)((uint8_t)host->state.load() - 1)
        );
        new_recs = true;
      }
    } else {
      if((uint8_t)host->state.load() 
          < (uint8_t)DB::Types::MngrState::ACTIVE) {
        update_state(
          host->endpoints, 
          (DB::Types::MngrState)((uint8_t)host->state.load() + 1)
        );
        new_recs = true;
      }
    }
  }
    
  for(auto& host : states) {
    if(!is_group_off(host))
      continue;
    auto hosts_pr_group = 
        Env::Clients::get()->mngrs_groups->get_endpoints(
        host->role, host->cid_begin, host->cid_end);
    for(auto& h : hosts_pr_group) {
      if(Comm::has_endpoint(h, host->endpoints) 
        || !Comm::has_endpoint(h, m_local_endpoints))
        continue;

      MngrStatus::Ptr l_host = get_host(m_local_endpoints);
      MngrStatus::Ptr l_high = get_highest_state_host(l_host);
      if(!l_high || l_high->state < DB::Types::MngrState::WANT) {
        update_state(m_local_endpoints, DB::Types::MngrState::WANT);
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

      req_mngr_inchain(std::make_shared<Comm::Protocol::Mngr::Req::MngrState>(
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
    
  SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM); );
  apply_role_changes();
}

void MngrRole::update_manager_addr(uint64_t hash, 
                                   const Comm::EndPoint& mngr_host) {
  bool major_updates;
  {
    std::scoped_lock lock(m_mutex);

    bool new_srv = m_mngrs_client_srv.emplace(hash, mngr_host).second;
    if(new_srv) {
      //m_major_updates = true;
      schedule_checkin(500);
    }
    major_updates = m_major_updates;
    m_major_updates = false;
  }
  if(major_updates)
    Env::Mngr::mngd_columns()->require_sync();
}
  
void MngrRole::disconnection(const Comm::EndPoint& endpoint_server, 
                             const Comm::EndPoint& endpoint_client, 
                             bool srv) {
  Comm::EndPoints endpoints;
  {
    std::scoped_lock lock(m_mutex);
    
    auto it = m_mngrs_client_srv.find(Comm::endpoint_hash(endpoint_server));
    if(it != m_mngrs_client_srv.end()) {
      endpoints.push_back(it->second);
      m_mngrs_client_srv.erase(it);
    } else 
      endpoints.push_back(endpoint_server);
  }
  MngrStatus::Ptr host_set = get_host(endpoints);
  if(!host_set)
    return;

  schedule_checkin(
    host_set->state == DB::Types::MngrState::ACTIVE ? 
    cfg_delay_fallback->get() : cfg_check_interval->get());

  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_OSTREAM << "disconnection, srv=" << srv 
                    << " server=" << endpoint_server
                    << " client=" << endpoint_client;
  );
  if(host_set->state != DB::Types::MngrState::ACTIVE)
    update_state(endpoint_server, DB::Types::MngrState::OFF);
    // m_major_updates = true;
  return;
}

void MngrRole::stop() {
  Env::Mngr::rangers()->stop();
  Env::Mngr::mngd_columns()->stop();
  {
    Core::MutexAtomic::scope lock(m_mutex_timer);
    m_check_timer.cancel();
    m_run = false;
  }

  m_mngr_inchain->stop();

  {
    std::shared_lock lock(m_mutex);
    for(auto& host : m_states) {
      if(host->conn && host->conn->is_open())
        Env::Mngr::post([conn=host->conn]() {conn->do_close();});
    }
  }
}

void MngrRole::print(std::ostream& out) {
  out << "Mngrs Role:";
  for(auto& h : m_states)
    h->print(out << "\n ");

  m_mngr_inchain->print(out << "\nMngrInchain ");

  out << "\nLocal-Endpoints: [";
  for(auto& endpoint : m_local_endpoints)
    out << endpoint << ',';
  out << ']';
}
 
void MngrRole::_apply_cfg() {
  auto groups = Env::Clients::get()->mngrs_groups->get_groups();
  std::vector<Comm::EndPoint> tmp;
  for(auto& g : groups) {
    // SWC_LOG(LOG_DEBUG,  g->to_string().c_str());
    uint32_t pr = 0;
    for(auto& endpoints : g->get_hosts()) {
      tmp.insert(tmp.end(), endpoints.begin(), endpoints.end());
      ++pr;
      bool found = false;
      for(auto& host : m_states) {
        if((found = Comm::has_endpoint(endpoints, host->endpoints))) {
          host->priority = pr;
          break;
        }
      }
      if(found)
        continue;

      m_states.emplace_back(
        new MngrStatus(
          g->role, g->cid_begin, g->cid_end, endpoints, nullptr, pr));
    }
  }
  for(auto it=m_states.begin(); it<m_states.end(); ) {
    if(Comm::has_endpoint((*it)->endpoints, tmp))
      ++it;
    else
      m_states.erase(it);
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
  bool has_role;
  {
    std::scoped_lock lock(m_mutex);
    _apply_cfg();
    sz = m_states.size();
    has_role = !m_local_groups.empty();
  }
  if(has_role)
    return managers_checker(0, sz, false);

  std::shared_lock lock(m_mutex);
  std::string s;
  for(auto& endpoint : m_local_endpoints) {
    s.append(
      endpoint.address().to_string() + "|" + 
      std::to_string(endpoint.port())+ ",");
  }

  if(!m_mngrs_client_srv.empty()) {
    SWC_LOGF(LOG_DEBUG, "Manager(%s) decommissioned", s.c_str());
    std::raise(SIGINT);
    return;
  }

  SWC_LOGF(LOG_DEBUG, "Manager(%s) without role", s.c_str());
  m_checkin = false;
  return schedule_checkin(cfg_check_interval->get());
}

void MngrRole::fill_states() {
  MngrsStatus states;
  {
    std::shared_lock lock(m_mutex);
    states.assign(m_states.begin(), m_states.end());
  }
  fill_states(states, 0, nullptr);
}

void MngrRole::managers_checker(size_t next, size_t total, bool flw) {
    // set manager followed(in-chain) local manager, incl. last's is first
  if(!total) {
    m_checkin = false;
    return schedule_checkin(cfg_check_interval->get());
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

  if(Comm::has_endpoint(host_chk->endpoints, m_local_endpoints) && total >= 1) {
    if(flw) {
      m_checkin = false;
      return schedule_checkin(cfg_check_interval->get());
    }
    flw = true;
    if(total > 1)
      return managers_checker(next, total, flw);
  }

  if(!flw)
    return managers_checker(next, total, flw);
        
  if(host_chk->conn && host_chk->conn->is_open())
    return set_mngr_inchain(host_chk->conn);

  Env::Clients::get()->mngr->service->get_connection(
    host_chk->endpoints, 
    [this, host_chk, next, total, flw] (const Comm::ConnHandlerPtr& conn) {
      manager_checker(host_chk, next, total, flw, conn);
    },
    std::chrono::milliseconds(cfg_conn_timeout->get()), 
    cfg_conn_probes->get()
  );
}

void MngrRole::manager_checker(MngrStatus::Ptr host, 
                               size_t next, size_t total, bool flw, 
                               const Comm::ConnHandlerPtr& conn) {
  if(!conn || !conn->is_open()) {
    if(host->state == DB::Types::MngrState::ACTIVE
       && ++host->failures <= cfg_conn_fb_failures->get()) {   
      m_checkin = false;
      SWC_LOGF(LOG_DEBUG, "Allowed conn Failure=%d before fallback", 
                host->failures);
      return schedule_checkin(cfg_delay_fallback->get());
    }
    host->state = DB::Types::MngrState::OFF;
    return managers_checker(next, total-1, flw);
  }
  host->conn = conn;
  host->failures = 0;
  //conn->accept_requests();
  m_major_updates = true;
  set_mngr_inchain(host->conn);
}
  
void MngrRole::update_state(const Comm::EndPoint& endpoint, 
                            DB::Types::MngrState state) {
  std::scoped_lock lock(m_mutex);

  for(auto& host : m_states) {
    if(Comm::has_endpoint(endpoint, host->endpoints)) {
      host->state = state;
    }
  }
}

void MngrRole::update_state(const Comm::EndPoints& endpoints, 
                            DB::Types::MngrState state) {
  std::scoped_lock lock(m_mutex);

  for(auto& host : m_states) {
    if(Comm::has_endpoint(endpoints, host->endpoints)) {
      host->state = state;
    }
  }
}
  
MngrStatus::Ptr MngrRole::get_host(const Comm::EndPoints& endpoints) {
  std::shared_lock lock(m_mutex);

  for(auto& host : m_states) {
    if(Comm::has_endpoint(endpoints, host->endpoints))
      return host;
  }
  return nullptr;
}

MngrStatus::Ptr MngrRole::get_highest_state_host(const MngrStatus::Ptr& other) {
  std::shared_lock lock(m_mutex);

  MngrStatus::Ptr h = nullptr;
  for(auto& host : m_states) {
    if(host->eq_grouping(*other.get()) && 
       (!h || h->state < host->state))
      h = host;
  }
  return h;
}
  
bool MngrRole::is_group_off(const MngrStatus::Ptr& other) {
  std::shared_lock lock(m_mutex);

  bool offline = true;
  for(auto& host : m_states) {
    if(host->eq_grouping(*other.get()) && 
       host->state != DB::Types::MngrState::OFF)
      offline = false;
  }
  return offline;
}

void MngrRole::apply_role_changes() {
  MngrStatus::Ptr host_local;
  uint8_t role_old, role_new;
  bool has_cols = false;
  cid_t cid_begin = DB::Schema::NO_CID;
  cid_t cid_end = DB::Schema::NO_CID;
  {
    std::shared_lock lock(m_mutex);
    role_old = m_local_active_role;

    for(auto& host : m_states) {
      if(Comm::has_endpoint(host->endpoints, m_local_endpoints)) {
        host_local = host;
        break;
      }
    }
    if(host_local && host_local->state == DB::Types::MngrState::ACTIVE) {
      m_local_active_role = host_local->role;
      if((has_cols = m_local_active_role & DB::Types::MngrRole::COLUMNS)) {
        cid_begin = host_local->cid_begin;
        cid_end = host_local->cid_end;
      }
    } else {
      m_local_active_role = DB::Types::MngrRole::NONE;
    }
    role_new = m_local_active_role;
  }

  Env::Mngr::mngd_columns()->change_active(cid_begin, cid_end, has_cols);

  if(role_old == role_new)
    return;

  if(role_new & DB::Types::MngrRole::RANGERS) {
    if(!(role_old & DB::Types::MngrRole::RANGERS))
      Env::Mngr::rangers()->schedule_check(1);

  } else if(role_old & DB::Types::MngrRole::RANGERS) {
    SWC_LOG(LOG_INFO, "Manager(RANGERS) role has been decommissioned");
  }

  if(role_new & DB::Types::MngrRole::SCHEMAS) {
    if(!(role_old & DB::Types::MngrRole::SCHEMAS))
      Env::Mngr::mngd_columns()->initialize();

  } else if(role_old & DB::Types::MngrRole::SCHEMAS &&
            !(role_new & DB::Types::MngrRole::SCHEMAS)) {
    Env::Mngr::mngd_columns()->reset(true);
    SWC_LOG(LOG_INFO, "Manager(SCHEMAS) role has been decommissioned");
  }

  if(role_old & DB::Types::MngrRole::COLUMNS &&
     (role_new & DB::Types::MngrRole::NO_COLUMNS || !has_cols)) {
    SWC_LOG(LOG_INFO, "Manager(COLUMNS) role has been decommissioned");
  }

  if(!(role_new & DB::Types::MngrRole::RANGERS) &&
      (role_new & DB::Types::MngrRole::NO_COLUMNS || !has_cols)) {
    Env::Mngr::rangers()->stop(false);
  }

}
  
void MngrRole::set_mngr_inchain(const Comm::ConnHandlerPtr& mngr) {
  m_mngr_inchain->set(mngr);

  fill_states();
  m_checkin = false;
  schedule_checkin(cfg_check_interval->get());
}


} // namespace Manager


}


#include "swcdb/manager/ClientContextManager.cc"
#include "swcdb/manager/Protocol/Mngr/req/MngrState.cc"
