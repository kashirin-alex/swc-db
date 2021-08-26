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
    Core::MutexSptd::scope lock(m_mutex);
    clear();
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
    } else {
      Core::MutexSptd::scope lock(m_mutex);
      m_need_assign.push_back(col->cfg->cid);
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

  bool get_next_unassigned(Column::Ptr& col, Range::Ptr& range,
                           bool& waiting_meta) {
    if(col && col->cfg->cid <= DB::Types::SystemColumn::SYS_CID_END &&
      (range = col->get_next_unassigned()))
      return true;

    const_iterator it;
    Core::MutexSptd::scope lock(m_mutex);
    for(cid_t cid = DB::Types::SystemColumn::CID_MASTER_BEGIN;
        cid <= DB::Types::SystemColumn::SYS_CID_END; ++cid) {
      if((it = find(cid)) != cend()) {
        if((range = it->second->get_next_unassigned())) {
          col = it->second;
          return true;
        }
        if(it->second->state() != Column::State::OK)
          waiting_meta = true;
      }
      if(waiting_meta &&
         (cid == DB::Types::SystemColumn::CID_MASTER_END ||
          cid == DB::Types::SystemColumn::CID_META_END ||
          cid == DB::Types::SystemColumn::SYS_RGR_DATA))
        return false;
    }
    if(waiting_meta)
      return false;

    while(!m_need_assign.empty()) {
      auto it = find(m_need_assign.front());
      m_need_assign.pop_front();
      if(it != cend() &&
         (range = it->second->get_next_unassigned())) {
          col = it->second;
        return true;
      }
    }

    if(col) {
      if((range = col->get_next_unassigned()))
        return true;
      if((it = find(col->cfg->cid)) != cend())
        ++it;
      for(; it != cend(); ++it) {
        if(it->second->state() != Column::State::DELETED &&
           (range = it->second->get_next_unassigned())) {
          col = it->second;
          return true;
        }
      }
    }

    for(it = cbegin(); it != cend(); ++it) {
      if(it->second->state() != Column::State::DELETED &&
         (range = it->second->get_next_unassigned())) {
        col = it->second;
        return true;
      }
    }
    return false;
  }

  void set_rgr_unassigned(rgrid_t rgrid) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it)
      if(it->second->set_rgr_unassigned(rgrid))
        m_need_assign.push_back(it->second->cfg->cid);
  }

  void change_rgr(rgrid_t rgrid_old, rgrid_t rgrid) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it)
      if(it->second->change_rgr(rgrid_old, rgrid))
        m_need_assign.push_back(it->second->cfg->cid);
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
          if(std::find(chked.cbegin(),chked.cend(),it->first)
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
    Core::MutexSptd::scope lock(m_mutex);
    auto it = find(cid);
    if(it != cend())
      erase(it);
    m_need_assign.remove(cid);
  }

  void print(std::ostream& out) {
    out << "ColumnsAssignment:";
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it)
      it->second->print(out << "\n ");
  }

  private:
  Core::MutexSptd    m_mutex;
  std::list<cid_t>   m_need_assign;
  cid_t              m_health_last_cid;

};


}} // namespace SWC::Manager



#endif // swcdb_manager_db_Columns_h
