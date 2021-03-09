/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/Column.h"

namespace SWC { namespace Ranger {



Column::Column(const cid_t cid, const DB::Schema& schema)
              : cfg(new ColumnCfg(cid, schema)) {
  Env::Rgr::in_process(1);
  Env::Rgr::res().more_mem_usage(size_of());
}

Column::~Column() {
  Env::Rgr::res().less_mem_usage(size_of());
  Env::Rgr::in_process(-1);
}

size_t Column::size_of() const noexcept {
  return sizeof(*this) + sizeof(ColumnPtr);
}

size_t Column::ranges_count() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return size();
}

bool Column::removing() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return cfg->deleting;
}

bool Column::is_not_used() noexcept {
  return cfg.use_count() == 1 && m_q_mng.empty() && !ranges_count();
}

RangePtr Column::get_range(const rid_t rid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(rid);
  return it == end() ? nullptr : it->second;
}

RangePtr Column::get_next(size_t &idx) {
  Core::MutexSptd::scope lock(m_mutex);

  if(size() > idx) {
    auto it = begin();
    for(int i=idx; i; --i, ++it);
    return it->second;
  }
  idx = 0;
  return nullptr;
}

void Column::get_rids(std::vector<rid_t>& rids) {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = begin(); it != end(); ++it)
    rids.push_back(it->first);
}

void Column::schema_update(const DB::Schema& schema) {
  bool compact = cfg->c_versions > schema.cell_versions ||
                 (schema.cell_ttl && cfg->c_ttl > schema.cell_ttl);
  bool and_cells =  cfg->c_versions != schema.cell_versions ||
                    cfg->c_ttl != schema.cell_ttl ||
                    cfg->col_type != schema.col_type;
  cfg->update(schema);
  if(and_cells) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = begin(); it != end(); ++it)
      it->second->schema_update(compact);
  }
  if(compact)
    Env::Rgr::compaction_schedule(100);
}

void Column::compact() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = begin(); it != end(); ++it)
      it->second->compact_require(true);
  }
  Env::Rgr::compaction_schedule(100);
}


void Column::add_managing(const Callback::ManageBase::Ptr& req) {
  if(m_q_mng.activating(req))
    Env::Rgr::post(
      [req, ptr=shared_from_this()](){ ptr->run_mng_req(req); } );
}

void Column::run_mng_queue() {
  Callback::ManageBase::Ptr req;
  if(!m_q_mng.deactivating(&req))
    Env::Rgr::post(
      [req, ptr=shared_from_this()](){ ptr->run_mng_req(req); } );
}


size_t Column::release(size_t bytes) {
  if(m_release.running())
    return 0;

  RangePtr range;
  iterator it;
  size_t released = 0;
  for(size_t offset = 0; ; ++offset) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(cfg->deleting)
        break;
      it = begin();
      for(size_t i=0; i<offset && it != end(); ++it, ++i);
      if(it == end())
        break;
      range = it->second;
    }
    if(!range->is_loaded() || range->compacting())
      continue;
    released += range->blocks.release(bytes ? bytes-released : bytes);
    if(bytes && released >= bytes)
      break;
  }
  m_release.stop();
  return released;
}

void Column::print(std::ostream& out, bool minimal) {
  Core::MutexSptd::scope lock(m_mutex);
  cfg->print(out << '(');
  out << " ranges=[";

  for(auto it = begin(); it != end(); ++it){
    it->second->print(out, minimal);
    out << ", ";
  }
  out << "])";
}


