/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_db_Range_h
#define swcdb_manager_db_Range_h


#include "swcdb/db/Types/MngrRangeState.h"
#include "swcdb/common/Files/RgrData.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Cells/Interval.h"

#include "swcdb/db/Columns/RangeBase.h"


namespace SWC { namespace Manager {

class Range final {
  public:

  typedef std::shared_ptr<Range> Ptr;

  using State = DB::Types::MngrRange::State;

  const ColumnCfg::Ptr  cfg;
  const rid_t           rid;

  SWC_CAN_INLINE
  Range(const ColumnCfg::Ptr& cfg, const rid_t rid)
        : cfg(cfg), rid(rid),
          m_path(DB::RangeBase::get_path(cfg->cid, rid)),
          m_state(State::NOTSET), m_check_ts(0),
          m_rgrid(0), m_last_rgr(nullptr), m_load_revision(0) {
  }

  void init(int&) { }

  //~Range() { }

  SWC_CAN_INLINE
  State state() {
    Core::SharedLock lock(m_mutex);
    return m_state;
  }

  SWC_CAN_INLINE
  bool deleted() {
    Core::SharedLock lock(m_mutex);
    return m_state == State::DELETED;
  }

  SWC_CAN_INLINE
  bool assigned() {
    Core::SharedLock lock(m_mutex);
    return m_state == State::ASSIGNED;
  }

  SWC_CAN_INLINE
  bool queued() {
    Core::SharedLock lock(m_mutex);
    return m_state == State::QUEUED;
  }

  SWC_CAN_INLINE
  bool need_assign() {
    Core::SharedLock lock(m_mutex);
    return m_state == State::NOTSET;
  }

  SWC_CAN_INLINE
  bool assigned(rgrid_t rgrid) {
    Core::SharedLock lock(m_mutex);
    return m_state == State::ASSIGNED && m_rgrid == rgrid;
  }

  bool need_health_check(int64_t ts, uint32_t ms, rgrid_t rgrid) {
    Core::SharedLock lock(m_mutex);
    if((m_state == State::ASSIGNED || m_state == State::QUEUED) &&
       (!rgrid || m_rgrid == rgrid) && m_check_ts + ms < ts) {
      m_check_ts = ts;
      return true;
    }
    return false;
  }

  void set_state(State new_state, rgrid_t rgrid) {
    Core::ScopedLock lock(m_mutex);
    m_state = new_state;
    m_rgrid = rgrid;
    m_load_revision = 0;
    m_check_ts = 0;
  }

  void set_state_none() {
    set_state(State::NOTSET, 0);
  }

  void set_state_queued(rgrid_t rgrid) {
    int64_t ts = Time::now_ms();
    Core::ScopedLock lock(m_mutex);
    m_state = State::QUEUED;
    m_rgrid = rgrid;
    m_load_revision = 0;
    m_check_ts = ts;
  }

  void set_state_assigned(rgrid_t rgrid, int64_t revision) {
    int64_t ts = Time::now_ms();
    Core::ScopedLock lock(m_mutex);
    m_state = State::ASSIGNED;
    m_rgrid = rgrid;
    m_load_revision = revision;
    m_check_ts = ts;
    m_last_rgr = nullptr;
  }

  void set_deleted() {
    Core::ScopedLock lock(m_mutex);
    m_state = State::DELETED;
  }

  rgrid_t get_rgr_id() {
    Core::SharedLock lock(m_mutex);
    return m_rgrid;
  }

  SWC_CAN_INLINE
  void set_rgr_id(rgrid_t rgrid) {
    Core::ScopedLock lock(m_mutex);
    m_rgrid = rgrid;
  }

  SWC_CAN_INLINE
  const DB::RgrData& get_last_rgr() {
    Core::ScopedLock lock(m_mutex);
    if(!m_last_rgr) {
      m_last_rgr.reset(new DB::RgrData());
      DB::Types::SystemColumn::is_rgr_data_on_fs(cfg->cid)
        ? Common::Files::RgrData::get_rgr(
            *m_last_rgr.get(),
            DB::RangeBase::get_path_ranger(m_path)
          )
        : DB::RgrData::get_rgr(*m_last_rgr.get(), cfg->cid, rid);
    }
    return *m_last_rgr.get();
  }

  void set(const DB::Cells::Interval& intval, int64_t revision) {
    Core::ScopedLock lock(m_mutex);
    m_key_begin.copy(intval.key_begin);
    m_key_end.copy(intval.key_end);
    m_load_revision = revision;
  }

  void get_interval(DB::Cell::Key& key_begin, DB::Cell::Key& key_end,
                    int64_t& revision) {
    Core::SharedLock lock(m_mutex);
    key_begin.copy(m_key_begin);
    key_end.copy(m_key_end);
    revision = m_load_revision;
  }

  bool equal(const DB::Cells::Interval& intval) {
    Core::SharedLock lock(m_mutex);
    return m_key_begin.equal(intval.key_begin);
           m_key_end.equal(intval.key_end);
  }

  bool includes(const DB::Cell::Key& range_begin,
                const DB::Cell::Key& range_end) {
    Core::SharedLock lock(m_mutex);
    return (
        m_key_begin.empty() || range_end.empty() ||
        DB::KeySeq::compare(cfg->key_seq, range_end, m_key_begin)
          != Condition::GT
      ) && (
        m_key_end.empty() || range_begin.empty() ||
        DB::KeySeq::compare(cfg->key_seq, range_begin, m_key_end)
          != Condition::LT
      );
  }

  bool after(const Ptr& range) {
    Core::SharedLock lock(m_mutex);
    return range->before(m_key_end);
  }

  SWC_CAN_INLINE
  bool before(const DB::Cell::Key& key) {
    Core::SharedLock lock(m_mutex);
    return m_key_end.empty() ||
      (!key.empty() &&
        DB::KeySeq::compare(cfg->key_seq, key, m_key_end) != Condition::GT);
  }

  void print(std::ostream& out) {
    Core::SharedLock lock(m_mutex);
    cfg->print(out << '(');
    out << " rid="    << rid
        << " state="  << DB::Types::to_string(m_state)
        << " rgr="    << m_rgrid
        << " rev="    << m_load_revision
        << ')';
  }

  private:
  const std::string             m_path;

  std::shared_mutex             m_mutex;
  State                         m_state;
  int64_t                       m_check_ts;
  rgrid_t                       m_rgrid;
  std::unique_ptr<DB::RgrData>  m_last_rgr;

  DB::Cell::Key                 m_key_begin;
  DB::Cell::Key                 m_key_end;
  int64_t                       m_load_revision;

};


}}



#endif // swcdb_manager_db_Range_h
