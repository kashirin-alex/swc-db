/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/Columns.h"


namespace SWC { namespace Ranger {


SWC_CAN_INLINE
ColumnPtr Columns::get_column(const cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  return it == cend() ? nullptr : it->second;
}

SWC_CAN_INLINE
RangePtr Columns::get_range(int &err, const cid_t cid, const rid_t rid) {
  ColumnPtr col = get_column(cid);
  if(!col)
    return nullptr;
  if(col->removing())
    err = Error::COLUMN_MARKED_REMOVED;
  return err ? nullptr : col->get_range(rid);
}

ColumnPtr Columns::get_next(cid_t& last_cid, size_t& idx) {
  Core::MutexSptd::scope lock(m_mutex);
  if(last_cid) {
    auto it = find(last_cid);
    if(it != end()) {
      if(++it != end()) {
        last_cid = it->second->cfg->cid;
        return it->second;
      }
      idx = 0;
      last_cid = 0;
      return nullptr;
    }
  }
  if(size() > idx) {
    auto it = cbegin();
    for(size_t i=idx; i; --i, ++it);
    last_cid = it->second->cfg->cid;
    return it->second;
  }
  idx = 0;
  last_cid = 0;
  return nullptr;
}

void Columns::get_cids(std::vector<cid_t>& cids) {
  Core::MutexSptd::scope lock(m_mutex);
  cids.reserve(size());
  for(auto it = cbegin(); it != cend(); ++it)
    cids.push_back(it->first);
}

void Columns::get_columns(Core::Vector<ColumnPtr>& columns) {
  Core::MutexSptd::scope lock(m_mutex);
  columns.reserve(size());
  for(auto it = cbegin(); it != cend(); ++it)
    columns.push_back(it->second);
}

void Columns::load_range(const DB::SchemaPrimitives& schema,
                         const Callback::RangeLoad::Ptr& req) {
  int err = Error::OK;
  ColumnPtr col;
  if(Env::Rgr::is_shuttingdown() ||
     (Env::Rgr::is_not_accepting() &&
      DB::Types::SystemColumn::is_data(req->cid))) {
    err = Error::SERVER_SHUTTING_DOWN;

  } else {
    {
      Core::MutexSptd::scope lock(m_mutex);
      auto res = emplace(req->cid, nullptr);
      if(res.second) {
        res.first->second.reset(new Column(req->cid, schema));
      } else if(!res.first->second->ranges_count()) {
        if(res.first->second->cfg->use_count() > 1)
          SWC_LOGF(LOG_WARN,
                  "Column cid=%lu remained with use-count=%ld, resetting",
                  req->cid, res.first->second->cfg->use_count());
        res.first->second.reset(new Column(req->cid, schema));
      } else {
        res.first->second->cfg->update(schema);
      }
      col = res.first->second;
    }
    if(col->removing()) {
      err = Error::COLUMN_MARKED_REMOVED;

    } else if(Env::Rgr::is_shuttingdown() ||
              (Env::Rgr::is_not_accepting() &&
               DB::Types::SystemColumn::is_data(req->cid))) {
      err = Error::SERVER_SHUTTING_DOWN;
    }
  }
  err
    ? req->response(err)
    : col->add_managing(Callback::ManageBase::Ptr(req));
}

bool Columns::unload(cid_t cid_begin, cid_t cid_end,
                     Callback::ColumnsUnload::Ptr req) {
  {
    uint8_t count = 0;
    uint32_t max = Env::Rgr::res().concurrency() / 2;
    if(!max)
      max = 1;
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it) {
      if((!cid_begin || cid_begin <= it->first) &&
         (!cid_end || cid_end >= it->first)) {
        req->add(it->second);
        if(++count == max)
          return ++it != cend();
      }
    }
  }
  return false;
}

void Columns::unload_all(bool validation) {
  Callback::ColumnsUnloadAll::Ptr req(
    new Callback::ColumnsUnloadAll(validation));
  bool more;
  do {
    more = unload(DB::Types::SystemColumn::SYS_CID_END + 1, 0, req);
    req->run();
    req->wait();
  } while(more);

  do {
    more = unload(DB::Types::SystemColumn::SYS_RGR_DATA + 1, 0, req);
    req->run();
    req->wait();
  } while(more);

  do {
    more = unload(DB::Types::SystemColumn::CID_META_BEGIN, 0, req);
    req->run();
    req->wait();
  } while(more);

  do {
    more = unload(0, 0, req);
    req->run();
    req->wait();
  } while(more);
}

void Columns::erase_if_empty(cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  if(it != cend() && it->second->is_not_used()) {
    erase(it);
  }
}

void Columns::internal_delete(cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  if(it != cend())
    erase(it);
}

size_t Columns::release(size_t bytes) {
  if(m_release.running())
    return 0;

  size_t released = 0;
  uint8_t level = 0;
  /* Release-Levels:
    0: cellstores
    1: fragments
    2: blocks (only data-columns)
    3: commit-log
    4: blocks-structure (only data-columns)
  */
  do {
    SWC_LOGF(LOG_DEBUG, "Columns::release bytes=%lu level=%u released=%lu",
                        bytes, level, released);
    ColumnPtr col = nullptr;
    const_iterator it;
    for(size_t offset = 0; ; ++offset) {
      {
        Core::MutexSptd::scope lock(m_mutex);
        if(col && (it = find(col->cfg->cid)) != cend()) {
          ++it;
        } else {
          it = cbegin();
          for(size_t i=0; i < offset && it != cend(); ++it, ++i);
        }
        if(level == 2 || level == 4) {
          for(; it != cend() &&
                !DB::Types::SystemColumn::is_data(it->first);
                ++it);
        }
        if(it == cend())
          break;
        col = it->second;
      }
      released += col->release(bytes - released, level);
      if(released >= bytes)
        break;
    }
  } while(released < bytes && ++level < 5);

  m_release.stop();
  return released;
}

void Columns::print(std::ostream& out, bool minimal) {
  out << "columns=[";
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it){
    it->second->print(out, minimal);
    out << ", ";
  }
  out << "]";
}


}} // namespace SWC::Ranger
