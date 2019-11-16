/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_Block_h
#define swcdb_db_Cells_Block_h

#include "Mutable.h"
#include "Interval.h"


namespace SWC { namespace DB { namespace Cells {  

class Block {
  public:
  typedef Block* Ptr;
  
  Block(const Interval& interval, const Schema::Ptr s) 
        : m_interval(interval),  
          m_cells(Mutable(0, s->cell_versions, s->cell_ttl, s->col_type)) {
  }

  virtual void splitter() { }

  virtual void loaded_cellstores(int err) { }

  virtual void loaded_logs(int err) { }
  
  virtual ~Block() {
    //std::cout << " ~Block\n";
  }

  bool is_next(const Specs::Interval::Ptr spec) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return (spec->offset_key.empty() || m_interval.is_in_end(spec->offset_key))
            && m_interval.includes(spec);
  }

  bool includes(const Specs::Interval::Ptr spec) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_interval.includes(spec);
  }
  
  const bool is_consist(const Interval& intval) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return intval.consist(m_interval);
  }

  const bool is_include(const Interval& intval) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return intval.includes(m_interval);
  }

  const size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cells.size;
  }

  const size_t _size() {
    return m_cells.size;
  }
  
  const size_t size_bytes() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cells.size_bytes;
  }
  
  void load_cells(const Mutable::Ptr& cells) {
    std::lock_guard<std::mutex> lock(m_mutex);
    cells->scan(m_interval, m_cells);
  }

  size_t load_cells(const uint8_t* ptr, size_t remain, bool& was_splitted) {
    Cell cell;
    size_t count = 0;
    size_t added = 0;
    uint32_t sz = 0;
    
    auto ts = Time::now_ns();
    std::lock_guard<std::mutex> lock(m_mutex);

    bool synced = !m_cells.size;
    while(remain) {
      try {
        cell.read(&ptr, &remain);
        count++;
      } catch(std::exception) {
        HT_ERRORF(
          "Cell trunclated at count=%llu remain=%llu %s, %s", 
          count, remain, cell.to_string().c_str(),  _to_string().c_str());
        break;
      }
      
      if(!m_interval.key_end.empty() 
          && m_interval.key_end.compare(cell.key) == Condition::GT)
        break;
      
      if(!m_interval.key_begin.empty() 
          && m_interval.key_begin.compare(cell.key) == Condition::LT)
        continue;

      //if(!m_interval.consist(cell.key))
        //continue;

      if(synced)
        m_cells.push_back(cell);
      else
        m_cells.add(cell);
      
      added++;
      
      sz = m_cells.size;
      splitter();
      if(!was_splitted && sz != m_cells.size)
        was_splitted = true;
    }

    auto took = Time::now_ns()-ts;
    std::cout << "Cells::Block::load_cells took=" << took
              << " synced=" << synced
              << " avg=" << (added>0 ? took / added : 0)
              << " " << m_cells.to_string() << "\n";
    return added;
  }

  void free_key_begin() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_interval.key_begin.free();
  }

  void free_key_end() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_interval.key_end.free();
  }

  
  protected:
  std::mutex              m_mutex;
  Interval                m_interval;
  Mutable                 m_cells;
};

}}}

#endif