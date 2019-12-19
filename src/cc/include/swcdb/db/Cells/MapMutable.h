/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swcdb_lib_db_Cells_MapMutable_h
#define swcdb_lib_db_Cells_MapMutable_h

#include "swcdb/db/Cells/Mutable.h"
#include "swcdb/db/Columns/Schema.h"

namespace SWC { namespace DB { namespace Cells {

class ColCells final {

  public:
  typedef std::shared_ptr<ColCells> Ptr;

  static Ptr make(const int64_t cid, uint32_t versions, uint32_t ttl, 
                  Types::Column type) {
    return std::make_shared<ColCells>(cid, versions, ttl, type);
  }

  const int64_t cid;

  ColCells(const int64_t cid, uint32_t versions, uint32_t ttl, 
           Types::Column type)
          : cid(cid), m_cells(0, versions, ttl, type) { 

  }

  ~ColCells() {}

  DB::Cell::Key::Ptr get_first_key() {
    auto key = std::make_shared<DB::Cell::Key>();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cells.get(0, *key.get()); 
    return key;
  }
  
  DB::Cell::Key::Ptr get_key_next(const DB::Cell::Key& eval_key, 
                                  bool start_key=false) {
    auto key = std::make_shared<DB::Cell::Key>();
    std::lock_guard<std::mutex> lock(m_mutex);
    if(eval_key.empty() || 
      !m_cells.get(
        eval_key, start_key? Condition::GE : Condition::GT, *key.get()))
      return nullptr;
    return key;
  }

  DynamicBuffer::Ptr get_buff(const DB::Cell::Key& key_start, 
                              const DB::Cell::Key&  key_end, 
                              size_t buff_sz, bool& more) {
    auto cells_buff = std::make_shared<DynamicBuffer>();
    std::lock_guard<std::mutex> lock(m_mutex);
    more = m_cells.write_and_free(
      key_start, key_end, *cells_buff.get(), buff_sz);
    if(cells_buff->fill())
      return cells_buff;
    return nullptr;
  }

  void add(const SWC::DB::Cells::Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cells.add(cell);
  }

  void add(const DynamicBuffer& cells) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cells.add(cells);
  }

  const size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cells.size();
  }

  const size_t size_bytes() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cells.size_bytes();
  }

  const std::string to_string() {
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

  private:
  std::mutex   m_mutex;
  Mutable      m_cells;

};

class MapMutable {
  public:
  
  typedef std::shared_ptr<MapMutable>                 Ptr;
  typedef std::unordered_map<int64_t, ColCells::Ptr>  Columns;
  
  MapMutable() { }

  virtual ~MapMutable() {}

  const bool create(Schema::Ptr schema) {
    return create(
      schema->cid, schema->cell_versions, schema->cell_ttl, schema->col_type);
  }

  const bool create(int64_t cid, uint32_t versions, uint32_t ttl, 
                    Types::Column type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if(m_map.find(cid) != m_map.end())
      return false;

    return m_map.insert(
      std::make_pair(cid, ColCells::make(cid, versions, ttl, type))
    ).second;
  }

  const bool exists(int64_t cid) {
    std::lock_guard<std::mutex> lock(m_mutex);
  
    return m_map.find(cid) != m_map.end();
  }
  /*
  void add(const int64_t cid, Mutable::Ptr cells) {

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end())
      m_map.insert(ColumnCells(cid, cells));
    else
      cells->add_to(it->second);
  }
  */
  
  void add(const int64_t cid, const Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end()){
      HT_THROWF(ENOKEY, "Map Missing column=%d (1st do create)", cid);
    }
    it->second->add(cell);
  }

  ColCells::Ptr get_idx(size_t offset) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if(offset < m_map.size()) {
      auto it = m_map.begin();
      for(it; offset--; it++);
      return it->second;
    }
    return nullptr;
  }

  ColCells::Ptr get_col(const int64_t cid) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    return it != m_map.end() ? it->second : nullptr;
  }

  void pop(ColCells::Ptr& col) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.begin();
    if(it != m_map.end()) {
      col = it->second;
      m_map.erase(it);
    } else {
      col = nullptr;
    }
  }

  void pop(const int64_t cid, ColCells::Ptr& col) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it != m_map.end()){
      col = it->second;
      m_map.erase(it);
    } else {
      col = nullptr;
    }
  }

  void remove(const int64_t cid) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_map.find(cid);
    if(it != m_map.end())
      m_map.erase(it);
  }

  const size_t size_bytes() {
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t total = 0;
    for(auto it = m_map.begin(); it != m_map.end(); ++it)
      total += it->second->size_bytes();
    return total;
  }

  const size_t size_bytes(const int64_t cid) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end())
      return (size_t)0;
    return it->second->size_bytes();
  }

  const std::string to_string() {
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

  private:
  std::mutex   m_mutex;
  Columns      m_map;
};

}}}
#endif