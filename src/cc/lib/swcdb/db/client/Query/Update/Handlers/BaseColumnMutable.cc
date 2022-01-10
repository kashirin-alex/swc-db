/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Update/Handlers/BaseColumnMutable.h"


namespace SWC { namespace client { namespace Query { namespace Update {

namespace Handlers {



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
  DB::Cell::Key::Ptr key(new DB::Cell::Key());
  Core::MutexSptd::scope lock(m_mutex);
  if(m_cells.size())
    m_cells.get(0, *key.get());
  return key;
}

DB::Cell::Key::Ptr
ColumnMutable::get_key_next(const DB::Cell::Key& eval_key, bool start_key) {
  if(eval_key.empty())
    return nullptr;
  DB::Cell::Key key;
  Core::MutexSptd::scope lock(m_mutex);
  return
    m_cells.get(eval_key, start_key ? Condition::GE : Condition::GT, key)
      ? DB::Cell::Key::Ptr(new DB::Cell::Key(std::move(key)))
      : nullptr;
}

size_t ColumnMutable::add(const DynamicBuffer& cells,
                          const DB::Cell::Key& upto_key,
                          const DB::Cell::Key& from_key,
                          uint32_t skip,
                          bool malformed) {
  Core::MutexSptd::scope lock(m_mutex);
  auto sz = m_cells.size();
  m_cells.add_raw(cells, upto_key, from_key, skip, malformed, false);
  return m_cells.size() - sz;
}

size_t ColumnMutable::add(const DynamicBuffer& cells, bool finalized) {
  Core::MutexSptd::scope lock(m_mutex);
  auto sz = m_cells.size();
  m_cells.add_raw(cells, finalized);
  return m_cells.size() - sz;
}

void ColumnMutable::add(const DB::Cells::Cell& cell, bool finalized) {
  Core::MutexSptd::scope lock(m_mutex);
  m_cells.add_raw(cell, finalized);
}

bool ColumnMutable::get_buff(const DB::Cell::Key& key_start,
                             const DB::Cell::Key& key_end,
                             size_t buff_sz, bool& more,
                             DynamicBuffer& cells_buff) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    more = m_cells.write_and_free(
      key_start, key_end, cells_buff, buff_sz);
  }
  return cells_buff.fill();
}

bool ColumnMutable::get_buff(size_t buff_sz, bool& more,
                             DynamicBuffer& cells_buff) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    more = m_cells.write_and_free(cells_buff, buff_sz);
  }
  return cells_buff.fill();
}



}}}}}
