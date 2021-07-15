/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/manager/Rangers.h"
#include "swcdb/manager/Protocol/Mngr/req/RgrUpdate.h"

#include "swcdb/manager/Protocol/Rgr/req/RangeLoad.h"
#include "swcdb/manager/Protocol/Rgr/req/RangeUnload.h"

#include "swcdb/manager/Protocol/Rgr/req/ColumnUpdate.h"
#include "swcdb/manager/Protocol/Rgr/req/ColumnDelete.h"
#include "swcdb/manager/Protocol/Rgr/req/ColumnsUnload.h"

#include "swcdb/db/Protocol/Rgr/req/ColumnCompact.h"

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"



namespace SWC { namespace Manager {


Rangers::Rangers(const Comm::IoContextPtr& app_io)
    : cfg_rgr_failures(
        Env::Config::settings()->get<Config::Property::V_GUINT16>(
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
      cfg_column_health_chkers_delay(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.mngr.column.health.checks.delay")),
      m_run(true),
      m_timer(asio::high_resolution_timer(app_io->executor())),
      m_assignments(0) {
}

void Rangers::stop(bool shuttingdown) {
   if(shuttingdown)
    m_run.store(false);
  {
    Core::MutexSptd::scope lock(m_mutex);
    m_timer.cancel();
  }
  for(Ranger::Ptr h;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_rangers.empty())
        break;
      h = m_rangers.front();
      m_rangers.erase(m_rangers.cbegin());
    }
    h->stop();
  }
  if(shuttingdown)
    wait_health_check();
}

SWC_CAN_INLINE
bool Rangers::empty() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_rangers.empty();
}

void Rangers::schedule_check(uint32_t t_ms) {
  if(!m_run)
    return;

  Core::MutexSptd::scope lock(m_mutex);

  auto set_in = std::chrono::milliseconds(t_ms);
  auto set_on = m_timer.expiry();
  auto now = asio::high_resolution_timer::clock_type::now();
  if(set_on > now && set_on < now + set_in)
    return;

  m_timer.expires_after(set_in);
  m_timer.async_wait([this](const asio::error_code& ec) {
    if(ec != asio::error::operation_aborted)
      schedule_run();
  });

  if(t_ms > 10000)
    SWC_LOGF(LOG_DEBUG, "Rangers scheduled in ms=%u", t_ms);
}

void Rangers::schedule_run() {
  if(Env::Mngr::role()->is_active_role(DB::Types::MngrRole::RANGERS)) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      m_rangers_resources.check(m_rangers);
    }
    schedule_check(m_rangers_resources.cfg_check->get());
  }
  if(Env::Mngr::mngd_columns()->has_active()) {
    assign_ranges();
    health_check_columns();
    schedule_check(cfg_column_health_chk->get());
  }
}

void Rangers::rgr_report(
      rgrid_t rgrid,
      int err,
      const Comm::Protocol::Rgr::Params::Report::RspRes& rsp) {
  if(m_rangers_resources.add_and_more(rgrid, err, rsp))
    return;

  m_rangers_resources.evaluate();
  RangerList changed;
  {
    Core::MutexSptd::scope lock(m_mutex);
    m_rangers_resources.changes(m_rangers, changed);
  }
  if(!changed.empty())
    changes(changed);
}

Ranger::Ptr Rangers::rgr_get(const rgrid_t rgrid) {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto& rgr : m_rangers) {
    if(rgr->rgrid == rgrid)
      return rgr;
  }
  return nullptr;
}

SWC_CAN_INLINE
void Rangers::rgr_get(const rgrid_t rgrid, Comm::EndPoints& endpoints) {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto& rgr : m_rangers) {
    if(rgr->rgrid == rgrid) {
      if(rgr->state & RangerState::ACK)
        endpoints = rgr->endpoints;
      break;
    }
  }
}

void Rangers::rgr_get(const Ranger::Ptr& rgr, Comm::EndPoints& endpoints) {
  Core::MutexSptd::scope lock(m_mutex);
  endpoints = rgr->endpoints;
}

