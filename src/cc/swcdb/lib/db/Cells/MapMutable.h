/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swcdb_lib_db_Cells_MapMutable_h
#define swcdb_lib_db_Cells_MapMutable_h

#include "Mutable.h"
#include "swcdb/lib/db/Columns/Schema.h"

namespace SWC { namespace DB { namespace Cells {

class MapMutable {
  public:
  
  typedef std::shared_ptr<MapMutable>        Ptr;

  typedef std::unordered_map<int64_t, Mutable::Ptr> Columns;
  typedef std::pair<int64_t, Mutable::Ptr>          ColumnCells;
  
  MapMutable() { }

  virtual ~MapMutable() {}

  bool create(SchemaPtr schema) {
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_map.insert(
      ColumnCells(
        schema->cid, 
        Mutable::make(
          1, 
          schema->cell_versions,
          schema->cell_ttl, 
          schema->col_type
        )
      )
    ).second;
  }

  void add(const int64_t cid, Mutable::Ptr cells) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end())
      m_map.insert(ColumnCells(cid, cells));
    else
      cells->add_to(it->second);
  }
  
  void add(const int64_t cid, const Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end()){
      HT_THROWF(ENOKEY, "Map Missing column=%d (1st do create)", cid);
    }
    it->second->add(cell);
  }

  bool get(size_t offset, ColumnCells& pair) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if(offset < m_map.size()){
      auto it = m_map.begin();
      for(it; offset--; it++);
      pair.first = it->first;
      pair.second = it->second;
      return true;
    }
    return false;
  }

  void get(const int64_t cid, ColumnCells& pair) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it != m_map.end()){
      pair.first = it->first;
      pair.second = it->second;
    } else {
      pair.first = 0;
      pair.second = nullptr;
    }
  }

  void pop(ColumnCells& pair) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.begin();
    if(it != m_map.end()){
      pair.first = it->first;
      pair.second = it->second;
      m_map.erase(it);
    } else {
      pair.first = 0;
      pair.second = nullptr;
    }
  }

  void pop(const int64_t cid, ColumnCells& pair) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it != m_map.end()){
      pair.first = it->first;
      pair.second = it->second;
      m_map.erase(it);
    } else {
      pair.first = 0;
      pair.second = nullptr;
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
      s.append("(cid=");
      s.append(std::to_string(it->first));
      s.append(" ");
      s.append(it->second->to_string());
      s.append("), ");
    }
    s.append("])");
    return s;
  }

  private:
  std::mutex   m_mutex;
  Columns      m_map;
};

}}}
#endif