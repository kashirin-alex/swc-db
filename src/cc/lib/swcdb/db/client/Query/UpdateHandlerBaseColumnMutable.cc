
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/UpdateHandlerBaseColumnMutable.h"


namespace SWC { namespace client { namespace Query { namespace Update {

namespace Handlers {



ColumnMutable::ColumnMutable(const cid_t cid, DB::Types::KeySeq seq,
                             uint32_t versions, uint32_t ttl_secs,
                             DB::Types::Column type)
                        : state_error(Error::OK),
                          cid(cid),
                          m_cells(seq, versions, ttl_secs*1000000000, type) {
}

void ColumnMutable::print(std::ostream& out) {
  out << "(cid=" << cid;
  {
    Core::MutexSptd::scope lock(m_mutex);
    m_cells.print(out << ' ');
  }
  out << ')';
}

bool ColumnMutable::empty() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.empty();
}

size_t ColumnMutable::size() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size();
}

size_t ColumnMutable::size_bytes() noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return m_cells.size_bytes();
}

DB::Cell::Key::Ptr ColumnMutable::get_first_key() {
  auto key = std::make_shared<DB::Cell::Key>();
  Core::MutexSptd::scope lock(m_mutex);
  if(m_cells.size())
    m_cells.get(0, *key.get());
  return key;
}

DB::Cell::Key::Ptr
ColumnMutable::get_key_next(const DB::Cell::Key& eval_key, bool start_key) {
  auto key = std::make_shared<DB::Cell::Key>();
  Core::MutexSptd::scope lock(m_mutex);
  if(eval_key.empty() ||
    !m_cells.get(
      eval_key, start_key? Condition::GE : Condition::GT, *key.get()))
    return nullptr;
  return key;
}

size_t ColumnMutable::add(const DynamicBuffer& cells,
                          const DB::Cell::Key& upto_key,
                          const DB::Cell::Key& from_key,
                          uint32_t skip,
                          bool malformed) {
  Core::MutexSptd::scope lock(m_mutex);
  auto sz = m_cells.size();
  m_cells.add_raw(cells, upto_key, from_key, skip, malformed);
  return m_cells.size() - sz;
}

size_t ColumnMutable::add(const DynamicBuffer& cells) {
  Core::MutexSptd::scope lock(m_mutex);
  auto sz = m_cells.size();
  m_cells.add_raw(cells);
  return m_cells.size() - sz;
}

void ColumnMutable::add(const DB::Cells::Cell& cell) {
  Core::MutexSptd::scope lock(m_mutex);
  m_cells.add_raw(cell);
}

DynamicBuffer::Ptr ColumnMutable::get_buff(const DB::Cell::Key& key_start,
                                           const DB::Cell::Key& key_end,
                                           size_t buff_sz, bool& more) {
  auto cells_buff = std::make_shared<DynamicBuffer>();
  Core::MutexSptd::scope lock(m_mutex);
  more = m_cells.write_and_free(
    key_start, key_end, *cells_buff.get(), buff_sz);
  if(cells_buff->fill())
    return cells_buff;
  return nullptr;
}



}}}}}
