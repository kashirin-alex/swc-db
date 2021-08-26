/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_db_Column_h
#define swcdb_manager_db_Column_h


#include "swcdb/manager/db/Range.h"
#include "swcdb/db/Types/MngrColumnState.h"


namespace SWC { namespace Manager {


class Column final : private Core::Vector<Range::Ptr> {

  public:

  using State = DB::Types::MngrColumn::State;

  typedef std::shared_ptr<Column> Ptr;

  static bool create(int &err, const cid_t cid) {
    Env::FsInterface::interface()->mkdirs(err, DB::RangeBase::get_path(cid));
    return true;
  }

  static bool remove(int &err, const cid_t cid) {
    Env::FsInterface::interface()->rmdir_incl_opt_subs(
      err,
      DB::RangeBase::get_column_path(cid),
      DB::RangeBase::get_column_path()
    );
    return true;
  }

  const ColumnCfg::Ptr cfg;

  SWC_CAN_INLINE
  Column(const DB::Schema::Ptr& schema)
        : cfg(new ColumnCfg(schema)),
          m_state(State::LOADING), m_check_ts(0) {
  }

  //~Column() { }

  void init(int &err) {
    FS::IdEntries_t entries;
    Env::FsInterface::interface()->get_structured_ids(
      err, DB::RangeBase::get_path(cfg->cid), entries);
    if(err)
      return;
    if(entries.empty()) {
      SWC_LOGF(LOG_INFO, "Init. New Column(%lu) Range(1)", cfg->cid);
      entries.push_back(1);
    }
    for(auto rid : entries)
      get_range(rid, true);
  }

  void set_loading() {
    Core::ScopedLock lock(m_mutex);
    _set_loading();
  }

  void set_unloaded(const Range::Ptr& range) {
    Core::ScopedLock lock(m_mutex);
    range->set_state_none();
    _set_loading();
  }

  bool set_merging(const Range::Ptr& range) {
    Core::ScopedLock lock(m_mutex);
    if(m_state == State::DELETED)
      return false;
    range->set_state(Range::State::MERGE, 0);
    m_state.store(State::LOADING);
    return true;
  }

  SWC_CAN_INLINE
  State state() {
    Core::SharedLock lock(m_mutex);
    return m_state;
  }

  SWC_CAN_INLINE
  void state(int& err) {
    Core::SharedLock lock(m_mutex);
    if(m_state == State::OK)
      return;
    if(m_state == State::DELETED)
      err = Error::COLUMN_MARKED_REMOVED;
    else
      err = Error::COLUMN_NOT_READY;
  }

  size_t ranges() {
    Core::SharedLock lock(m_mutex);
    return size();
  }

  void assigned(rgrid_t rgrid, size_t& num,
                Core::Vector<Range::Ptr>& ranges) {
    Core::SharedLock lock(m_mutex);
    for(auto& range : *this) {
      if(range->assigned(rgrid)) {
        ranges.push_back(range);
        if(!-num)
          break;
      }
    }
  }

  void get_ranges(Core::Vector<Range::Ptr>& ranges) {
    Core::SharedLock lock(m_mutex);
    ranges.assign(cbegin(), cend());
  }

  Range::Ptr get_range(const rid_t rid, bool initialize=false) {
    Core::ScopedLock lock(m_mutex);

    for(auto& range : *this) {
      if(range->rid == rid)
        return range;
    }
    return initialize ? emplace_back(new Range(cfg, rid)) : nullptr;
  }

  Range::Ptr get_range(int&,
                       const DB::Cell::Key& range_begin,
                       const DB::Cell::Key& range_end,
                       bool next_range) {
    bool found = false;
    Core::SharedLock lock(m_mutex);
    for(auto& range : *this) {
      if(!range->includes(range_begin, range_end))
        continue;
      if(!next_range)
        return range;
      if(!found) {
        found = true;
        continue;
      }
      return range;
    }
    return nullptr;
  }

