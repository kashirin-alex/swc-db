/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 


#include "swcdb/db/Cells/MapMutable.h"


namespace SWC { namespace DB { namespace Cells {


ColCells::Ptr ColCells::make(const cid_t cid, Types::KeySeq seq, 
                             uint32_t versions, uint32_t ttl, 
                             Types::Column type) {
  return std::make_shared<ColCells>(cid, seq, versions, ttl, type);
}

ColCells::Ptr ColCells::make(const cid_t cid, Mutable& cells) {
  return std::make_shared<ColCells>(cid, cells);
}

ColCells::ColCells(const cid_t cid, Types::KeySeq seq, 
                   uint32_t versions, uint32_t ttl, 
                   Types::Column type)
                  : cid(cid), m_cells(seq, versions, ttl*1000000000, type) { 
}

ColCells::ColCells(const cid_t cid, Mutable& cells)
                  : cid(cid), m_cells(cells) { 
}

ColCells::~ColCells() {}

Types::KeySeq ColCells::get_sequence() const {
  return m_cells.key_seq;
}

DB::Cell::Key::Ptr ColCells::get_first_key() {
  auto key = std::make_shared<DB::Cell::Key>();
  Mutex::scope lock(m_mutex);
  SWC_ASSERT(m_cells.size()); // bad call , assure size pre-check
  m_cells.get(0, *key.get()); 
  return key;
}

DB::Cell::Key::Ptr ColCells::get_key_next(const DB::Cell::Key& eval_key, 
                                          bool start_key) {
  auto key = std::make_shared<DB::Cell::Key>();
  Mutex::scope lock(m_mutex);
  if(eval_key.empty() || 
    !m_cells.get(
      eval_key, start_key? Condition::GE : Condition::GT, *key.get()))
    return nullptr;
  return key;
}

DynamicBuffer::Ptr ColCells::get_buff(const DB::Cell::Key& key_start, 
                                      const DB::Cell::Key& key_end, 
                                      size_t buff_sz, bool& more) {
  auto cells_buff = std::make_shared<DynamicBuffer>();
  Mutex::scope lock(m_mutex);
  more = m_cells.write_and_free(
    key_start, key_end, *cells_buff.get(), buff_sz);
  if(cells_buff->fill())
    return cells_buff;
  return nullptr;
}

void ColCells::add(const DB::Cells::Cell& cell) {
  Mutex::scope lock(m_mutex);
  m_cells.add_raw(cell);
}

size_t ColCells::add(const DynamicBuffer& cells) {
  Mutex::scope lock(m_mutex);
  auto sz = m_cells.size();
  m_cells.add_raw(cells);
  return m_cells.size() - sz;
}

size_t ColCells::add(const DynamicBuffer& cells, 
                     const DB::Cell::Key& upto_key, 
                     const DB::Cell::Key& from_key) {
  Mutex::scope lock(m_mutex);
  auto sz = m_cells.size();
  m_cells.add_raw(cells, upto_key, from_key);
  return m_cells.size() - sz;
}

size_t ColCells::size() {
  Mutex::scope lock(m_mutex);
  return m_cells.size();
}

size_t ColCells::size_bytes() {
  Mutex::scope lock(m_mutex);
  return m_cells.size_bytes();
}

std::string ColCells::to_string() {
  std::string s("(cid=");
  s.append(std::to_string(cid));
  s.append(" ");
  {
    Mutex::scope lock(m_mutex);
    s.append(m_cells.to_string());
  }
  s.append(")");
  return s;
}





MapMutable::MapMutable() { }

MapMutable::~MapMutable() {}

bool MapMutable::create(const Schema::Ptr& schema) {
  return create(
    schema->cid, schema->col_seq, 
    schema->cell_versions, schema->cell_ttl, 
    schema->col_type);
}

bool MapMutable::create(const cid_t cid, Types::KeySeq seq, 
                        uint32_t versions, uint32_t ttl, Types::Column type) {
  Mutex::scope lock(m_mutex);
  return find(cid) == end() 
    ? emplace(cid, ColCells::make(cid, seq, versions, ttl, type)).second
    : false;
}

bool MapMutable::create(const cid_t cid, Mutable& cells) {
  Mutex::scope lock(m_mutex);
  return emplace(cid, ColCells::make(cid, cells)).second;
}

bool MapMutable::exists(const cid_t cid) {
  Mutex::scope lock(m_mutex);

  return find(cid) != end();
}

void MapMutable::add(const cid_t cid, const Cell& cell) {
  Mutex::scope lock(m_mutex);

  auto it = find(cid);
  if(it == end())
    SWC_THROWF(ENOKEY, "Map Missing column=%d (1st do create)", cid);
  it->second->add(cell);
}

ColCells::Ptr MapMutable::get_idx(size_t offset) {
  Mutex::scope lock(m_mutex);

  if(offset < Columns::size()) {
    auto it = begin();
    for(; offset; --offset, ++it);
    return it->second;
  }
  return nullptr;
}

ColCells::Ptr MapMutable::get_col(const cid_t cid) {
  Mutex::scope lock(m_mutex);

  auto it = find(cid);
  return it == end() ? nullptr : it->second;
}

void MapMutable::pop(ColCells::Ptr& col) {
  Mutex::scope lock(m_mutex);

  auto it = begin();
  if(it == end()) {
    col = nullptr;
  } else {
    col = it->second;
    erase(it);
  }
}

void MapMutable::pop(const cid_t cid, ColCells::Ptr& col) {
  Mutex::scope lock(m_mutex);

  auto it = find(cid);
  if(it == end()) {
    col = nullptr;
  } else {
    col = it->second;
    erase(it);
  }
}

void MapMutable::remove(const cid_t cid) {
  Mutex::scope lock(m_mutex);
  auto it = find(cid);
  if(it != end())
    erase(it);
}

size_t MapMutable::size() {
  Mutex::scope lock(m_mutex);

  size_t total = 0;
  for(auto it = begin(); it != end(); ++it)
    total += it->second->size();
  return total;
}

size_t MapMutable::size(const cid_t cid) {
  Mutex::scope lock(m_mutex);

  auto it = find(cid);
  return it == end() ? 0 : it->second->size();
}

size_t MapMutable::size_bytes() {
  Mutex::scope lock(m_mutex);

  size_t total = 0;
  for(auto it = begin(); it != end(); ++it)
    total += it->second->size_bytes();
  return total;
}

size_t MapMutable::size_bytes(const cid_t cid) {
  Mutex::scope lock(m_mutex);

  auto it = find(cid);
  return it == end() ? 0 : it->second->size_bytes();
}

std::string MapMutable::to_string() {
  Mutex::scope lock(m_mutex);
  std::string s("MapMutable(size=");
  s.append(std::to_string(Columns::size()));

  s.append(" map=[");
  for(auto it = begin(); it != end(); ++it) {
    s.append("\n");
    s.append(it->second->to_string());
  }
  s.append("\n])");
  return s;
}



}}}
