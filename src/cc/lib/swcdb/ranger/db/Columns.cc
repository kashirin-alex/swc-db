/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/Columns.h"


namespace SWC { namespace Ranger {


ColumnPtr Columns::get_column(const cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  return it == end() ? nullptr : it->second;
}

RangePtr Columns::get_range(int &err, const cid_t cid, const rid_t rid) {
  ColumnPtr col = get_column(cid);
  if(!col) 
    return nullptr;
  if(col->removing()) 
    err = Error::COLUMN_MARKED_REMOVED;
  return err ? nullptr : col->get_range(rid);
}

ColumnPtr Columns::get_next(size_t& idx) {
  Core::MutexSptd::scope lock(m_mutex);
  if(size() > idx) {
    auto it = begin();
    for(int i=idx; i; --i, ++it);
    return it->second;
  }
  idx = 0;
  return nullptr;
}

void Columns::get_cids(std::vector<cid_t>& cids) {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = begin(); it != end(); ++it)
    cids.push_back(it->first);
}

void Columns::load_range(const DB::Schema& schema, 
                         const Callback::RangeLoad::Ptr& req) {
  int err = Error::OK;
  ColumnPtr col;
  if(Env::Rgr::is_shuttingdown() || 
     (Env::Rgr::is_not_accepting() && 
      DB::Types::MetaColumn::is_data(req->cid))) {
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
                  "Column cid=%lu remained with use-count=%lu, resetting",
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
               DB::Types::MetaColumn::is_data(req->cid))) {
      err = Error::SERVER_SHUTTING_DOWN;
    }
  }
  err ? req->response(err) : col->add_managing(req);
}

void Columns::unload(cid_t cid_begin, cid_t cid_end,
                     Callback::ColumnsUnload::Ptr req) {
  std::vector<ColumnPtr> cols;
  {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = begin(); it != end(); ++it) {
      if((!cid_begin || cid_begin <= it->first) &&
         (!cid_end || cid_end >= it->first)) {
        req->add(it->second);
        cols.push_back(it->second);
      }
    }
  }
  if(cols.empty()) {
    req->response();
  } else {
    for(auto& col : cols)
      col->add_managing(req);
  }
}

void Columns::unload_all(bool validation) {
  auto req = std::make_shared<Callback::ColumnsUnloadAll>(validation);
  unload(9, 0, req);
  req->wait();

  req = std::make_shared<Callback::ColumnsUnloadAll>(validation);
  unload(5, 0, req);
  req->wait();

  req = std::make_shared<Callback::ColumnsUnloadAll>(validation);
  unload(0, 0, req);
  req->wait();
}

void Columns::erase_if_empty(cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  if(it != end() && it->second->is_not_used()) {
    erase(it);
  }
}

size_t Columns::release(size_t bytes) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(m_releasing)
      return 0;
    m_releasing = true;
  }

  ColumnPtr col;
  iterator it;
  size_t released = 0;
  for(size_t offset = 0; ; ++offset) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      it = begin();
      for(size_t i=0; i<offset && it != end(); ++it, ++i);
      if(it == end())
        break;
      if(!DB::Types::MetaColumn::is_data(it->first))
        continue;
      col = it->second;
    }
    released += col->release(bytes ? bytes-released : bytes);
    if(bytes && released >= bytes)
      break;
  }
  m_releasing = false;
  return released;
}

void Columns::print(std::ostream& out, bool minimal) {
  out << "columns=[";
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = begin(); it != end(); ++it){
    it->second->print(out, minimal);
    out << ", ";
  }
  out << "]"; 
}


}} // namespace SWC::Ranger
