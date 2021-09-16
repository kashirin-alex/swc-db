/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/Column.h"

namespace SWC { namespace Ranger {



SWC_CAN_INLINE
Column::Column(const cid_t cid, const DB::SchemaPrimitives& schema)
              : cfg(new ColumnCfg(cid, schema)) {
  Env::Rgr::in_process(1);
}

Column::~Column() {
  Env::Rgr::in_process(-1);
}

size_t Column::ranges_count() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return size();
}

SWC_CAN_INLINE
bool Column::removing() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return cfg->deleting;
}

bool Column::is_not_used() noexcept {
  return cfg.use_count() == 1 && m_q_mng.empty() && !ranges_count();
}

SWC_CAN_INLINE
RangePtr Column::get_range(const rid_t rid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(rid);
  return it == cend() ? nullptr : it->second;
}

RangePtr Column::get_next(cid_t& last_rid, size_t &idx) {
  Core::MutexSptd::scope lock(m_mutex);
  if(last_rid) {
    auto it = find(last_rid);
    if(it != end()) {
      if(++it != end()) {
        last_rid = it->second->rid;
        return it->second;
      }
      idx = 0;
      last_rid = 0;
      return nullptr;
    }
  }
  if(size() > idx) {
    auto it = cbegin();
    for(size_t i=idx; i; --i, ++it);
    last_rid = it->second->rid;
    return it->second;
  }
  idx = 0;
  last_rid = 0;
  return nullptr;
}

void Column::get_rids(rids_t& rids) {
  Core::MutexSptd::scope lock(m_mutex);
  rids.reserve(size());
  for(auto it = cbegin(); it != cend(); ++it)
    rids.push_back(it->first);
}

void Column::get_ranges(Core::Vector<RangePtr>& ranges) {
  Core::MutexSptd::scope lock(m_mutex);
  ranges.reserve(size());
  for(auto it = cbegin(); it != cend(); ++it)
    ranges.push_back(it->second);
}

void Column::schema_update(const DB::SchemaPrimitives& schema) {
  bool compact =
    cfg->c_versions > schema.cell_versions ||
    (!cfg->c_ttl && schema.cell_ttl) ||
    cfg->c_ttl > uint64_t(schema.cell_ttl) * 1000000000;
  bool and_cells =  cfg->c_versions != schema.cell_versions ||
                    cfg->col_type != schema.col_type ||
                    cfg->c_ttl != uint64_t(schema.cell_ttl) * 1000000000;
  cfg->update(schema);
  if(and_cells) {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it)
      it->second->schema_update(compact);
  }
  if(compact)
    Env::Rgr::compaction_schedule(100);
}

void Column::compact() {
  {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto it = cbegin(); it != cend(); ++it)
      it->second->compact_require(true);
  }
  Env::Rgr::compaction_schedule(100);
}


SWC_CAN_INLINE
size_t Column::release(size_t bytes, uint8_t level) {
  if(m_release.running())
    return 0;

  RangePtr range = nullptr;
  const_iterator it;
  size_t released = 0;
  for(size_t offset = 0; ; ++offset) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(cfg->deleting)
        break;
      if(range && (it = find(range->rid)) != cend()) {
        ++it;
      } else {
        it = cbegin();
        for(size_t i=0; i<offset && it != cend(); ++it, ++i);
      }
      if(it == cend())
        break;
      range = it->second;
    }
    if(!range->is_loaded() || range->compacting())
      continue;
    released += range->blocks.release(bytes - released, level);
    if(released >= bytes)
      break;
  }
  m_release.stop();
  return released;
}

void Column::print(std::ostream& out, bool minimal) {
  Core::MutexSptd::scope lock(m_mutex);
  cfg->print(out << '(');
  out << " ranges=[";

  for(auto it = cbegin(); it != cend(); ++it){
    it->second->print(out, minimal);
    out << ", ";
  }
  out << "])";
}


RangePtr Column::internal_create(int& err, rid_t rid, bool compacting) {
  Core::MutexSptd::scope lock(m_mutex);
  if(Env::Rgr::is_shuttingdown() ||
     (Env::Rgr::is_not_accepting() &&
      DB::Types::SystemColumn::is_data(cfg->cid))) {
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
  if(it != cend())
    erase(it);
}


struct Column::TaskRunMngReq {
  ColumnPtr                 ptr;
  Callback::ManageBase::Ptr req;
  SWC_CAN_INLINE
  TaskRunMngReq(ColumnPtr&& a_ptr, Callback::ManageBase::Ptr&& a_req)
                noexcept : ptr(std::move(a_ptr)), req(std::move(a_req)) {
  }
  void operator()() {
    switch(req->action) {
    case Callback::ManageBase::RANGE_LOAD: {
      ptr->load(
        std::dynamic_pointer_cast<Callback::RangeLoad>(req));
      break;
    }

    case Callback::ManageBase::RANGE_UNLOAD: {
      ptr->unload(
        std::dynamic_pointer_cast<Callback::RangeUnload>(req));
      break;
    }

    case Callback::ManageBase::RANGE_UNLOAD_INTERNAL: {
      ptr->unload(
        std::dynamic_pointer_cast<Callback::RangeUnloadInternal>(req));
      break;
    }

    case Callback::ManageBase::COLUMNS_UNLOAD: {
      ptr->unload_all(
        std::dynamic_pointer_cast<Callback::ColumnsUnload>(req));
      break;
    }

    case Callback::ManageBase::COLUMN_DELETE: {
      ptr->remove(
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
};

void Column::add_managing(Callback::ManageBase::Ptr&& req) {
  if(m_q_mng.activating(std::move(req)))
    Env::Rgr::post(TaskRunMngReq(shared_from_this(), std::move(req)));
}

void Column::run_mng_queue() {
  Callback::ManageBase::Ptr req;
  if(!m_q_mng.deactivating(req))
    Env::Rgr::post(TaskRunMngReq(shared_from_this(), std::move(req)));
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

void Column::unload(const Callback::RangeUnloadInternal::Ptr& req) {
  if(req) {
    bool chk_empty = false;
    internal_unload(req->rid, chk_empty);
    req->response();
    Env::Rgr::columns()->erase_if_empty(cfg->cid);
  }
  run_mng_queue();
}

void Column::unload_all(const Callback::ColumnsUnload::Ptr& req) {
  if(!cfg->deleting) for(RangePtr range;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(empty())
        break;
      range = cbegin()->second;
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
  Core::Vector<RangePtr> ranges;
  {
    Core::MutexSptd::scope lock(m_mutex);
    ranges.reserve(size());
    for(auto it=cbegin(); it != cend(); ++it) {
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
