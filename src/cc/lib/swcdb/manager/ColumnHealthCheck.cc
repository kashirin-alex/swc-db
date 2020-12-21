/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 



#include "swcdb/manager/ColumnHealthCheck.h"
#include "swcdb/manager/Protocol/Rgr/req/RangeIsLoaded.h"


namespace SWC { namespace Manager {


ColumnHealthCheck::RangerCheck::RangerCheck(
                const ColumnHealthCheck::Ptr& col_checker, 
                const Ranger::Ptr& rgr)
                : col_checker(col_checker), rgr(rgr), m_checkings(0),
                  m_success(0), m_failures(0) { 
}

ColumnHealthCheck::RangerCheck::~RangerCheck() { 
  if(!m_success && m_failures) // && m_success < m_failures)
    rgr->failures.fetch_add(1);
}

void ColumnHealthCheck::RangerCheck::add_range(const Range::Ptr& range) {
  Core::MutexSptd::scope lock(m_mutex);
  _add_range(range);
}
    
bool ColumnHealthCheck::RangerCheck::add_ranges(uint8_t more) {
  if(rgr->state == DB::Types::MngrRangerState::ACK) {
    std::vector<Range::Ptr> ranges;
    col_checker->col->need_health_check(
      col_checker->check_ts, col_checker->check_intval, 
      ranges, rgr->rgrid, more
    );
    if(!ranges.empty()) {
      for(auto& range : ranges)
        add_range(range);
      return true;
    }
  }
  return false;
}

void ColumnHealthCheck::RangerCheck::handle(const Range::Ptr& range, int err) {
  uint8_t more;
  {
    Core::MutexSptd::scope lock(m_mutex);
    --m_checkings;
    while(!m_ranges.empty() && m_checkings < 10) {
      auto r = m_ranges.front();
      if(r->assigned() && r->get_rgr_id() == rgr->rgrid)
        _add_range(r);
      m_ranges.pop();
    }
    size_t sz = m_checkings + m_ranges.size();
    more = sz < 10 ? 10 - sz  : 0;
  }

  if(err == Error::COMM_CONNECT_ERROR) {
    m_failures.fetch_add(1);
  } else {
    m_success.fetch_add(1);
    if(!err) {
      rgr->failures.store(0);
    } else if(err == Error::RGR_NOT_LOADED_RANGE) {
      col_checker->col->set_unloaded(range);
      Env::Mngr::rangers()->schedule_check(2000);
    }
  }

  SWC_LOGF((err ? LOG_WARN : LOG_DEBUG),
    "Column-Health FINISH range(%lu/%lu) rgr=%lu err=%d(%s)",
    range->cfg->cid, range->rid, rgr->rgrid.load(),
    err, Error::get_text(err));

  if(more)
    add_ranges(more);
  col_checker->finishing(true);
}

void ColumnHealthCheck::RangerCheck::_add_range(const Range::Ptr& range) {
  if(m_checkings == 10) {
    m_ranges.push(range);
  } else {
    col_checker->completion.increment();
    ++m_checkings;
    rgr->put(
      std::make_shared<Comm::Protocol::Rgr::Req::RangeIsLoaded>(
        shared_from_this(), range));
    SWC_LOGF(LOG_DEBUG, "Column-Health START range(%lu/%lu) rgr=%lu",
             range->cfg->cid, range->rid, rgr->rgrid.load());
  }
}
  


  
ColumnHealthCheck::ColumnHealthCheck(const Column::Ptr& col, 
                                     int64_t check_ts, uint32_t check_intval)
                                    : col(col), check_ts(check_ts), 
                                      check_intval(check_intval),
                                      completion(1) {
  SWC_LOGF(LOG_DEBUG, "Column-Health START cid(%lu)", col->cfg->cid);
}
  
ColumnHealthCheck::~ColumnHealthCheck() { }

void ColumnHealthCheck::run(bool initial) {
  if(m_check.running())
    return;

  std::vector<Range::Ptr> ranges;
  col->need_health_check(check_ts, check_intval, ranges, 0, 100);

  RangerCheck::Ptr checker;
  rgrid_t rgrid;
  for(auto& range : ranges) {
    if(!range->assigned())
      continue;
    rgrid = range->get_rgr_id();
    {
      Core::MutexSptd::scope lock(m_mutex);
      auto it = std::find_if(
        m_checkers.begin(), m_checkers.end(), 
        [rgrid](const RangerCheck::Ptr& checker) { 
          return rgrid == checker->rgr->rgrid; }
      );
      checker = it == m_checkers.end() ? nullptr : *it;
    }
    if(!checker) {
      auto rgr = Env::Mngr::rangers()->rgr_get(rgrid);
      if(!rgr || rgr->state != DB::Types::MngrRangerState::ACK)
        continue;

      Core::MutexSptd::scope lock(m_mutex);
      checker = m_checkers.emplace_back(
        new RangerCheck(shared_from_this(), rgr));

    } else if(checker->rgr->state != DB::Types::MngrRangerState::ACK) {
      continue;
    }

    checker->add_range(range);
  }

  m_check.stop();

  if(initial)
    finishing(false);
}

void ColumnHealthCheck::finishing(bool finished_range) {
  if(finished_range)
    run(false);

  SWC_LOGF(LOG_DEBUG, "Column-Health finishing cid(%lu) in-process=%lu", 
                        col->cfg->cid, completion.count());
  if(!completion.is_last())
    return;
  /*
  if(col->state() == Error::OK) {
    // match ranger's rids to mngr assignments (dup. unload from all Rangers)
    RangerList rangers;
    Env::Mngr::rangers()->rgr_get(0, rangers);
  }
  */
  SWC_LOGF(LOG_DEBUG, "Column-Health FINISH cid(%lu)", col->cfg->cid);
  Env::Mngr::rangers()->health_check_finished(shared_from_this());
}



}}