  void sort(Range::Ptr& range,
            const DB::Cells::Interval& interval, int64_t revision) {
    Core::ScopedLock lock(m_mutex);

    // std::sort(begin(), end(), Range);
    if(!range->equal(interval)) {
      range->set(interval, revision);

      if(size() > 1) {
        for(auto it = cbegin(); it != cend(); ++it) {
          if(range->rid == (*it)->rid) {
            erase(it);
            break;
          }
        }
        bool added = false;
        for(auto it=cbegin(); it != cend(); ++it) {
          if((*it)->after(range)) {
            insert(it, range);
            added = true;
            break;
          }
        }
        if(!added)
          push_back(range);
      }
    }

    apply_loaded_state();
  }

  Range::Ptr left_sibling(const Range::Ptr& right) {
    Core::SharedLock lock(m_mutex);

    if(!empty()) for(auto it = cbegin() + 1; it != cend(); ++it) {
      if((*it)->rid == right->rid)
        return *--it;
    }
    return nullptr;
  }

  Range::Ptr create_new_range(rgrid_t rgrid) {
    Core::ScopedLock lock(m_mutex);

    auto& range = emplace_back(new Range(cfg, _get_next_rid()));
    // if !rid - err reached id limit
    range->set_state(Range::State::CREATED, rgrid);
    return range;
  }

  rid_t get_next_rid() {
    Core::SharedLock lock(m_mutex);
    return _get_next_rid();
  }

  Range::Ptr get_next_unassigned() {
    Core::ScopedLock lock(m_mutex);

    for(auto& range : *this) {
      if(range->need_assign()) {
        _set_loading();
        return range;
      }
    }
    return nullptr;
  }

  bool set_rgr_unassigned(rgrid_t rgrid) {
    bool found = false;
    Core::ScopedLock lock(m_mutex);

    for(auto& range : *this) {
      if(range->get_rgr_id() == rgrid) {
        range->set_state_none();
        _set_loading();
        found = true;
      }
    }
    _remove_rgr_schema(rgrid);
    return found;
  }

  bool change_rgr(rgrid_t rgrid_old, rgrid_t rgrid) {
    bool found = false;
    Core::ScopedLock lock(m_mutex);

    for(auto& range : *this) {
      if(range->get_rgr_id() == rgrid_old) {
        range->set_rgr_id(rgrid);
        found = true;
      }
    }
    auto it = m_schemas_rev.find(rgrid_old);
    if(it != m_schemas_rev.cend()) {
      int64_t rev = it->second;
      m_schemas_rev.erase(it);
      m_schemas_rev[rgrid] = rev;
    }
    return found;
  }

  void change_rgr_schema(const rgrid_t rgrid, int64_t rev=0) {
    Core::ScopedLock lock(m_mutex);
    m_schemas_rev[rgrid] = rev;
  }

  void remove_rgr_schema(const rgrid_t rgrid) {
    Core::ScopedLock lock(m_mutex);
    _remove_rgr_schema(rgrid);
  }

  void need_schema_sync(int64_t rev, Core::Vector<rgrid_t> &rgrids) {
    Core::SharedLock lock(m_mutex);

    for(auto it = m_schemas_rev.cbegin(); it != m_schemas_rev.cend(); ++it) {
      if(it->second != rev)
        rgrids.push_back(it->first);
    }
  }

  bool need_schema_sync(const rgrid_t rgrid, int64_t rev) {
    Core::SharedLock lock(m_mutex);

    auto it = m_schemas_rev.find(rgrid);
    if(it != m_schemas_rev.cend())
      return rev != it->second;
    return true;
  }

  void assigned(Core::Vector<rgrid_t> &rgrids) {
    Core::SharedLock lock(m_mutex);

    rgrid_t rgrid;
    for(auto& range : *this) {
      if(!(rgrid = range->get_rgr_id()))
        continue;
      if(std::find(rgrids.cbegin(), rgrids.cend(), rgrid) == rgrids.cend())
        rgrids.push_back(rgrid);
    }
  }

