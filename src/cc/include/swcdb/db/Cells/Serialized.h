/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_cells_Serialized_h
#define swc_lib_db_cells_Serialized_h

#include "swcdb/core/DynamicBuffer.h"
#include "swcdb/db/Cells/Cell.h"

namespace SWC {  namespace DB {  namespace Cells { 

class Serialized final {
  public:
  
  typedef std::shared_ptr<DB::Cells::Serialized>        Ptr;

  struct ColumnData final {
    Specs::Interval::Ptr interval;
    DynamicBuffer::Ptr   buffer;
  };
  typedef std::unordered_map<int64_t, ColumnData> Columns;
  typedef std::pair<int64_t, ColumnData>          Pair;
  
  Serialized() { }

  Serialized(Columns map): m_map(map) { }

  /*
  Serialized(const int64_t cid, DynamicBuffer::Ptr buff) { 
    add(cid, buff); 
  }
  */

  ~Serialized() {}

  size_t get_size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    
    for(auto it = m_map.begin(); it != m_map.end(); ++it)
      total += it->second.buffer->fill();
    return total;
  }

  size_t get_size(const int64_t cid) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end())
      return (size_t)0;
    return it->second.buffer->fill();
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
    }
  }

  void pop(const int64_t cid, Pair& pair) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it != m_map.end()){
      pair = *it;
      m_map.erase(it);
    } else {
      pair.first = 0;
    }
  }
/*
  void add(const int64_t cid, DynamicBuffer::Ptr buff) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end()){
      m_map.insert(Pair(cid, buff));
    } else {
      it->second->add(buff->base, buff->fill());
    }
  }
*/
  void add(const int64_t cid, Cell& cell) {
    ColumnData data;
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_map.find(cid);
    if(it == m_map.end()){
      data.interval = Specs::Interval::make_ptr();
      data.buffer = std::make_shared<DynamicBuffer>();
      m_map.insert(Pair(cid, data));
    } else
      data = it->second;

    data.buffer->set_mark();
    try{
      cell.write(*data.buffer.get());
      data.interval->expand(cell);
    } catch(...){
      data.buffer->ptr = data.buffer->mark;
      SWC_THROWF(Error::SERIALIZATION_INPUT_OVERRUN, 
                "bad-%s", cell.to_string().c_str());
    }
  }

  private:
  std::mutex   m_mutex;
  Columns      m_map;
};

}}}
#endif