RangePtr Column::internal_create(int& err, rid_t rid, bool compacting) {
  Core::MutexSptd::scope lock(m_mutex);
  if(Env::Rgr::is_shuttingdown() ||
     (Env::Rgr::is_not_accepting() &&
      DB::Types::MetaColumn::is_data(cfg->cid))) {
    err = Error::SERVER_SHUTTING_DOWN;
  } else if(cfg->deleting) {
    err = Error::COLUMN_MARKED_REMOVED;
  }
  if(err)
    return nullptr;

  auto res = emplace(rid, nullptr);
  if(res.second) {
    res.first->second.reset(new Range(cfg, rid));
    res.first->second->init();
    if(compacting)
      res.first->second->compacting(Range::COMPACT_APPLYING);
  }
  return res.first->second;
}

void Column::internal_unload(const rid_t rid) {
  auto range = get_range(rid);
  if(range) {
    bool chk_empty = false;
    range->internal_unload(true, chk_empty);
    range = nullptr;
    internal_delete(rid);
  }
}

void Column::internal_unload(const rid_t rid, bool& chk_empty) {
  auto range = get_range(rid);
  if(range) {
    range->internal_unload(true, chk_empty);
    range = nullptr;
    internal_delete(rid);
  }
}

void Column::internal_remove(int &err, const rid_t rid) {
  auto range = get_range(rid);
  if(range) {
    range->internal_remove(err);
    range = nullptr;
    internal_delete(rid);
  }
}

void Column::internal_delete(rid_t rid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(rid);
  if(it != end())
    erase(it);
}


void Column::run_mng_req(const Callback::ManageBase::Ptr& req) {
  switch(req->action) {
    case Callback::ManageBase::RANGE_LOAD: {
      load(
        std::dynamic_pointer_cast<Callback::RangeLoad>(req));
      break;
    }

    case Callback::ManageBase::RANGE_UNLOAD: {
      unload(
        std::dynamic_pointer_cast<Callback::RangeUnload>(req));
      break;
    }

    case Callback::ManageBase::COLUMNS_UNLOAD: {
      unload_all(
        std::dynamic_pointer_cast<Callback::ColumnsUnload>(req));
      break;
    }

    case Callback::ManageBase::COLUMN_DELETE: {
      remove(
        std::dynamic_pointer_cast<Callback::ColumnDelete>(req));
      break;
    }
    /*
    default: {
      req->send_error(Error::NOT_IMPLEMENTED);
      run_mng_queue();
      break;
    }
    */
  }
}

void Column::load(const Callback::RangeLoad::Ptr& req) {
  if(req->expired(1000))
    return run_mng_queue();

  int err = Error::OK;
  auto range = internal_create(err, req->rid, false);
  req->col = shared_from_this();
  err ? req->loaded(err) : range->load(req);
}

void Column::unload(const Callback::RangeUnload::Ptr& req) {
  if(req->expired(1000))
    return run_mng_queue();

  bool chk_empty = true;
  internal_unload(req->rid, chk_empty);
  if(chk_empty)
    req->rsp_params.set_empty();
  req->response_params();
  Env::Rgr::columns()->erase_if_empty(cfg->cid);
  run_mng_queue();
}

void Column::unload_all(const Callback::ColumnsUnload::Ptr& req) {
  if(!cfg->deleting) for(RangePtr range;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(empty())
        break;
      range = begin()->second;
    }
    if(range) {
      bool chk_empty = false;
      range->internal_unload(req->completely, chk_empty);
      req->unloaded(range);
      internal_delete(range->rid);
    }
  }
  req->unloaded(shared_from_this());
  Env::Rgr::columns()->erase_if_empty(cfg->cid);
  run_mng_queue();
}

void Column::remove(const Callback::ColumnDelete::Ptr& req) {
  if(req->expired(1000))
    return run_mng_queue();

  if(cfg->deleting)
    return req->response_ok();

  cfg->deleting.store(true);
  std::vector<RangePtr> ranges;
  {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it=begin(); it != end(); ++it) {
      req->add(it->second);
      ranges.push_back(it->second);
    }
  }
  req->col = shared_from_this();
  if(ranges.empty()) {
    req->complete();
  } else {
    for(auto& range : ranges)
      range->remove(req);
  }
}


}}
