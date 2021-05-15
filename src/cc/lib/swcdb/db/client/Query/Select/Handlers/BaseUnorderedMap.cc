/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Select/Handlers/BaseUnorderedMap.h"


namespace SWC { namespace client { namespace Query { namespace Select {

namespace Handlers {



bool BaseUnorderedMap::Rsp::add_cells(const StaticBuffer& buffer,
                                      bool reached_limit,
                                      DB::Specs::Interval& interval) {
  Core::MutexSptd::scope lock(m_mutex);
  size_t recved = m_cells.add(buffer.base, buffer.size);

  if(interval.flags.limit) {
    if(interval.flags.limit <= recved) {
      interval.flags.limit = 0;
      return false;
    }
    interval.flags.limit -= recved;
  }

  if(reached_limit) {
    auto last = m_cells.back();
    interval.offset_key.copy(last->key);
    interval.offset_rev = last->get_timestamp();
  }
  return true;
}

void BaseUnorderedMap::Rsp::get_cells(DB::Cells::Result& cells) {
  Core::MutexSptd::scope lock(m_mutex);
  cells.take(m_cells);
}

size_t BaseUnorderedMap::Rsp::get_size() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size();
}

size_t BaseUnorderedMap::Rsp::get_size_bytes() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size_bytes();
}

bool BaseUnorderedMap::Rsp::empty() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.empty();
}

void BaseUnorderedMap::Rsp::free() {
  Core::MutexSptd::scope lock(m_mutex);
  m_cells.free();
}


void BaseUnorderedMap::Rsp::error(int err) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  m_err = err;
}

int BaseUnorderedMap::Rsp::error() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_err;
}



void BaseUnorderedMap::error(const cid_t cid, int err) {
  get_columnn(cid)->error(err);
}

bool BaseUnorderedMap::add_cells(const cid_t cid, const StaticBuffer& buffer,
                       bool reached_limit, DB::Specs::Interval& interval) {
  return get_columnn(cid)->add_cells(buffer, reached_limit, interval);
}

size_t BaseUnorderedMap::get_size_bytes() noexcept {
  size_t sz = 0;
  Core::MutexSptd::scope lock(m_mutex);
  for(const auto& col : m_columns)
    sz += col.second->get_size_bytes();
  return sz;
}

void BaseUnorderedMap::add_column(const cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  m_columns.emplace(cid, std::make_shared<Rsp>());
}

BaseUnorderedMap::Rsp::Ptr& BaseUnorderedMap::get_columnn(const cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  auto& c = m_columns[cid];
  if(!c)
    c.reset(new Rsp());
  return c;
}

void BaseUnorderedMap::get_cells(const cid_t cid, DB::Cells::Result& cells) {
  get_columnn(cid)->get_cells(cells);
}

size_t BaseUnorderedMap::get_size(const cid_t cid) {
  return get_columnn(cid)->get_size();
}

bool BaseUnorderedMap::empty() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  for(const auto& col : m_columns)
    if(!col.second->empty())
      return false;
  return true;
}

std::vector<cid_t> BaseUnorderedMap::get_cids() {
  Core::MutexSptd::scope lock(m_mutex);
  std::vector<cid_t> list(m_columns.size());
  int i = 0;
  for(const auto& col : m_columns)
    list[i++] = col.first;
  return list;
}

void BaseUnorderedMap::free(const cid_t cid) {
  get_columnn(cid)->free();
}

void BaseUnorderedMap::remove(const cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  m_columns.erase(cid);
}




}}}}}
