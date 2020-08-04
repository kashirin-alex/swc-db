
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/manager/Rangers.h"
#include "swcdb/manager/Protocol/Mngr/req/RgrUpdate.h"

#include "swcdb/manager/Protocol/Rgr/req/AssignIdNeeded.h"
#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.h"
#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.h"
#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.h"

#include "swcdb/db/Protocol/Rgr/req/ColumnCompact.h"

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"



namespace SWC { namespace Manager {


Rangers::Rangers()
    : m_run(true),
      m_timer(asio::high_resolution_timer(*Env::IoCtx::io()->ptr())),
      m_runs_assign(false), m_assignments(0),
      cfg_rgr_failures(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.ranges.assign.Rgr.remove.failures")),
      cfg_delay_rgr_chg(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.ranges.assign.delay.onRangerChange")),
      cfg_chk_assign(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.ranges.assign.interval.check")),
      cfg_assign_due(Env::Config::settings()->get<Property::V_GINT32>(
        "swc.mngr.ranges.assign.due")) { 
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
      asio::post(*Env::IoCtx::io()->ptr(), [h]() { h->stop(); });
  }
}

void Rangers::schedule_check(uint32_t t_ms) {
  if(!m_run)
    return;

  std::lock_guard lock(m_mutex_timer);

  auto set_in = std::chrono::milliseconds(t_ms);
  auto set_on = m_timer.expires_from_now();
  if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
    return;
  m_timer.cancel();
  m_timer.expires_from_now(set_in);

  m_timer.async_wait(
    [this](const asio::error_code& ec) {
      if (ec != asio::error::operation_aborted) {
        if(Env::Mngr::role()->is_active_role(Types::MngrRole::RANGERS)) {
          std::lock_guard lock(m_mutex);
          m_rangers_resources.check(m_rangers);
        }
        assign_ranges();
      }
  });

  if(t_ms > 10000)
    SWC_LOGF(LOG_DEBUG, "%s", to_string().c_str());

  SWC_LOGF(LOG_DEBUG, "Rangers assign_ranges scheduled in ms=%d", t_ms);
}


void Rangers::rgr_report(rgrid_t rgrid, 
                         const Protocol::Rgr::Params::ReportResRsp& rsp) {
  if(m_rangers_resources.add_and_more(rgrid, rsp))
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


void Rangers::rgr_get(const rgrid_t rgrid, EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);
  for(auto& rgr : m_rangers) {
    if(rgr->rgrid == rgrid) {
      if(rgr->state == Ranger::State::ACK)
        endpoints = rgr->endpoints;
      break;
    }
  }
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

rgrid_t Rangers::rgr_set_id(const EndPoints& endpoints, rgrid_t opt_rgrid) {
  std::lock_guard lock(m_mutex);
  return rgr_set(endpoints, opt_rgrid)->rgrid;
}

bool Rangers::rgr_ack_id(rgrid_t rgrid, const EndPoints& endpoints) {
  bool ack = false;
  Ranger::Ptr new_ack = nullptr;
  {
    std::lock_guard lock(m_mutex);
    
    for(auto& h : m_rangers) {
      if(has_endpoint(h->endpoints, endpoints) && rgrid == h->rgrid) {
        if(h->state != Ranger::State::ACK)
          new_ack = h;
        h->state = Ranger::State::ACK;
        ack = true;
        break;
      }
    }
  }

  if(new_ack) {
    RangerList hosts({new_ack});
    changes(hosts);
  }
  return ack;
}

rgrid_t Rangers::rgr_had_id(rgrid_t rgrid, const EndPoints& endpoints) {
  bool new_id_required = false;
  {
    std::lock_guard lock(m_mutex);

    for(auto& h : m_rangers) {
      if(rgrid == h->rgrid) {
        if(has_endpoint(h->endpoints, endpoints))
          return 0; // zero=OK
        new_id_required = true;
        break;
      }
    }
  }
  return rgr_set_id(endpoints, new_id_required ? 0 : rgrid);
}

void Rangers::rgr_shutdown(rgrid_t, const EndPoints& endpoints) {
  Ranger::Ptr removed = nullptr;
  {
    std::lock_guard lock(m_mutex);
    for(auto it=m_rangers.begin(); it<m_rangers.end(); ++it) {
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)) {
        removed = h;
        m_rangers.erase(it);
        removed->state = Ranger::State::REMOVED;
        Env::Mngr::columns()->set_rgr_unassigned(removed->rgrid);
        break;
      }
    }
  }
  if(removed) {
    RangerList hosts({removed});
    changes(hosts);
  }
}


void Rangers::sync() {
  changes(m_rangers, true);
}

void Rangers::update_status(RangerList new_rgr_status, bool sync_all) {
  bool rangers_mngr = Env::Mngr::role()->is_active_role(
    Types::MngrRole::RANGERS);

  if(rangers_mngr && !sync_all)
    return;

  RangerList changed;
  {
    Ranger::Ptr h;
    bool found;
    bool chg;

    std::lock_guard lock(m_mutex);

    for(auto& rs_new : new_rgr_status) {
      found = false;
      for(auto it=m_rangers.begin(); it<m_rangers.end(); ++it) {
        h = *it;
        if(!has_endpoint(h->endpoints, rs_new->endpoints))
          continue;

        chg = false;
        if(rs_new->rgrid != h->rgrid) { 
          if(rangers_mngr)
            rs_new->rgrid = rgr_set(rs_new->endpoints, rs_new->rgrid)->rgrid;
          
          if(rangers_mngr && rs_new->rgrid != h->rgrid)
            Env::Mngr::columns()->change_rgr(h->rgrid, rs_new->rgrid);

          h->rgrid = rs_new->rgrid;
          chg = true;
        }
        for(auto& endpoint: rs_new->endpoints) {
          if(!has_endpoint(endpoint, h->endpoints)) {
            h->set(rs_new->endpoints);
            chg = true;
            break;
          }
        }
        for(auto& endpoint: h->endpoints) {
          if(!has_endpoint(endpoint, rs_new->endpoints)) {
            h->set(rs_new->endpoints);
            chg = true;
            break;
          }
        }
        if(rs_new->state == Ranger::State::ACK) {
          if(rs_new->state != h->state) {
            h->state = Ranger::State::ACK;
            chg = true;
          }
        } else {
          Env::Mngr::columns()->set_rgr_unassigned(h->rgrid);
          m_rangers.erase(it);
          chg = true;
        }

        if(rs_new->load_scale != h->load_scale) { 
          h->load_scale = rs_new->load_scale.load();
          h->interm_ranges = 0;
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
  }

  changes(
    sync_all ? m_rangers : changed, 
    sync_all && !rangers_mngr
  );
}


void Rangers::assign_range_chk_last(int err, const Ranger::Ptr& rs_chk) {
  client::ConnQueue::ReqBase::Ptr req;
  for(;;) {
    {
      std::lock_guard lock(m_mutex);
      if(!rs_chk->pending_id_pop(req))
        return;
    }

    auto qreq = std::dynamic_pointer_cast<
      Protocol::Rgr::Req::AssignIdNeeded>(req);
    if(!err) 
      range_loaded(qreq->rs_nxt, qreq->range, Error::RS_NOT_READY);
    else
      assign_range(qreq->rs_nxt, qreq->range);
  }
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
  bool run_assign = m_assignments-- > cfg_assign_due->get();           

  if(!range->deleted()) {
    if(err) {
      if(failure)
        ++rgr->failures;

      range->set_state(Range::State::NOTSET, 0); 
      if(!run_assign) 
        schedule_check(2000);

    } else {
      rgr->failures = 0;
      range->set_state(Range::State::ASSIGNED, rgr->rgrid); 
      range->clear_last_rgr();
      Env::Mngr::mngd_columns()->load_pending(range->cfg->cid);
    }
    if(verbose)
      SWC_LOGF(LOG_INFO, "RANGE-STATUS %d(%s), %s", 
                err, Error::get_text(err), range->to_string().c_str());
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


std::string Rangers::to_string() {
  std::string s("Rangers:");
  std::lock_guard lock(m_mutex);
  for(auto& h : m_rangers) {
    s.append("\n ");
    s.append(h->to_string());
  }
  return s;
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
  asio::post(*Env::IoCtx::io()->ptr(), [this]() { assign_ranges_run(); });
}

void Rangers::assign_ranges_run() {
  int err;
  Range::Ptr range;

  for(;;) {
    {
      std::lock_guard lock(m_mutex);
      if(m_rangers.empty() || !m_run) {
        runs_assign(true);
        schedule_check();
        return;
      }
    }

    if(!(range = Env::Mngr::columns()->get_next_unassigned())) {
      runs_assign(true);
      break;
    }

    Files::RgrData::Ptr last_rgr = range->get_last_rgr(err = Error::OK);
    Ranger::Ptr rgr = nullptr;
    next_rgr(last_rgr, rgr);
    if(!rgr) {
      runs_assign(true);
      schedule_check();
      return;
    }

    range->set_state(Range::State::QUEUED, rgr->rgrid);
    assign_range(rgr, range, last_rgr);
    if(++m_assignments > cfg_assign_due->get()) {
      runs_assign(true);
      return;
    }
  }

  // balance/check-assigments if not runs :for ranger cid-rid state
  schedule_check(cfg_chk_assign->get());
}

void Rangers::next_rgr(Files::RgrData::Ptr& last_rgr, Ranger::Ptr& rs_set) {
  std::lock_guard lock(m_mutex);

  if(last_rgr->endpoints.size()) {
      for(auto& rgr : m_rangers) {
        if(rgr->state == Ranger::State::ACK
          && rgr->failures < cfg_rgr_failures->get() 
          && has_endpoint(rgr->endpoints, last_rgr->endpoints)) {
          rs_set = rgr;
          last_rgr = nullptr;
          break;
        }
     }
  } else 
    last_rgr = nullptr;
    
  size_t num_rgr;
  size_t avg_ranges;
  Ranger::Ptr rgr;

  while(!rs_set && m_rangers.size()) {

    avg_ranges = 0;
    num_rgr = 0;
    for(auto it=m_rangers.begin(); it<m_rangers.end(); ) {
      if((rgr = *it)->failures >= cfg_rgr_failures->get()) {
        m_rangers.erase(it);
        Env::Mngr::columns()->set_rgr_unassigned(rgr->rgrid);
        continue;
      }
      ++it;
      if(rgr->state != Ranger::State::ACK)
        continue;
      avg_ranges += rgr->interm_ranges;
      ++num_rgr;
    }
    if(num_rgr)
      avg_ranges /= num_rgr;

    uint16_t best = 0;
    for(auto it=m_rangers.begin(); it<m_rangers.end(); ++it) {
      if((rgr = *it)->state != Ranger::State::ACK ||
         avg_ranges < rgr->interm_ranges ||
         rgr->load_scale < best)
        continue;
      best = rgr->load_scale;
      rs_set = rgr;
    }
  }

  if(rs_set)
    ++rs_set->interm_ranges;
  return;
}

void Rangers::assign_range(const Ranger::Ptr& rgr, const Range::Ptr& range, 
                           const Files::RgrData::Ptr& last_rgr) {
  if(!last_rgr)
    return assign_range(rgr, range);

  bool id_due = false;
  Ranger::Ptr rs_last = nullptr;
  {
    std::lock_guard lock(m_mutex);
    for(auto& rs_chk : m_rangers) {
      if(has_endpoint(rs_chk->endpoints, last_rgr->endpoints)) {
        rs_last = rs_chk;
        id_due = rs_last->state == Ranger::State::AWAIT;
        rs_last->state = Ranger::State::AWAIT;
        break;
      }
    }
  }
  if(!rs_last) {
    std::lock_guard lock(m_mutex);
    rs_last = m_rangers.emplace_back(new Ranger(0, last_rgr->endpoints));
    rs_last->init_queue();
    rs_last->state = Ranger::State::AWAIT;
  }
    
  auto req = std::make_shared<Protocol::Rgr::Req::AssignIdNeeded>(
    rs_last, rgr, range);
  if(id_due)
    rs_last->pending_id(req);
  else
    rs_last->put(req);
}

Ranger::Ptr Rangers::rgr_set(const EndPoints& endpoints, rgrid_t opt_rgrid) {
  for(auto it=m_rangers.begin();it<m_rangers.end(); ++it) {
    auto h = *it;
    if(has_endpoint(h->endpoints, endpoints)) {
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


void Rangers::changes(RangerList& hosts, bool sync_all) {
  {
    std::lock_guard lock(m_mutex);
    if(!hosts.empty()) {
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

      SWC_LOG_OUT(LOG_INFO) << "Rangers::changes:\n";
      for(auto& h : hosts)
        std::cout << " " << h->to_string() << "\n";
      std::cout << SWC_LOG_OUT_END;
    }
  }
    
  if(Env::Mngr::mngd_columns()->has_active())
    schedule_check(cfg_delay_rgr_chg->get());
}



}} // namespace SWC::Manager




#include "swcdb/manager/Protocol/Rgr/req/AssignIdNeeded.cc"
#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.cc"
#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.cc"
#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.cc"
#include "swcdb/manager/Protocol/Rgr/req/ReportRes.cc"
