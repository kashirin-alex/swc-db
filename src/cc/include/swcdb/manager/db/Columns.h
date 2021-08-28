/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_db_Columns_h
#define swcdb_manager_db_Columns_h


#include "swcdb/db/Types/SystemColumn.h"
#include "swcdb/fs/Interface.h"

#include "swcdb/manager/db/ColumnCfg.h"
#include "swcdb/manager/db/Column.h"
#include "swcdb/common/Files/Schema.h"
#include <list>


namespace SWC { namespace Manager {



class Columns final : private std::unordered_map<cid_t, Column::Ptr> {

  public:

  static void columns_by_fs(int &err, FS::IdEntries_t &entries) {
    Env::FsInterface::interface()->get_structured_ids(
      err, DB::RangeBase::get_column_path(), entries);
  }

  typedef Columns* Ptr;

  Columns() noexcept : m_health_last_cid(DB::Schema::NO_CID) { }

  //~Columns() { }

  void reset() {
    {
      Core::MutexSptd::scope lock(m_mutex);
      clear();
    }
    for(auto& g : m_need_assign)
      g.reset();
  }

  void state(int& err) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); !err && it != cend(); ++it)
      it->second->state(err);
  }

  bool is_an_initialization(int &err, const DB::Schema::Ptr& schema) {
    Column::Ptr col = nullptr;
    {
      Core::MutexSptd::scope lock(m_mutex);
      auto res = emplace(schema->cid, nullptr);
      if(!res.second)
        return false;
      res.first->second.reset(new Column(schema));
      col = res.first->second;
    }
    col->init(err);
    if(err) {
      remove(schema->cid);
    }
    return !err;
  }

  Column::Ptr get_column(int &err, const cid_t cid) {
    Core::MutexSptd::scope lock(m_mutex);
    auto it = find(cid);
    if(it != cend())
      return it->second;
    err = Error::COLUMN_NOT_EXISTS;
    return nullptr;
  }

  Range::Ptr get_next_unassigned(bool& waiting_meta) {
    #define _PRELOAD(_GROUP, _BEGIN, _END) \
      if(Range::Ptr range = m_need_assign[_GROUP].get()) \
        return range; \
      for(cid_t cid = _BEGIN; cid <= _END; ++cid) { \
        Core::MutexSptd::scope lock(m_mutex); \
        auto it = find(cid); \
        if(it != cend()) { \
          if((waiting_meta = it->second->_state() != Column::State::OK)) \
            return nullptr; \
        } \
      }
    _PRELOAD(
      0,
      DB::Types::SystemColumn::CID_MASTER_BEGIN,
      DB::Types::SystemColumn::CID_MASTER_END
    );
    _PRELOAD(
      1,
      DB::Types::SystemColumn::CID_META_BEGIN,
      DB::Types::SystemColumn::CID_META_END
    );
    _PRELOAD(
      2,
      DB::Types::SystemColumn::SYS_RGR_DATA,
      DB::Types::SystemColumn::SYS_RGR_DATA
    );
    #undef _PRELOAD
    return m_need_assign[3].get();
  }

  void set_rgr_unassigned(rgrid_t rgrid) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it)
      it->second->set_rgr_unassigned(rgrid);
  }

  void change_rgr(rgrid_t rgrid_old, rgrid_t rgrid) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it)
      it->second->change_rgr(rgrid_old, rgrid);
  }

  void assigned(rgrid_t rgrid, size_t num, Core::Vector<Range::Ptr>& ranges) {
    Column::Ptr chk;
    Core::Vector<cid_t> chked;
    do {
      chk = nullptr;
      {
        Core::MutexSptd::scope lock(m_mutex);
        chked.reserve(size());
        for(auto it = cbegin(); it != cend(); ++it) {
          if(std::find(chked.cbegin(), chked.cend(), it->first)
              == chked.cend()) {
            chk = it->second;
            chked.push_back(it->first);
            break;
          }
        }
      }
      if(!chk)
        return;
      chk->assigned(rgrid, num, ranges);
    } while(num);
  }

  Column::Ptr get_need_health_check(int64_t ts, uint32_t ms) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = m_health_last_cid ? find(m_health_last_cid) : cbegin();
        it != cend(); ++it) {
      if(it->second->need_health_check(ts, ms)) {
        m_health_last_cid = it->second->cfg->cid;
        return it->second;
      }
    }
    m_health_last_cid = DB::Schema::NO_CID;
    return nullptr;
  }

  void remove(const cid_t cid) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      auto it = find(cid);
      if(it == cend())
        return;
      erase(it);
    }
    assign_group(cid).remove(cid);
  }

  void assign_add(Range::Ptr&& range) {
    assign_group(range->cfg->cid).add(std::move(range));
  }

  void assign_add(const Range::Ptr& range) {
    assign_group(range->cfg->cid).add(range);
  }

  void assign_remove(const Range::Ptr& range) {
    assign_group(range->cfg->cid).remove(range);
  }

  void print(std::ostream& out) {
    out << "ColumnsAssignment:";
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it)
      it->second->print(out << "\n ");
  }

  private:

  class AssignGroup final : private std::unordered_set<Range::Ptr> {
    public:

    AssignGroup() noexcept { }

    AssignGroup(AssignGroup&&)                 = delete;
    AssignGroup(const AssignGroup&)            = delete;
    AssignGroup& operator=(AssignGroup&&)      = delete;
    AssignGroup& operator=(const AssignGroup&) = delete;

    SWC_CAN_INLINE
    Range::Ptr get() {
      Core::MutexSptd::scope lock(m_mutex);
      return empty() ? nullptr : *cbegin();
    }

    SWC_CAN_INLINE
    void add(Range::Ptr&& range) {
      Core::MutexSptd::scope lock(m_mutex);
      insert(std::move(range));
    }

    SWC_CAN_INLINE
    void add(const Range::Ptr& range) {
      Core::MutexSptd::scope lock(m_mutex);
      insert(range);
    }

    SWC_CAN_INLINE
    void remove(const Range::Ptr& range) {
      Core::MutexSptd::scope lock(m_mutex);
      erase(range);
    }

    SWC_CAN_INLINE
    void remove(const cid_t cid) {
      Core::MutexSptd::scope lock(m_mutex);
      for(auto it = cbegin();
          it != cend();
          (*it)->cfg->cid == cid ? (it = erase(it)) : ++it
      );
    }

    SWC_CAN_INLINE
    void reset() {
      Core::MutexSptd::scope lock(m_mutex);
      clear();
    }

    private:
    Core::MutexSptd                 m_mutex;
  };

  AssignGroup& assign_group(const cid_t cid) noexcept {
    return m_need_assign[
      cid > DB::Types::SystemColumn::SYS_RGR_DATA
        ? 3
        : (cid > DB::Types::SystemColumn::CID_META_END
            ? 2
            : (cid > DB::Types::SystemColumn::CID_MASTER_END
              ? 1
              : 0))
      ];
  }

  AssignGroup       m_need_assign[4];
  Core::MutexSptd   m_mutex;
  cid_t             m_health_last_cid;

};


}} // namespace SWC::Manager



#endif // swcdb_manager_db_Columns_h
