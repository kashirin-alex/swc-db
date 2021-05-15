/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Update/Handlers/BaseUnorderedMap.h"


namespace SWC { namespace client { namespace Query { namespace Update {

namespace Handlers {


bool BaseUnorderedMap::requires_commit() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it)
    if(!it->second->error() && !it->second->empty())
      return true;
  return false;
}

bool BaseUnorderedMap::empty() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it)
    if(!it->second->empty())
      return false;
  return true;
}

size_t BaseUnorderedMap::size() noexcept {
  size_t total = 0;
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it)
    total += it->second->size();
  return total;
}

size_t BaseUnorderedMap::size_bytes() noexcept {
  size_t total = 0;
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it)
    total += it->second->size_bytes();
  return total;
}

void BaseUnorderedMap::next(std::vector<Base::Column*>& cols) noexcept {
  cols.clear();
  Core::MutexSptd::scope lock(m_mutex);
  for(auto it = cbegin(); it != cend(); ++it) {
    if(!it->second->error() && !it->second->empty())
      cols.push_back(it->second.get());
  }
}

Base::Column* BaseUnorderedMap::next(cid_t cid) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  return it != end() && !it->second->error() && !it->second->empty()
    ? it->second.get() : nullptr;
}

void BaseUnorderedMap::error(cid_t cid, int err) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  if(it != cend())
    it->second->error(err);
}

ColumnMutable::Ptr& BaseUnorderedMap::create(const cid_t cid,
                                             DB::Types::KeySeq seq,
                                             uint32_t versions,
                                             uint32_t ttl_secs,
                                             DB::Types::Column type) {
  Core::MutexSptd::scope lock(m_mutex);
  auto& col = (*this)[cid];
  if(!col)
    col.reset(new ColumnMutable(cid, seq, versions, ttl_secs, type));
  return col;
}

bool BaseUnorderedMap::exists(const cid_t cid) noexcept{
  Core::MutexSptd::scope lock(m_mutex);
  return find(cid) != end();
}

void BaseUnorderedMap::add(const cid_t cid, const DB::Cells::Cell& cell) {
  ColumnMutable::Ptr col;
  {
    Core::MutexSptd::scope lock(m_mutex);
    auto it = find(cid);
    if(it == end())
      SWC_THROWF(ENOKEY, "Map Missing column=%lu (1st do create)", cid);
    col = it->second;
  }
  col->add(cell);
}

ColumnMutable::Ptr BaseUnorderedMap::get(cid_t cid) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  return it == end() ? nullptr : it->second;
}

Base::Column* BaseUnorderedMap::get_base_ptr(cid_t cid) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  auto it = find(cid);
  return it == end() ? nullptr : it->second.get();
}

}}}}}
