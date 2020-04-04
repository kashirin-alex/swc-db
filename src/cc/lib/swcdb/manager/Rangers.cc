
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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
      m_assign_timer(asio::high_resolution_timer(*Env::IoCtx::io()->ptr())),
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
    m_assign_timer.cancel();
  }
  {
    std::lock_guard lock(m_mutex);
    for(auto& h : m_rangers)
      asio::post(*Env::IoCtx::io()->ptr(), [h]() { h->stop(); });
  }
}

void Rangers::schedule_assignment_check(uint32_t t_ms) {
  if(!m_run)
    return;

  std::lock_guard lock(m_mutex_timer);

  auto set_in = std::chrono::milliseconds(t_ms);
  auto set_on = m_assign_timer.expires_from_now();
  if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
    return;
  m_assign_timer.cancel();
  m_assign_timer.expires_from_now(set_in);

  m_assign_timer.async_wait(
    [this](const asio::error_code ec) {
      if (ec != asio::error::operation_aborted) {
        assign_ranges();
      }
  }); 

  if(t_ms > 10000)
    SWC_LOGF(LOG_DEBUG, "%s", to_string().c_str());

  SWC_LOGF(LOG_DEBUG, "Rangers assign_ranges scheduled in ms=%d", t_ms);
}


void Rangers::rgr_get(const uint64_t id, EndPoints& endpoints) {
  std::lock_guard lock(m_mutex);
  for(auto& rgr : m_rangers) {
    if(rgr->id == id) {
      if(rgr->state == Ranger::State::ACK)
        endpoints = rgr->endpoints;
      break;
    }
  }
}

void Rangers::rgr_list(const uint64_t rgr_id, RangerList& rangers) {
  std::lock_guard lock(m_mutex);
  for(auto& rgr : m_rangers) {
    if(!rgr_id || rgr->id == rgr_id) {
      rangers.push_back(rgr);
      if(rgr_id)
        break;
    }
  }
}

uint64_t Rangers::rgr_set_id(const EndPoints& endpoints, uint64_t opt_id) {
  std::lock_guard lock(m_mutex);
  return rgr_set(endpoints, opt_id)->id;
}

bool Rangers::rgr_ack_id(uint64_t id, const EndPoints& endpoints) {
  bool ack = false;
  Ranger::Ptr new_ack = nullptr;
  {
    std::lock_guard lock(m_mutex);
    
    for(auto& h : m_rangers) {
      if(has_endpoint(h->endpoints, endpoints) && id == h->id) {
        if(h->state != Ranger::State::ACK)
          new_ack = h;
        h->state = Ranger::State::ACK;
        ack = true;
        break;
      }
    }
  }

  if(new_ack != nullptr) {
    RangerList hosts({new_ack});
    changes(hosts);
  }
  return ack;
}

uint64_t Rangers::rgr_had_id(uint64_t id, const EndPoints& endpoints) {
  bool new_id_required = false;
  {
    std::lock_guard lock(m_mutex);

    for(auto& h : m_rangers) {
      if(id == h->id) {
        if(has_endpoint(h->endpoints, endpoints))
          return 0; // zero=OK
        new_id_required = true;
        break;
      }
    }
  }
  return rgr_set_id(endpoints, new_id_required ? 0 : id);
}

void Rangers::rgr_shutdown(uint64_t id, const EndPoints& endpoints) {
  Ranger::Ptr removed = nullptr;
  {
    std::lock_guard lock(m_mutex);
    for(auto it=m_rangers.begin();it<m_rangers.end(); ++it) {
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)) {
        removed = h;
        m_rangers.erase(it);
        removed->state = Ranger::State::REMOVED;
        Env::Mngr::columns()->set_rgr_unassigned(removed->id);
        break;
      }
    }
  }
  if(removed != nullptr) {
    RangerList hosts({removed});
    changes(hosts);
  }
}

  
void Rangers::sync() {
  changes(m_rangers, true);
}

