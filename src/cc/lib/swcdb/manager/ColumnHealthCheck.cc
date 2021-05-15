/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/manager/ColumnHealthCheck.h"
#include "swcdb/manager/Protocol/Rgr/req/RangeIsLoaded.h"
#include "swcdb/manager/Protocol/Rgr/req/RangeUnoadForMerge.h"
#include "swcdb/db/client/Query/Select/Handlers/Common.h"
#include "swcdb/db/Cells/CellValueSerialFields.h"


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

void ColumnHealthCheck::RangerCheck::handle(const Range::Ptr& range,
                                            int err, uint8_t flags) {
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
      if(flags & Comm::Protocol::Rgr::Params::RangeIsLoadedRsp::CAN_MERGE) {
        col_checker->add_mergeable(range);
      }
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

void ColumnHealthCheck::add_mergeable(const Range::Ptr& range) {
  SWC_LOG_OUT(LOG_DEBUG,
    range->print(SWC_LOG_OSTREAM << "RANGE-CAN-MERGE: ");
  );
  Core::MutexSptd::scope lock(m_mutex);
  m_mergeable_ranges.push_back(range);
}

void ColumnHealthCheck::finishing(bool finished_range) {
  if(finished_range)
    run(false);

  SWC_LOGF(LOG_DEBUG, "Column-Health finishing cid(%lu) in-process=%lu",
                        col->cfg->cid, completion.count());
  if(!completion.is_last())
    return;

  m_checkers.clear();

  if(m_mergeable_ranges.empty()) {
    /*
    if(col->state() == Error::OK) {
      // match ranger's rids to mngr assignments (dup. unload from all Rangers)
      RangerList rangers;
      Env::Mngr::rangers()->rgr_get(0, rangers);
    }
    */
    SWC_LOGF(LOG_DEBUG, "Column-Health FINISH cid(%lu)", col->cfg->cid);
    Env::Mngr::rangers()->health_check_finished(shared_from_this());
    return;
  }

  auto merger = std::make_shared<ColumnMerger>(
    shared_from_this(), std::move(m_mergeable_ranges));
  if(DB::Types::SystemColumn::is_master(col->cfg->cid)) {
    return merger->run_master();
  }

  cid_t meta_cid = DB::Types::SystemColumn::get_sys_cid(
    col->cfg->key_seq, DB::Types::SystemColumn::get_range_type(col->cfg->cid));
  DB::Specs::Interval spec(DB::Types::Column::SERIAL);
  auto& key_intval = spec.key_intervals.add();
  key_intval.start.reserve(2);
  key_intval.start.add(std::to_string(col->cfg->cid), Condition::EQ);
  key_intval.start.add("", Condition::GE);

  auto hdlr = client::Query::Select::Handlers::Common::make(
    Env::Clients::get(),
    [merger, meta_cid]
    (const client::Query::Select::Handlers::Common::Ptr& hdlr) {
      int err = hdlr->state_error;
      if(!err) {
        auto col = hdlr->get_columnn(meta_cid);
        if(!(err = col->error()) && !col->empty())
          col->get_cells(merger->cells);
      }
      (err || merger->cells.empty())
        ? merger->completion()
        : merger->run();
    },
    false,
    Env::Mngr::io()
  );
  client::Query::Select::scan(
    hdlr, col->cfg->key_seq, meta_cid, std::move(spec));
}



ColumnHealthCheck::ColumnMerger::ColumnMerger(
            const ColumnHealthCheck::Ptr& col_checker,
            std::vector<Range::Ptr>&& ranges) noexcept
            : col_checker(col_checker), m_ranges(std::move(ranges)) {
}

void ColumnHealthCheck::ColumnMerger::run_master() {
  std::vector<Range::Ptr> sorted;
  for(auto& range : m_ranges) {
    bool added = false;
    for(auto it=sorted.begin(); it != sorted.end(); ++it) {
      if((*it)->after(range)) {
        sorted.insert(it, range);
        added = true;
        break;
      }
    }
    if(!added)
      sorted.push_back(range);
  }

  std::vector<Range::Ptr> group;
  for(auto& range : sorted) {
    auto left = col_checker->col->left_sibling(range);
    if(!left) {
      group.push_back(range);
    } else if(group.empty()) {
      group.push_back(left); // group-merge-to-this-range
      group.push_back(range);
    } else if(left->rid == group.back()->rid) {
      group.push_back(range);
    } else {
      if(group.size() > 1)
        m_mergers.emplace_back(
          new RangesMerger(shared_from_this(), std::move(group)));
      group.push_back(left); // group-merge-to-this-range
      group.push_back(range);
    }
  }
  if(group.size() > 1)
    m_mergers.emplace_back(
      new RangesMerger(shared_from_this(), std::move(group)));

  if(m_mergers.empty()) {
    SWC_LOGF(LOG_WARN, "Column-Health FINISH cid(%lu) not-mergeable=%lu",
             col_checker->col->cfg->cid, m_ranges.size());
    Env::Mngr::rangers()->health_check_finished(col_checker);
  } else {
    SWC_LOGF(LOG_DEBUG, "Column-Health MERGE cid(%lu) merger-groups=%lu",
             col_checker->col->cfg->cid, m_mergers.size());
    completion();
  }
}

void ColumnHealthCheck::ColumnMerger::run() {
  std::vector<Range::Ptr> group;
  Range::Ptr left = nullptr;
  for(auto& cell : cells) {
    //SWC_LOG_OUT(LOG_DEBUG,
    //    cell->print(SWC_LOG_OSTREAM, DB::Types::Column::SERIAL); );

    StaticBuffer v;
    cell->get_value(v);
    const uint8_t* ptr = v.base;
    size_t remain = v.size;
    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    DB::Cell::Key key_end; // skip-through
    key_end.decode(&ptr, &remain, false);
    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    rid_t rid = Serialization::decode_vi64(&ptr, &remain);

    Range::Ptr range = col_checker->col->get_range(rid, false);
    if(!range) {
      m_mergers.clear();
      break;
    }
    if(!left) {
      group.push_back(range);
    } else if(group.empty()) {
      group.push_back(left); // group-merge-to-this-range
      group.push_back(range);
    } else if(left->rid == group.back()->rid) {
      group.push_back(range);
    } else {
      if(group.size() > 1)
        m_mergers.emplace_back(
          new RangesMerger(shared_from_this(), std::move(group)));
      group.push_back(left); // group-merge-to-this-range
      group.push_back(range);
    }
    left = range;
  }
  if(group.size() > 1)
    m_mergers.emplace_back(
      new RangesMerger(shared_from_this(), std::move(group)));

  if(m_mergers.empty()) {
    SWC_LOGF(LOG_WARN, "Column-Health FINISH cid(%lu) not-mergeable=%lu",
             col_checker->col->cfg->cid, m_ranges.size());
    Env::Mngr::rangers()->health_check_finished(col_checker);
  } else {
    SWC_LOGF(LOG_DEBUG, "Column-Health MERGE cid(%lu) merger-groups=%lu",
             col_checker->col->cfg->cid, m_mergers.size());
    completion();
  }
}

void ColumnHealthCheck::ColumnMerger::completion() {
  if(m_mergers.empty()) {
    SWC_LOGF(LOG_DEBUG, "Column-Health FINISH cid(%lu) MERGE",
             col_checker->col->cfg->cid);
    Env::Mngr::rangers()->health_check_finished(col_checker);
    return;
  }
  auto merger = m_mergers.back();
  m_mergers.erase(m_mergers.end() - 1);
  merger->run();
}



ColumnHealthCheck::ColumnMerger::RangesMerger::RangesMerger(
                const ColumnMerger::Ptr& col_merger,
                std::vector<Range::Ptr>&& ranges) noexcept
      : col_merger(col_merger), m_err(Error::OK),
        m_ranges(std::move(ranges)) {
}

void ColumnHealthCheck::ColumnMerger::RangesMerger::run() {
  for(auto& range : m_ranges) {
    auto rgrid = range->get_rgr_id();
    if(rgrid) {
      auto rgr = Env::Mngr::rangers()->rgr_get(rgrid);
      if(rgr) {
        rgr->put(
          std::make_shared<Comm::Protocol::Rgr::Req::RangeUnoadForMerge>(
             rgr, shared_from_this(), range));
          SWC_LOGF(LOG_DEBUG,
            "Column-Health MERGE-UNLOAD range(%lu/%lu) rgr=%lu",
            range->cfg->cid, range->rid, rgr->rgrid.load());
        continue;
      }
    }
    handle(range, Error::OK, false);
  }
}

void ColumnHealthCheck::ColumnMerger::RangesMerger::handle(
                                const Range::Ptr& range, int err,
                                bool empty) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(err) {
      if(!m_err)
        m_err = err;
      m_ready.push_back(nullptr);
    } else if(!empty && m_ranges.front() != range) {
      SWC_LOGF(LOG_WARN, "Column-Health MERGE-UNLOAD range(%lu/%lu)"
                         " NOT-EMPTY cancelling-merge",
                          range->cfg->cid, range->rid);
      m_ready.push_back(range);
      if(!m_err)
        m_err = Error::CANCELLED;
    } else if(col_merger->col_checker->col->set_merging(range)) {
      m_ready.push_back(range);
    } else {
      m_ready.push_back(nullptr);
      if(!m_err)
        m_err = Error::COLUMN_MARKED_REMOVED;
    }
    if(m_ranges.size() > m_ready.size())
      return;
  }

  if(m_err) { // CANCEL-MERGE
    col_merger->col_checker->col->state(m_err=Error::OK);
    if(m_err != Error::COLUMN_MARKED_REMOVED) {
      for(auto& range : m_ready) {
        if(range) // reset merge state
          col_merger->col_checker->col->set_unloaded(range);
      }
      Env::Mngr::rangers()->schedule_check(2000);
    }
    return col_merger->completion();
  }
  m_ready.clear();

  Range::Ptr main_range = m_ranges.front(); //group-merge-to-this-range
  m_ranges.erase(m_ranges.begin());

  const std::string main_range_path =
    DB::RangeBase::get_path(
      main_range->cfg->cid, main_range->rid);
  const std::string main_cs_path =
    DB::RangeBase::get_path_on_range(
      main_range_path, DB::RangeBase::CELLSTORES_DIR);

  std::vector<Range::Ptr> merged;
  csid_t last_cs_id;

  const auto& fs = Env::FsInterface::interface();
  FS::DirentList files;

  // get last used cs-id
  fs->readdir(err, main_cs_path, files);
  if(err)
    goto finalize;

  {
    FS::IdEntries_t entries;
    entries.reserve(files.size());
    for(auto& entry : files) {
      if(entry.name.find(".cs", entry.name.length()-3) != std::string::npos) {
        auto idn = entry.name.substr(0, entry.name.length()-3);
        entries.push_back(strtoll(idn.c_str(), nullptr, 0));
      }
    }
    std::sort(entries.begin(), entries.end());
    if(entries.empty()) {
      err = Error::CANCELLED;
      goto finalize;
    }
    last_cs_id = entries.back();
  }

  for(auto& range : m_ranges) {
    // sanity check if no-logs exist
    const std::string range_path = DB::RangeBase::get_path(
      range->cfg->cid, range->rid);
    files.clear();
    fs->readdir(
      err,
      DB::RangeBase::get_path_on_range(range_path, DB::RangeBase::LOG_DIR),
      files);
    if(err || !files.empty()) // sanity-check
      goto finalize;

    // read cs files to move/rename (expect one)
    const std::string cs_path = DB::RangeBase::get_path_on_range(
      range_path, DB::RangeBase::CELLSTORES_DIR);
    fs->readdir(err, cs_path, files);
    if(err || files.size() != 1 || !files.front().name.ends_with(".cs"))
      goto finalize;

    // rename to main-range cs-path
    const std::string to_cs = DB::RangeBase::get_path_cs(
      main_range_path, DB::RangeBase::CELLSTORES_DIR, ++last_cs_id);
    fs->rename(err, cs_path + files.front().name, to_cs);
    if(err)
      goto finalize;

    // delete the R/rid folder
    fs->rmdir(err, range_path);
    merged.push_back(range);
    range = nullptr;
  }


  finalize:

  for(auto& range : merged)
    col_merger->col_checker->col->remove_range(range->rid);

  if(!merged.empty() &&
     !DB::Types::SystemColumn::is_master(main_range->cfg->cid)) {
    auto hdlr = client::Query::Update::Handlers::Common::make(
      Env::Clients::get());
    cid_t meta_cid = DB::Types::SystemColumn::get_sys_cid(
      main_range->cfg->key_seq,
      DB::Types::SystemColumn::get_range_type(main_range->cfg->cid));
    auto& col = hdlr->create(
      meta_cid, main_range->cfg->key_seq, 1, 0, DB::Types::Column::SERIAL);
    for(auto& cell : col_merger->cells) {
      StaticBuffer v;
      cell->get_value(v);
      const uint8_t* ptr = v.base;
      size_t remain = v.size;
      DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
      DB::Cell::Key key_end; // skip-through
      key_end.decode(&ptr, &remain, false);
      DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
      rid_t rid = Serialization::decode_vi64(&ptr, &remain);

      for(auto& range : merged) {
        if(rid == range->rid) {
          cell->flag = DB::Cells::DELETE;
          cell->free(); // no-need-value-data
          col->add(*cell);
          break;
        }
      }
    }
    hdlr->commit_if_need();
    hdlr->wait();
  }


  for(auto& range : m_ranges) {
    if(range) // reset unmerged state
      col_merger->col_checker->col->set_unloaded(range);
  }

  if(!merged.empty()) // delete expired/outdated range.data file
    fs->remove(err, DB::RangeBase::get_path_range_data(main_range_path));

  col_merger->col_checker->col->set_unloaded(main_range);
  Env::Mngr::rangers()->schedule_check(2000);

  SWC_LOGF(LOG_INFO,
    "Column-Health MERGE GROUP cid(%lu) ranges(%lu/%lu) to range(%lu)",
    main_range->cfg->cid, merged.size(), m_ranges.size(), main_range->rid);
  return col_merger->completion();
}


}}


#include "swcdb/manager/Protocol/Rgr/req/RangeUnoadForMerge.cc"