void Rangers::rgr_list(const rgrid_t rgrid, RangerList& rangers) {
  Core::MutexSptd::scope lock(m_mutex);
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
  Core::MutexSptd::scope lock(m_mutex);
  return rgr_set(endpoints, opt_rgrid)->rgrid;
}

bool Rangers::rgr_ack_id(rgrid_t rgrid, const Comm::EndPoints& endpoints) {
  bool ack = false;
  Ranger::Ptr new_ack = nullptr;
  {
    uint8_t state;
    Core::MutexSptd::scope lock(m_mutex);

    for(auto& h : m_rangers) {
      if(Comm::has_endpoint(h->endpoints, endpoints) && rgrid == h->rgrid) {
        state = h->state.load();
        if(!(state & RangerState::ACK)) {
          if(state != RangerState::ACK)
            new_ack = h;
          h->state.store(RangerState::ACK);
        }
        ack = true;
        break;
      }
    }
  }

  if(new_ack) {
    RangerList chged;
    chged.push_back(new_ack);
    changes(chged);
    schedule_check(500);
  }
  return ack;
}

rgrid_t Rangers::rgr_had_id(rgrid_t rgrid, const Comm::EndPoints& endpoints) {
  bool new_id_required = false;
  {
    Core::MutexSptd::scope lock(m_mutex);
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
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it=m_rangers.cbegin(); it != m_rangers.cend(); ++it) {
      if(Comm::has_endpoint((*it)->endpoints, endpoints)) {
        removed = std::move(*it);
        m_rangers.erase(it);
        removed->state.store(RangerState::REMOVED);
        Env::Mngr::columns()->set_rgr_unassigned(removed->rgrid);
        break;
      }
    }
  }
  if(removed) {
    RangerList chged;
    chged.push_back(removed);
    changes(chged);
    schedule_check(5000);
  }
}


SWC_CAN_INLINE
void Rangers::sync() {
  changes(m_rangers, true);
}

void Rangers::update_status(const RangerList& new_rgr_status, bool sync_all) {
  bool rangers_mngr = Env::Mngr::role()->is_active_role(
    DB::Types::MngrRole::RANGERS);

  if(rangers_mngr && !sync_all)
    return;

  RangerList changed;
  RangerList balance_rangers;
  {
    Ranger::Ptr h;
    bool found;
    bool chg;

    Core::MutexSptd::scope lock(m_mutex);

    for(auto& rs_new : new_rgr_status) {
      found = false;
      for(auto it=m_rangers.cbegin(); it != m_rangers.cend(); ++it) {
        h = *it;
        if(!Comm::has_endpoint(h->endpoints, rs_new->endpoints))
          continue;

        found = true;
        chg = false;
        if(rs_new->rgrid != h->rgrid) {
          if(rangers_mngr)
            rs_new->rgrid.store(
              rgr_set(rs_new->endpoints, rs_new->rgrid)->rgrid.load());

          if(rangers_mngr && rs_new->rgrid != h->rgrid)
            Env::Mngr::columns()->change_rgr(h->rgrid, rs_new->rgrid);

          h->rgrid.store(rs_new->rgrid);
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
          if(rs_new->state & RangerState::ACK) {
            if(h->state & RangerState::SHUTTINGDOWN) {
              rs_new->state.store(h->state);
            } else if(rs_new->state == RangerState::ACK &&
                      h->state == RangerState::MARKED_OFFLINE) {
              cid_t cid_begin, cid_end = DB::Schema::NO_CID;
              if(Env::Mngr::mngd_columns()->active(cid_begin, cid_end))
                h->put(Comm::Protocol::Rgr::Req::ColumnsUnload::Ptr(
                  new Comm::Protocol::Rgr::Req::ColumnsUnload(
                    h, cid_begin, cid_end)
                ));
              h->failures.store(0);
            }
          } else {
            Env::Mngr::columns()->set_rgr_unassigned(h->rgrid);
            if(rs_new->state == RangerState::REMOVED)
              m_rangers.erase(it);
          }
          h->state.store(rs_new->state);
          chg = true;
        }

        if(rs_new->load_scale != h->load_scale) {
          h->load_scale.store(rs_new->load_scale);
          chg = true;
        }

        if(rs_new->rebalance()) {
          h->rebalance(rs_new->rebalance());
          balance_rangers.push_back(h);
        } else {
          h->rebalance(0);
        }

        if(chg || sync_all)
          changed.push_back(h);
        break;
      }

      if(!found) {
        if(rs_new->state == RangerState::ACK) {
          rs_new->init_queue();
          m_rangers.push_back(rs_new);
          changed.push_back(rs_new);
        }
      }
    }

    for(auto& h : m_rangers)
      h->interm_ranges.store(0);
  }

  for(auto h : balance_rangers) {
    Core::Vector<Range::Ptr> ranges;
    Env::Mngr::columns()->assigned(h->rgrid, 1, ranges);
    int err = Error::OK;
    for(auto& range : ranges) {
      auto col = Env::Mngr::columns()->get_column(err, range->cfg->cid);
      if(err || !h->can_rebalance())
        break;
      h->put(Comm::Protocol::Rgr::Req::RangeUnload::Ptr(
        new Comm::Protocol::Rgr::Req::RangeUnload(h, col, range, true)
      ));
      h->interm_ranges.fetch_add(1);
    }
    if(h->rebalance() && !sync_all) {
      if(std::find(changed.cbegin(), changed.cend(), h) == changed.cend())
        changed.push_back(h);
    }
  }

  if(!changed.empty())
    changes(changed, sync_all && !rangers_mngr);
}