void Rangers::update_status(RangerList new_rgr_status, bool sync_all) {
  if(Env::Mngr::mngd_columns()->is_root_mngr() && !sync_all)
    return;

  RangerList changed;
  {
    Ranger::Ptr h;
    bool found;
    bool chg;

    std::lock_guard lock(m_mutex);

    for(auto& rs_new : new_rgr_status) {
      found = false;
      for(auto it=m_rangers.begin();it<m_rangers.end(); ++it) {
        h = *it;
        if(!has_endpoint(h->endpoints, rs_new->endpoints))
          continue;

        chg = false;
        if(rs_new->id != h->id) { 
          if(Env::Mngr::mngd_columns()->is_root_mngr())
            rs_new->id = rgr_set(rs_new->endpoints, rs_new->id)->id;
          
          if(Env::Mngr::mngd_columns()->is_root_mngr() && rs_new->id != h->id)
            Env::Mngr::columns()->change_rgr(h->id, rs_new->id);

          h->id = rs_new->id;
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
          Env::Mngr::columns()->set_rgr_unassigned(h->id);
          m_rangers.erase(it);
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
  if(!changed.empty())
    std::cout << " update_status: ";

  changes(
    sync_all ? m_rangers : changed, 
    sync_all && !Env::Mngr::mngd_columns()->is_root_mngr()
  );
}


void Rangers::assign_range_chk_last(int err, Ranger::Ptr rs_chk) {
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

void Rangers::assign_range(Ranger::Ptr rgr, Range::Ptr range) {
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
      rgr->total_ranges--;
      if(failure)
        ++rgr->failures;

      range->set_state(Range::State::NOTSET, 0); 
      if(!run_assign) 
        schedule_assignment_check(2000);

    } else {
      rgr->failures=0;
      range->set_state(Range::State::ASSIGNED, rgr->id); 
      range->clear_last_rgr();
      // adjust rgr->resource
      // ++ mng_inchain - req. MngrRsResource

      Env::Mngr::mngd_columns()->load_pending(range->cfg->cid);
    }
    if(verbose)
      SWC_LOGF(LOG_INFO, "RANGE-STATUS %d(%s), %s", 
                err, Error::get_text(err), range->to_string().c_str());
  }

  if(run_assign)
    assign_ranges();
}


bool Rangers::update(DB::Schema::Ptr schema, bool ack_required) {
  std::vector<uint64_t> rgr_ids;
  int err = Error::OK;
  Env::Mngr::columns()->get_column(err, schema->cid, false)
                          ->need_schema_sync(schema->revision, rgr_ids);
  bool undergo = false;
  for(auto& id : rgr_ids) {
    std::lock_guard lock(m_mutex);

    for(auto& rgr : m_rangers) {
      if(rgr->failures < cfg_rgr_failures->get() 
        && rgr->state == Ranger::State::ACK && rgr->id == id) {
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

void Rangers::column_delete(const int64_t cid, 
                            const std::vector<uint64_t>& rgr_ids) {
  for(auto id : rgr_ids) {
    std::lock_guard lock(m_mutex);
    for(auto& rgr : m_rangers) {
      if(id != rgr->id)
        continue;
      rgr->total_ranges--; // reduce all ranges-count of cid
      rgr->put(std::make_shared<Protocol::Rgr::Req::ColumnDelete>(rgr, cid));
    }
  }
}

void Rangers::column_compact(int& err, const int64_t cid) {
  auto col = Env::Mngr::columns()->get_column(err, cid, false);
  if(!err)  
    col->state(err);
  if(err)
    return;

  std::vector<uint64_t> rgr_ids;
  col->assigned(rgr_ids);
  for(auto& id : rgr_ids) {
    std::lock_guard lock(m_mutex);

    for(auto& rgr : m_rangers) {
      if(rgr->failures < cfg_rgr_failures->get() 
        && rgr->state == Ranger::State::ACK && rgr->id == id) {
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
        schedule_assignment_check();
        return;
      }
    }

    if((range = Env::Mngr::columns()->get_next_unassigned()) == nullptr) {
      runs_assign(true);
      break;
    }

    err = Error::OK;
    Files::RgrData::Ptr last_rgr = range->get_last_rgr(err);
    Ranger::Ptr rgr = nullptr;
    next_rgr(last_rgr, rgr);
    if(rgr == nullptr) {
      runs_assign(true);
      schedule_assignment_check();
      return;
    }

    range->set_state(Range::State::QUEUED, rgr->id);
    assign_range(rgr, range, last_rgr);
    if(++m_assignments > cfg_assign_due->get()) {
      runs_assign(true);
      return;
    }
  }

  // balance/check-assigments if not runs :for ranger cid-rid state
  schedule_assignment_check(cfg_chk_assign->get());
}

void Rangers::next_rgr(Files::RgrData::Ptr &last_rgr, Ranger::Ptr &rs_set) {
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

  while(rs_set == nullptr && m_rangers.size()) {
    avg_ranges = 0;
    num_rgr = 0;
    // avg_resource_ratio = 0;
    for(auto it=m_rangers.begin();it<m_rangers.end(); ++it) {
      rgr = *it;
      if(rgr->state != Ranger::State::ACK)
        continue;
      avg_ranges = avg_ranges*num_rgr + rgr->total_ranges;
      // resource_ratio = avg_resource_ratio*num_rgr + rgr->resource();
      avg_ranges /= ++num_rgr;
      // avg_resource_ratio /= num_rgr;
    }

    for(auto it=m_rangers.begin();it<m_rangers.end(); ++it) {
      rgr = *it;
      if(rgr->state != Ranger::State::ACK || avg_ranges < rgr->total_ranges)
        continue;

      if(rgr->failures >= cfg_rgr_failures->get()) {
        m_rangers.erase(it);
        Env::Mngr::columns()->set_rgr_unassigned(rgr->id);
        continue;
      }
      rs_set = rgr;
      break;
    }
  }

  if(rs_set != nullptr)
    ++rs_set->total_ranges;
  return;
}

void Rangers::assign_range(Ranger::Ptr rgr, Range::Ptr range, 
                           Files::RgrData::Ptr last_rgr) {
  if(last_rgr == nullptr) {
    assign_range(rgr, range);
    return;
  }

  bool id_due;
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
  if(rs_last == nullptr) {
    std::lock_guard lock(m_mutex);
    rs_last = m_rangers.emplace_back(new Ranger(0, last_rgr->endpoints));
    rs_last->init_queue();
    rs_last->state = Ranger::State::AWAIT;
    id_due = false;
  }
    
  auto req = std::make_shared<Protocol::Rgr::Req::AssignIdNeeded>(
    rs_last, rgr, range);
  if(id_due)
    rs_last->pending_id(req);
  else
    rs_last->put(req);
}

Ranger::Ptr Rangers::rgr_set(const EndPoints& endpoints, uint64_t opt_id) {
  for(auto it=m_rangers.begin();it<m_rangers.end(); ++it) {
    auto h = *it;
    if(has_endpoint(h->endpoints, endpoints)) {
      if(h->state == Ranger::State::ACK) {
        h->set(endpoints);
        return h;
      } else {
        Env::Mngr::columns()->set_rgr_unassigned(h->id);
        m_rangers.erase(it);
        break;
      }
    }
  }

  uint64_t next_id=0;
  uint64_t nxt;
  bool ok;
  do {
    if(!opt_id) {
      nxt = ++next_id;
    } else {
      nxt = opt_id;
      opt_id = 0;
    }
      
    ok = true;
    for(auto& h : m_rangers) {
      if(nxt == h->id) {
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
    if(hosts.size()) {
      Env::Mngr::role()->req_mngr_inchain(
        std::make_shared<Protocol::Mngr::Req::RgrUpdate>(
        hosts, sync_all));

      std::cout << " changes: \n";
      for(auto& h : hosts)
        std::cout << " " << h->to_string() << "\n";
    }
  }
    
  if(Env::Mngr::mngd_columns()->has_active())
    schedule_assignment_check(cfg_delay_rgr_chg->get());
}



}} // namespace SWC::Manager




#include "swcdb/manager/Protocol/Rgr/req/AssignIdNeeded.cc"
#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.cc"
#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.cc"
#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.cc"
