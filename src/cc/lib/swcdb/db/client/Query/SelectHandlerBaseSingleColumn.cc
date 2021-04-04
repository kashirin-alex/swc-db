/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/SelectHandlerBaseSingleColumn.h"


namespace SWC { namespace client { namespace Query { namespace Select {

namespace Handlers {


void BaseSingleColumn::error(const cid_t _cid, int err) {
  int at = Error::OK;
  state_error.compare_exchange_weak(
    at, _cid == cid ? err : Error::CLIENT_MISMATCHED_CID);
}

bool BaseSingleColumn::add_cells(const cid_t _cid, const StaticBuffer& buffer,
                                 bool reached_limit,
                                 DB::Specs::Interval& interval) {
  if(_cid != cid) {
    error(_cid, Error::OK);
    return false;
  }

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

size_t BaseSingleColumn::get_size_bytes() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size_bytes();
}

size_t BaseSingleColumn::get_size() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size();
}

bool BaseSingleColumn::empty() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.empty();
}

void BaseSingleColumn::get_cells(DB::Cells::Result& cells) {
  Core::MutexSptd::scope lock(m_mutex);
  cells.take(m_cells);
}


}}}}}
