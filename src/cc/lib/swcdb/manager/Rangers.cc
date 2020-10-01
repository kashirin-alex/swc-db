
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/manager/Rangers.h"
#include "swcdb/manager/Protocol/Mngr/req/RgrUpdate.h"

#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.h"
#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.h"
#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.h"
#include "swcdb/manager/Protocol/Rgr/req/ColumnsUnload.h"

#include "swcdb/db/Protocol/Rgr/req/ColumnCompact.h"

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"



namespace SWC { namespace Manager {


Rangers::Rangers()
    : cfg_rgr_failures(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.ranges.assign.Rgr.remove.failures")),
      cfg_delay_rgr_chg(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.ranges.assign.delay.onRangerChange")),
      cfg_chk_assign(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.ranges.assign.interval.check")),
      cfg_assign_due(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.ranges.assign.due")),
      cfg_column_health_chk(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.column.health.interval.check")),
      cfg_column_health_chkers(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.column.health.checks")),
      m_run(true),
      m_timer(asio::high_resolution_timer(*Env::IoCtx::io()->ptr())),
      m_runs_assign(false), m_assignments(0) { 
}

Rangers::~Rangers() { }

void Rangers::stop(bool shuttingdown) {
  if(shuttingdown)
    m_run = false;
  {
    std::lock_guard lock(m_mutex_timer);
    m_timer.cancel();
  }
  {
    std::lock_guard lock(m_mutex);
    for(auto& h : m_rangers)
      Env::IoCtx::post([h]() { h->stop(); });
  }
}

bool Rangers::empty() {
  std::lock_guard lock(m_mutex);
  return m_rangers.empty();
}

void Rangers::schedule_check(uint32_t t_ms) {
  if(!m_run)
    return;

  std::lock_guard lock(m_mutex_timer);

  auto set_in = std::chrono::milliseconds(t_ms);
  auto set_on = m_timer.expiry();
  auto now = asio::high_resolution_timer::clock_type::now();
  if(set_on > now && set_on < now + set_in)
    return;

  m_timer.expires_after(set_in);
  m_timer.async_wait(
    [this](const asio::error_code& ec) {
      if (ec != asio::error::operation_aborted) {
        if(Env::Mngr::role()->is_active_role(DB::Types::MngrRole::RANGERS)) {
          std::lock_guard lock(m_mutex);
          m_rangers_resources.check(m_rangers);
          schedule_check(m_rangers_resources.cfg_rgr_res_check->get());
        }
        if(Env::Mngr::mngd_columns()->has_active()) {
          assign_ranges();
          health_check_columns();
          schedule_check(cfg_column_health_chk->get());
        }
      }
  });

  if(t_ms > 10000)
    SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM); );

  SWC_LOGF(LOG_DEBUG, "Rangers scheduled in ms=%d", t_ms);
}


void Rangers::rgr_report(rgrid_t rgrid, int err,
                         const Protocol::Rgr::Params::Report::RspRes& rsp) {
  if(m_rangers_resources.add_and_more(rgrid, err, rsp))
    return;

  m_rangers_resources.evaluate();
  RangerList changed;
  {
    std::lock_guard lock(m_mutex);
    m_rangers_resources.changes(m_rangers, changed);
  }
  if(!changed.empty())
    changes(changed);
}

Ranger::Ptr Rangers::rgr_get(const rgrid_t rgrid) {
  std::lock_guard lock(m_mutex);
  for(auto& rgr : m_rangers) {
    if(rgr->rgrid == rgrid)
      return rgr;
  }
  return nullptr;
}

void Rangers::rgr_get(const rgrid_t rgrid, Comm::EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);
  for(auto& rgr : m_rangers) {
    if(rgr->rgrid == rgrid) {
      if(rgr->state == Ranger::State::ACK)
        endpoints = rgr->endpoints;
      break;
    }
  }
}

void Rangers::rgr_get(const Ranger::Ptr& rgr, Comm::EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);
  endpoints = rgr->endpoints;
}

void Rangers::rgr_list(const rgrid_t rgrid, RangerList& rangers) {
  std::lock_guard lock(m_mutex);
  for(auto& rgr : m_rangers) {
    if(!rgrid || rgr->rgrid == rgrid) {
      rangers.push_back(rgr);
      if(rgrid)
        break;
    }
  }
}

rgrid_t Rangers::rgr_set_id(const Comm::EndPoints& endpoints, 
                            rgrid_t opt_rgrid) {
  std::lock_guard lock(m_mutex);
  return rgr_set(endpoints, opt_rgrid)->rgrid;
}

bool Rangers::rgr_ack_id(rgrid_t rgrid, const Comm::EndPoints& endpoints) {
  bool ack = false;
  Ranger::Ptr new_ack = nullptr;
  {
    std::lock_guard lock(m_mutex);
    
    for(auto& h : m_rangers) {
      if(Comm::has_endpoint(h->endpoints, endpoints) && rgrid == h->rgrid) {
        if(h->state != Ranger::State::ACK)
          new_ack = h;
        h->state = Ranger::State::ACK;
        ack = true;
        break;
      }
    }
  }

  if(new_ack) {
    changes({new_ack});
    schedule_check(500);
  }
  return ack;
}

rgrid_t Rangers::rgr_had_id(rgrid_t rgrid, const Comm::EndPoints& endpoints) {
  bool new_id_required = false;
  {
    std::lock_guard lock(m_mutex);

    for(auto& h : m_rangers) {
      if(rgrid == h->rgrid) {
        if(Comm::has_endpoint(h->endpoints, endpoints))
          return 0; // zero=OK
        new_id_required = true;
        break;
      }
    }
  }
  return rgr_set_id(endpoints, new_id_required ? 0 : rgrid);
}

void Rangers::rgr_shutdown(rgrid_t, const Comm::EndPoints& endpoints) {
  Ranger::Ptr removed = nullptr;
  {
    std::lock_guard lock(m_mutex);
    for(auto it=m_rangers.begin(); it<m_rangers.end(); ++it) {
      auto h = *it;
      if(Comm::has_endpoint(h->endpoints, endpoints)) {
        removed = h;
        m_rangers.erase(it);
        removed->state = Ranger::State::REMOVED;
        Env::Mngr::columns()->set_rgr_unassigned(removed->rgrid);
        break;
      }
    }
  }
  if(removed) {
    changes({removed});
    schedule_check(500);
  }
}


void Rangers::sync() {
  changes(m_rangers, true);
}

void Rangers::update_status(RangerList new_rgr_status, bool sync_all) {
  bool rangers_mngr = Env::Mngr::role()->is_active_role(
    DB::Types::MngrRole::RANGERS);

  if(rangers_mngr && !sync_all)
    return;

  RangerList changed;
  {
    Ranger::Ptr h;
    bool found;
    bool chg;
    bool load_scale_updated = false;

    std::lock_guard lock(m_mutex);

    for(auto& rs_new : new_rgr_status) {
      found = false;
      for(auto it=m_rangers.begin(); it<m_rangers.end(); ++it) {
        h = *it;
        if(!Comm::has_endpoint(h->endpoints, rs_new->endpoints))
          continue;

        chg = false;
        if(rs_new->rgrid != h->rgrid) { 
          if(rangers_mngr)
            rs_new->rgrid = rgr_set(
              rs_new->endpoints, rs_new->rgrid)->rgrid.load();
          
          if(rangers_mngr && rs_new->rgrid != h->rgrid)
            Env::Mngr::columns()->change_rgr(h->rgrid, rs_new->rgrid);

          h->rgrid = rs_new->rgrid.load();
          chg = true;
        }
        for(auto& endpoint: rs_new->endpoints) {
          if(!Comm::has_endpoint(endpoint, h->endpoints)) {
            h->set(rs_new->endpoints);
            chg = true;
            break;
          }
        }
        for(auto& endpoint: h->endpoints) {
          if(!Comm::has_endpoint(endpoint, rs_new->endpoints)) {
            h->set(rs_new->endpoints);
            chg = true;
            break;
          }
        }
        if(rs_new->state != h->state) {
          if(rs_new->state == Ranger::State::ACK) {
            if(h->state == Ranger::State::MARKED_OFFLINE) {
              cid_t cid_begin, cid_end = DB::Schema::NO_CID;
              if(Env::Mngr::mngd_columns()->active(cid_begin, cid_end))
                h->put(std::make_shared<Protocol::Rgr::Req::ColumnsUnload>(
                  h, cid_begin, cid_end)); 
            }
          } else {
            Env::Mngr::columns()->set_rgr_unassigned(h->rgrid);
            if(rs_new->state == Ranger::State::REMOVED)
              m_rangers.erase(it);
          }
          h->state = rs_new->state.load();
          chg = true;
        }

        if(rs_new->load_scale != h->load_scale) { 
          h->load_scale = rs_new->load_scale.load();
          load_scale_updated = true;
          chg = true;
        }

        if(chg && !sync_all)
          changed.push_back(rs_new);
        found = true;
        break;
      }

      if(!found) {
        if(rs_new->state == Ranger::State::ACK) {
          rs_new->init_queue();
          m_rangers.push_back(rs_new);
          if(!sync_all)
            changed.push_back(rs_new);
        }
      }
    }

    if(load_scale_updated) {
      for(auto& h : m_rangers)
        h->interm_ranges = 0;
    }
  }

  changes(
    sync_all ? m_rangers : changed, 
    sync_all && !rangers_mngr
  );
}

void Rangers::assign_range(const Ranger::Ptr& rgr, const Range::Ptr& range) {
  rgr->put(
    std::make_shared<Protocol::Rgr::Req::RangeLoad>(
      rgr, 
      range,
      Env::Mngr::schemas()->get(range->cfg->cid)
    )
  );
}

void Rangers::range_loaded(Ranger::Ptr rgr, Range::Ptr range, 
                           int err, bool failure, bool verbose) {
  bool run_assign;
  {
    std::lock_guard lock(m_mutex);
    run_assign = m_assignments-- == cfg_assign_due->get();
  }

  if(!range->deleted()) {
    if(err) {
      if(failure)
        ++rgr->failures;

      range->set_state(Range::State::NOTSET, 0); 
      if(!run_assign)
        schedule_check(2000);
      if(verbose)
        SWC_LOG_OUT(LOG_DEBUG,
          Error::print(SWC_LOG_OSTREAM << "RANGE-STATUS ", err);
          range->print(SWC_LOG_OSTREAM << ", ");
        );

    } else {
      rgr->failures = 0;
      range->set_state(Range::State::ASSIGNED, rgr->rgrid); 
      range->clear_last_rgr();
      Env::Mngr::mngd_columns()->load_pending(range->cfg->cid);
    }
  }

  if(run_assign)
    assign_ranges();
}


bool Rangers::update(const DB::Schema::Ptr& schema, bool ack_required) {
  std::vector<rgrid_t> rgrids;
  int err = Error::OK;
  Env::Mngr::columns()->get_column(err, schema->cid)
                          ->need_schema_sync(schema->revision, rgrids);
  bool undergo = false;
  for(rgrid_t rgrid : rgrids) {
    std::lock_guard lock(m_mutex);

    for(auto& rgr : m_rangers) {
      if(rgr->failures < cfg_rgr_failures->get() 
        && rgr->state == Ranger::State::ACK && rgr->rgrid == rgrid) {
        undergo = true;
        if(ack_required) {
          rgr->put(
            std::make_shared<Protocol::Rgr::Req::ColumnUpdate>(rgr, schema));
        }
      }
    }
  }
  return undergo;
}

void Rangers::column_delete(const cid_t cid, 
                            const std::vector<rgrid_t>& rgrids) {
  for(rgrid_t rgrid : rgrids) {
    std::lock_guard lock(m_mutex);
    for(auto& rgr : m_rangers) {
      if(rgrid != rgr->rgrid)
        continue;
      rgr->put(std::make_shared<Protocol::Rgr::Req::ColumnDelete>(rgr, cid));
    }
  }
}

void Rangers::column_compact(int& err, const cid_t cid) {
  auto col = Env::Mngr::columns()->get_column(err, cid);
  if(!err)  
    col->state(err);
  if(err)
    return;

  std::vector<rgrid_t> rgrids;
  col->assigned(rgrids);
  for(rgrid_t rgrid : rgrids) {
    std::lock_guard lock(m_mutex);

    for(auto& rgr : m_rangers) {
      if(rgr->failures < cfg_rgr_failures->get() 
        && rgr->state == Ranger::State::ACK && rgr->rgrid == rgrid) {
        rgr->put(std::make_shared<Protocol::Rgr::Req::ColumnCompact>(cid));
      }
    }
  }
}

void Rangers::need_health_check(const Column::Ptr& col) {
  {
    std::lock_guard lock(m_mutex_assign);
    auto it = std::find_if(
      m_columns_check.begin(), m_columns_check.end(), 
      [col](const ColumnHealthCheck::Ptr chk) { return chk->col == col; });
    if(it != m_columns_check.end())
      return;
  }
  col->reset_health_check();
  health_check_columns();
}

void Rangers::health_check_finished(const ColumnHealthCheck::Ptr& chk) {
  {
    std::lock_guard lock(m_mutex_assign);
    auto it = std::find(m_columns_check.begin(), m_columns_check.end(), chk);
    if(it != m_columns_check.end())
      m_columns_check.erase(it);
  }
  health_check_columns();
}

void Rangers::print(std::ostream& out) {
  out << "Rangers:";
  std::lock_guard lock(m_mutex);
  for(auto& h : m_rangers)
    h->print(out << "\n ");
}


bool Rangers::runs_assign(bool stop) {
  std::lock_guard lock(m_mutex_assign);
  if(stop) 
    return (m_runs_assign = false);
  else if(m_runs_assign)
    return true;
  m_runs_assign = true;
  return false;
}

void Rangers::assign_ranges() {
  if(!m_run || runs_assign(false))
    return;
  Env::IoCtx::post([this]() { assign_ranges_run(); });
}

void Rangers::assign_ranges_run() {
  int err;
  Range::Ptr range;

  for(bool more;;) {
    {
      std::lock_guard lock(m_mutex);
      if(m_rangers.empty() || !m_run) {
        runs_assign(true);
        return schedule_check();
      }
    }

    if(!(range = Env::Mngr::columns()->get_next_unassigned())) {
      runs_assign(true);
      break;
    }

    Files::RgrData::Ptr last_rgr = range->get_last_rgr(err = Error::OK);
    Ranger::Ptr rgr = nullptr;
    next_rgr(last_rgr->endpoints, rgr);
    if(!rgr) {
      runs_assign(true);
      return schedule_check();
    }

    range->set_state(Range::State::QUEUED, rgr->rgrid);
    ++rgr->interm_ranges;
    {
      std::lock_guard lock(m_mutex);
      if(!(more = ++m_assignments < cfg_assign_due->get()))
        runs_assign(true);
    }
    assign_range(rgr, range);
    if(!more)
      return;
  }

  schedule_check(cfg_chk_assign->get());
}

void Rangers::next_rgr(const Comm::EndPoints& last_rgr, Ranger::Ptr& rs_set) {
  size_t n_rgrs = 0;
  size_t avg_ranges = 0;
  Ranger::Ptr rgr;
  std::lock_guard lock(m_mutex);

  for(auto it=m_rangers.begin(); it<m_rangers.end(); ++it) {
    if((rgr = *it)->state == Ranger::State::MARKED_OFFLINE) {
      continue;

    } else if(rgr->failures >= cfg_rgr_failures->get()) {
      Env::Mngr::columns()->set_rgr_unassigned(rgr->rgrid);
      rgr->state = Ranger::State::MARKED_OFFLINE;
      _changes({rgr});

    } else if(rgr->state == Ranger::State::ACK) {
      if(!last_rgr.empty() && Comm::has_endpoint(rgr->endpoints, last_rgr)) {
        rs_set = rgr;
        return;
      }
      avg_ranges += rgr->interm_ranges;
      ++n_rgrs;
    }
  }
  if(!n_rgrs)
    return;

  avg_ranges /= n_rgrs;
  uint16_t best = 0;
  for(auto it=m_rangers.begin(); it<m_rangers.end(); ++it) {
    if((rgr = *it)->state == Ranger::State::ACK &&
        avg_ranges >= rgr->interm_ranges &&
        rgr->load_scale >= best) {
      best = rgr->load_scale;
      rs_set = rgr;
    }
  }
}

void Rangers::health_check_columns() {
  if(!m_mutex_assign.try_lock())
    return;
  int64_t ts = Time::now_ms();
  uint32_t intval = cfg_column_health_chk->get();
  for(Column::Ptr col; 
      m_columns_check.size() < size_t(cfg_column_health_chkers->get()) &&
      (col = Env::Mngr::columns()->get_need_health_check(ts, intval)); ) {
    m_columns_check.emplace_back(
      new ColumnHealthCheck(col, ts, intval));
    Env::IoCtx::post([chk=m_columns_check.back()]() { chk->run(); });
  }
  m_mutex_assign.unlock();
}


Ranger::Ptr Rangers::rgr_set(const Comm::EndPoints& endpoints, rgrid_t opt_rgrid) {
  for(auto it=m_rangers.begin();it<m_rangers.end(); ++it) {
    auto h = *it;
    if(Comm::has_endpoint(h->endpoints, endpoints)) {
      if(h->state == Ranger::State::ACK) {
        h->set(endpoints);
        return h;
      } else {
        Env::Mngr::columns()->set_rgr_unassigned(h->rgrid);
        m_rangers.erase(it);
        break;
      }
    }
  }

  rgrid_t next_id=0;
  rgrid_t nxt;
  bool ok;
  do {
    if(!opt_rgrid) {
      nxt = ++next_id;
    } else {
      nxt = opt_rgrid;
      opt_rgrid = 0;
    }
      
    ok = true;
    for(auto& h : m_rangers) {
      if(nxt == h->rgrid) {
        ok = false;
        break;
      };
    }
  } while(!ok);

  Ranger::Ptr& h = m_rangers.emplace_back(new Ranger(nxt, endpoints));
  h->init_queue();
  return h;
}


void Rangers::changes(const RangerList& hosts, bool sync_all) {
  {
    std::lock_guard lock(m_mutex);
    _changes(hosts, sync_all);
  }
    
  if(Env::Mngr::mngd_columns()->has_active())
    schedule_check(cfg_delay_rgr_chg->get());
}

void Rangers::_changes(const RangerList& hosts, bool sync_all) {
  if(hosts.empty()) 
    return;

  // batches of 1000 hosts a req
  for(auto it = hosts.cbegin(); it < hosts.cend(); ) {
    auto from = it;
    Env::Mngr::role()->req_mngr_inchain(
      std::make_shared<Protocol::Mngr::Req::RgrUpdate>(
        RangerList(from, (it+=1000) < hosts.cend() ? it : hosts.cend()),
        sync_all
      )
    );
  }

  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_OSTREAM << " Rangers::changes:";
    for(auto& h : hosts)
      h->print(SWC_LOG_OSTREAM << "\n ");
  );
}



}} // namespace SWC::Manager




#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.cc"
#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.cc"
#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.cc"
#include "swcdb/manager/Protocol/Rgr/req/ReportRes.cc"