void Rangers::range_loaded(Ranger::Ptr rgr, Range::Ptr range,
                           int err, bool failure, bool verbose) {
  bool run_assign = m_assignments.fetch_sub(1) == cfg_assign_due->get();
  if(!range->deleted()) {
    if(err) {

      if(failure) {
        rgr->failures.fetch_add(1);

      } else if(err == Error::SERVER_SHUTTING_DOWN &&
                rgr->state == RangerState::ACK) {
        rgr->state.fetch_or(RangerState::SHUTTINGDOWN);
        RangerList chged;
        chged.push_back(rgr);
        changes(chged, true);
        schedule_check(1000);
      }

      range->set_state(Range::State::NOTSET, 0);
      if(!run_assign)
        schedule_check(2000);
      if(verbose)
        SWC_LOG_OUT(LOG_DEBUG,
          Error::print(SWC_LOG_OSTREAM << "RANGE-STATUS ", err);
          range->print(SWC_LOG_OSTREAM << ", ");
        );

    } else {
      rgr->failures.store(0);
      range->set_state(Range::State::ASSIGNED, rgr->rgrid);
      range->clear_last_rgr();
    }
  }

  if(run_assign)
    assign_ranges();
}


bool Rangers::update(const Column::Ptr& col, const DB::Schema::Ptr& schema,
                     uint64_t req_id, bool ack_required) {
  Core::Vector<rgrid_t> rgrids;
  col->need_schema_sync(schema->revision, rgrids);
  bool undergo = false;
  for(rgrid_t rgrid : rgrids) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto& rgr : m_rangers) {
      if(rgr->failures < cfg_rgr_failures->get() &&
         rgr->state == RangerState::ACK && rgr->rgrid == rgrid) {
        undergo = true;
        if(ack_required) {
          rgr->put(
            Comm::Protocol::Rgr::Req::ColumnUpdate::Ptr(
              new Comm::Protocol::Rgr::Req::ColumnUpdate(
                rgr, col, schema, req_id)
            )
          );
        }
      }
    }
  }
  return undergo;
}

