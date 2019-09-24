/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_cells_Serialized_h
#define swc_lib_db_cells_Serialized_h

#include "swcdb/lib/core/DynamicBuffer.h"
#include "Cell.h"

namespace SWC {  namespace DB {  namespace Cells { 

class Serialized {
  public:
  
  typedef std::shared_ptr<DB::Cells::Serialized>        Ptr;
  typedef std::unordered_map<int64_t, DynamicBufferPtr> Columns;
  typedef std::pair<int64_t, DynamicBufferPtr>          Pair;
  
  Serialized() { }

  Serialized(Columns map): m_map(map) { }

  Serialized(const int64_t cid, DynamicBufferPtr buff) { 
    add(cid, buff); 
  }

  virtual ~Serialized() {}

  size_t get_size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    
    for(auto it = m_map.begin(); it != m_map.end(); ++it)
      total += it->second->fill();
    return total;
  }

  size_t get_size(const int64_t cid) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end())
      return (size_t)0;
    return it->second->fill();
  }

  void pop(Pair& pair) {
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

  void pop(const int64_t cid, DynamicBufferPtr& buff) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it != m_map.end()){
      buff = it->second;
      m_map.erase(it);
    } else {
      buff = nullptr;
    }
  }

  void add(const int64_t cid, DynamicBufferPtr buff) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end()){
      m_map.insert(Pair(cid, buff));
    } else {
      it->second->add(buff->base, buff->fill());
    }
  }

  void add(const int64_t cid, Cell& cell) {
    DynamicBufferPtr buff;
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end()){
      buff = std::make_shared<DynamicBuffer>();
      m_map.insert(Pair(cid, buff));
    } else
      buff = it->second;

    buff->set_mark();
    try{
      cell.write(*buff.get());
    } catch(...){
      buff->ptr = buff->mark;
      HT_THROWF(Error::SERIALIZATION_INPUT_OVERRUN, 
                "bad-%s", cell.to_string().c_str());
    }
  }

  private:
  std::mutex   m_mutex;
  Columns      m_map;
};

}}}
#endif