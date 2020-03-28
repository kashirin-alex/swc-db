/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 


#include "swcdb/db/Cells/MapMutable.h"
#include <cassert>


namespace SWC { namespace DB { namespace Cells {


ColCells::Ptr ColCells::make(const int64_t cid, uint32_t versions, 
                             uint32_t ttl, Types::Column type) {
  return std::make_shared<ColCells>(cid, versions, ttl, type);
}

ColCells::Ptr ColCells::make(const int64_t cid, Mutable& cells) {
  return std::make_shared<ColCells>(cid, cells);
}

ColCells::ColCells(const int64_t cid, uint32_t versions, uint32_t ttl, 
                   Types::Column type)
                  : cid(cid), m_cells(versions, ttl*1000000000, type) { 
}

ColCells::ColCells(const int64_t cid, Mutable& cells)
        : cid(cid), m_cells(cells) { 
}

ColCells::~ColCells() {}

DB::Cell::Key::Ptr ColCells::get_first_key() {
  auto key = std::make_shared<DB::Cell::Key>();
  std::lock_guard<std::mutex> lock(m_mutex);
  assert(m_cells.size()); // bad call , assure size pre-check
  m_cells.get(0, *key.get()); 
  return key;
}

DB::Cell::Key::Ptr ColCells::get_key_next(const DB::Cell::Key& eval_key, 
                                          bool start_key) {
  auto key = std::make_shared<DB::Cell::Key>();
  std::lock_guard<std::mutex> lock(m_mutex);
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
  std::lock_guard<std::mutex> lock(m_mutex);
  more = m_cells.write_and_free(
    key_start, key_end, *cells_buff.get(), buff_sz);
  if(cells_buff->fill())
    return cells_buff;
  return nullptr;
}

void ColCells::add(const DB::Cells::Cell& cell) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_cells.add_raw(cell);
}

size_t ColCells::add(const DynamicBuffer& cells) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto sz = m_cells.size();
  m_cells.add_raw(cells);
  return m_cells.size() - sz;
}

size_t ColCells::add(const DynamicBuffer& cells, 
                     const DB::Cell::Key& upto_key, 
                     const DB::Cell::Key& from_key) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto sz = m_cells.size();
  m_cells.add_raw(cells, upto_key, from_key);
  return m_cells.size() - sz;
}

const size_t ColCells::size() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_cells.size();
}

const size_t ColCells::size_bytes() {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_cells.size_bytes();
}

const std::string ColCells::to_string() {
  std::string s("(cid=");
  s.append(std::to_string(cid));
  s.append(" ");
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    s.append(m_cells.to_string());
  }
  s.append(")");
  return s;
}





MapMutable::MapMutable() { }

MapMutable::~MapMutable() {}

const bool MapMutable::create(Schema::Ptr schema) {
  return create(
    schema->cid, schema->cell_versions, schema->cell_ttl, schema->col_type);
}

const bool MapMutable::create(int64_t cid, uint32_t versions, uint32_t ttl, 
                              Types::Column type) {
  std::lock_guard<std::mutex> lock(m_mutex);
  
  if(m_map.find(cid) != m_map.end())
    return false;

  return m_map.emplace(cid, ColCells::make(cid, versions, ttl, type)).second;
}

const bool MapMutable::create(const int64_t cid, Mutable& cells) {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_map.emplace(cid, ColCells::make(cid, cells)).second;
}

const bool MapMutable::exists(int64_t cid) {
  std::lock_guard<std::mutex> lock(m_mutex);

  return m_map.find(cid) != m_map.end();
}

void MapMutable::add(const int64_t cid, const Cell& cell) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_map.find(cid);
  if(it == m_map.end()){
    SWC_THROWF(ENOKEY, "Map Missing column=%d (1st do create)", cid);
  }
  it->second->add(cell);
}

ColCells::Ptr MapMutable::get_idx(size_t offset) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if(offset < m_map.size()) {
    auto it = m_map.begin();
    for(it; offset--; ++it);
    return it->second;
  }
  return nullptr;
}

ColCells::Ptr MapMutable::get_col(const int64_t cid) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_map.find(cid);
  return it != m_map.end() ? it->second : nullptr;
}

void MapMutable::pop(ColCells::Ptr& col) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_map.begin();
  if(it != m_map.end()) {
    col = it->second;
    m_map.erase(it);
  } else {
    col = nullptr;
  }
}

void MapMutable::pop(const int64_t cid, ColCells::Ptr& col) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_map.find(cid);
  if(it != m_map.end()){
    col = it->second;
    m_map.erase(it);
  } else {
    col = nullptr;
  }
}

void MapMutable::remove(const int64_t cid) {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_map.find(cid);
  if(it != m_map.end())
    m_map.erase(it);
}

const size_t MapMutable::size() {
  std::lock_guard<std::mutex> lock(m_mutex);

  size_t total = 0;
  for(auto it = m_map.begin(); it != m_map.end(); ++it)
    total += it->second->size();
  return total;
}

const size_t MapMutable::size(const int64_t cid) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_map.find(cid);
  if(it == m_map.end())
    return (size_t)0;
  return it->second->size();
}

const size_t MapMutable::size_bytes() {
  std::lock_guard<std::mutex> lock(m_mutex);

  size_t total = 0;
  for(auto it = m_map.begin(); it != m_map.end(); ++it)
    total += it->second->size_bytes();
  return total;
}

const size_t MapMutable::size_bytes(const int64_t cid) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_map.find(cid);
  if(it == m_map.end())
    return (size_t)0;
  return it->second->size_bytes();
}

const std::string MapMutable::to_string() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::string s("MapMutable(size=");
  s.append(std::to_string(m_map.size()));

  s.append(" map=[");
  for(auto it = m_map.begin(); it != m_map.end(); ++it){
    s.append("\n");
    s.append(it->second->to_string());
  }
  s.append("\n])");
  return s;
}



}}}