void Rangers::column_delete(const DB::Schema::Ptr& schema, uint64_t req_id,
                            const Core::Vector<rgrid_t>& rgrids) {
  for(rgrid_t rgrid : rgrids) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto& rgr : m_rangers) {
      if(rgrid != rgr->rgrid)
        continue;
      rgr->put(Comm::Protocol::Rgr::Req::ColumnDelete::Ptr(
        new Comm::Protocol::Rgr::Req::ColumnDelete(rgr, schema, req_id)
      ));
    }
  }
}

void Rangers::column_compact(const Column::Ptr& col) {
  Core::Vector<rgrid_t> rgrids;
  col->assigned(rgrids);
  for(rgrid_t rgrid : rgrids) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto& rgr : m_rangers) {
      if(rgr->failures < cfg_rgr_failures->get() &&
         rgr->state == RangerState::ACK && rgr->rgrid == rgrid) {
        rgr->put(Comm::Protocol::Rgr::Req::ColumnCompact::Ptr(
          new Comm::Protocol::Rgr::Req::ColumnCompact(col->cfg->cid)
        ));
      }
    }
  }
}

void Rangers::need_health_check(const Column::Ptr& col) {
  {
    Core::MutexSptd::scope lock(m_mutex_columns_check);
    auto it = std::find_if(
      m_columns_check.cbegin(), m_columns_check.cend(),
      [&col](const ColumnHealthCheck::Ptr chk) { return chk->col == col; });
    if(it != m_columns_check.cend())
      return;
  }
  col->reset_health_check();
  health_check_columns();
}

void Rangers::health_check_finished(const ColumnHealthCheck::Ptr& chk) {
  {
    Core::MutexSptd::scope lock(m_mutex_columns_check);
    auto it = std::find(
      m_columns_check.cbegin(), m_columns_check.cend(), chk);
    if(it != m_columns_check.cend())
      m_columns_check.erase(it);
  }
  health_check_columns();
}