  void reset_health_check() {
    Core::SharedLock lock(m_mutex);
    m_check_ts = 0;
  }

  bool need_health_check(int64_t ts, uint32_t ms) {
    Core::SharedLock lock(m_mutex);
    if(m_state == State::OK && m_check_ts + ms < ts) {
      m_check_ts = ts;
      return true;
    }
    return false;
  }

  void need_health_check(int64_t ts, uint32_t ms,
                         Core::Vector<Range::Ptr> &ranges,
                         rgrid_t rgrid = 0, size_t max = 0) {
    Core::SharedLock lock(m_mutex);
    for(auto& range : *this) {
      if(range->need_health_check(ts, ms, rgrid)) {
        ranges.push_back(range);
        if(max && ranges.size() == max)
          return;
      }
    }
  }

  void remove_range(rid_t rid) {
    Core::ScopedLock lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it) {
      if(rid == (*it)->rid) {
        (*it)->set_deleted();
        erase(it);
        apply_loaded_state();
        break;
      }
    }
  }

  bool do_remove() {
    Core::ScopedLock lock(m_mutex);

    bool was = m_state.exchange(State::DELETED) == State::DELETED;
    m_schemas_rev.clear();

    if(!was) {
      for(auto& range : *this)
        range->set_deleted();
    }
    return !was;
  }

  bool finalize_remove(int &err, rgrid_t rgrid=0) {
    Core::ScopedLock lock(m_mutex);

    if(!rgrid) {
      clear();
    } else {
      rgrid_t eid;
      for(auto it=cbegin(); it != cend(); ) {
        if((eid = (*it)->get_rgr_id()) == rgrid || !eid)
          erase(it);
        else
          ++it;
      }
    }
    if(empty()) {
      Env::FsInterface::interface()->rmdir(
        err, DB::RangeBase::get_path(cfg->cid));
      SWC_LOG_OUT(LOG_DEBUG, _print(SWC_LOG_OSTREAM << "FINALIZED REMOVE "); );
      return true;
    }
    return false;
  }

  void print(std::ostream& out) {
    Core::SharedLock lock(m_mutex);
    _print(out);
  }

  private:

  void _print(std::ostream& out) {
    cfg->print(out << '(');
    out << " next-rid=" << _get_next_rid()
        << " ranges=[";
    for(auto& range : *this) {
      range->print(out);
      out << ',';
    }
    out << "])";
  }

  SWC_CAN_INLINE
  void _set_loading() {
    State at(State::OK);
    if(m_state.compare_exchange_weak(at, State::LOADING))
      m_check_ts = 0;
  }

  rid_t _get_next_rid() {
    rid_t rid = 0;
    for(bool unused; ++rid; ) {
      unused = true;
      for(auto& range : *this) {
        if(range->rid == rid) {
          unused = false;
          break;
        }
      }
      if(unused)
        break;
    }
    return rid;
  }

  void _remove_rgr_schema(const rgrid_t rgrid) {
    auto it = m_schemas_rev.find(rgrid);
    if(it != m_schemas_rev.cend())
       m_schemas_rev.erase(it);
  }

  void apply_loaded_state() {
    if(m_state != State::LOADING)
      return;

    for(auto& range : *this) {
      if(!range->assigned())
        return;
    }
    if(m_state == State::LOADING) {
      /* if Ranger do not select/check ranges on rid value match
         once on start, Master & Meta column check rid consistency
         on dup. cell of rid, delete earliest
      */
      m_state.store(State::OK);
      m_check_ts = Time::now_ms();
    }
  }

  std::shared_mutex         m_mutex;
  Core::Atomic<State>       m_state;
  int64_t                   m_check_ts;

  std::unordered_map<rgrid_t, int64_t>   m_schemas_rev;

};

}}

#endif // swcdb_manager_db_Column_h