void Rangers::wait_health_check(cid_t cid) {
  for(;;) {
    {
      Core::MutexSptd::scope lock(m_mutex_columns_check);
      auto it = cid == DB::Schema::NO_CID
        ? m_columns_check.cbegin()
        : std::find_if(m_columns_check.cbegin(), m_columns_check.cend(),
            [cid](const ColumnHealthCheck::Ptr chk) {
              return chk->col->cfg->cid == cid; });
      if(it == m_columns_check.cend())
        return;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}

void Rangers::print(std::ostream& out) {
  out << "Rangers:";
  Core::MutexSptd::scope lock(m_mutex);
  for(auto& h : m_rangers)
    h->print(out << "\n ");
}

void Rangers::assign_ranges() {
  if(!Env::Mngr::mngd_columns()->expected_ready())
    return schedule_check(5000);

  if(m_run && !m_assign.running())
    Env::Mngr::post([this]() { assign_ranges_run(); });
}

void Rangers::assign_ranges_run() {
  Column::Ptr col;
  Range::Ptr range;

  for(bool state;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      state = m_rangers.empty() || !m_run;
    }
    if(state) {
      m_assign.stop();
      return schedule_check();
    }

    range = Env::Mngr::columns()->get_next_unassigned(col, state = false);
    if(!range) {
      if(state) // waiting-on-meta-ranges
        schedule_check(2000);
      m_assign.stop();
      break;
    }

    Ranger::Ptr rgr = nullptr;
    next_rgr(range, rgr);
    if(!rgr) {
      m_assign.stop();
      return schedule_check();
    }
    auto schema = Env::Mngr::schemas()->get(col->cfg->cid);
    if(!schema)
      continue;

    range->set_state(Range::State::QUEUED, rgr->rgrid);
    rgr->interm_ranges.fetch_add(1);
    state = m_assignments.add_rslt(1) < cfg_assign_due->get();
    if(!state)
      m_assign.stop();

    rgr->put(Comm::Protocol::Rgr::Req::RangeLoad::Ptr(
      new Comm::Protocol::Rgr::Req::RangeLoad(rgr, col, range, schema)
    ));
    if(!state)
      return;
  }

  schedule_check(cfg_chk_assign->get());
}

void Rangers::next_rgr(const Range::Ptr& range, Ranger::Ptr& rs_set) {
  size_t n_rgrs = 0;
  size_t avg_ranges = 0;
  Ranger::Ptr rgr;

  int err = Error::OK;
  auto last_rgr = range->get_last_rgr(err);
  uint8_t state;
  Core::MutexSptd::scope lock(m_mutex);

  for(auto& rgr : m_rangers) {
    state = rgr->state.load();
    if(state == RangerState::MARKED_OFFLINE) {
      continue;

    } else if(rgr->failures >= cfg_rgr_failures->get()) {
      Env::Mngr::columns()->set_rgr_unassigned(rgr->rgrid);
      rgr->state.store(RangerState::MARKED_OFFLINE);
      RangerList chged;
      chged.push_back(rgr);
      _changes(chged, true);

    } else if(state & RangerState::ACK) {
      if(!last_rgr->endpoints.empty() &&
         !(state & RangerState::SHUTTINGDOWN) &&
         Comm::has_endpoint(rgr->endpoints, last_rgr->endpoints)) {
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
  size_t interm_ranges = UINT64_MAX;
  for(auto& rgr : m_rangers) {
    state = rgr->state.load();
    if(state & RangerState::ACK &&
       avg_ranges >= rgr->interm_ranges &&
       interm_ranges >= rgr->interm_ranges &&
       rgr->load_scale >= best &&
       (!(state & RangerState::SHUTTINGDOWN) ||
        (n_rgrs == 1 && !DB::Types::SystemColumn::is_data(range->cfg->cid)) )) {
      best = rgr->load_scale;
      interm_ranges = rgr->interm_ranges;
      rs_set = rgr;
    }
  }
}

void Rangers::health_check_columns() {
  bool support;
  if(!m_run ||
     !Env::Mngr::mngd_columns()->expected_ready() ||
     !m_mutex_columns_check.try_full_lock(support))
    return;
  int64_t ts = Time::now_ms();
  if(ts - m_columns_check_ts < cfg_column_health_chkers_delay->get()) {
    schedule_check(cfg_column_health_chkers_delay->get());
  } else {
    m_columns_check_ts = ts;
    uint32_t intval = cfg_column_health_chk->get();
    for(Column::Ptr col;
        m_columns_check.size() < size_t(cfg_column_health_chkers->get()) &&
        (col = Env::Mngr::columns()->get_need_health_check(ts, intval)); ) {
      Env::Mngr::post(
        [chk=m_columns_check.emplace_back(
          new ColumnHealthCheck(col, ts, intval))]() { chk->run(); });
    }
  }
  m_mutex_columns_check.unlock(support);
}


Ranger::Ptr Rangers::rgr_set(const Comm::EndPoints& endpoints,
                             rgrid_t opt_rgrid) {
  for(auto it=m_rangers.cbegin();it != m_rangers.cend(); ++it) {
    auto h = *it;
    if(Comm::has_endpoint(h->endpoints, endpoints)) {
      if(h->state & RangerState::ACK) {
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
    Core::MutexSptd::scope lock(m_mutex);
    _changes(hosts, sync_all);
  }

  if(Env::Mngr::mngd_columns()->has_active())
    schedule_check(cfg_delay_rgr_chg->get());
}

void Rangers::_changes(const RangerList& hosts, bool sync_all) {
  if(hosts.empty())
    return;

  // batches of 1000 hosts a req
  for(auto it = hosts.cbegin(); it != hosts.cend(); ) {
    auto from = it;
    size_t n = hosts.cend() - from;
    it += n > 1000 ? 1000 : n;
    Env::Mngr::role()->req_mngr_inchain(
      Comm::Protocol::Mngr::Req::RgrUpdate::Ptr(
        new Comm::Protocol::Mngr::Req::RgrUpdate(
          RangerList(from, it),
          sync_all
        )
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
