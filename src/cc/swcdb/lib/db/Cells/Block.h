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

  virtual void splitter() = 0;

  virtual void loaded_cellstores(int err) = 0;

  virtual void loaded_logs(int err) = 0;
  
  virtual Ptr ptr() {
    return this;
  }

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
  
  void load_cells(const Mutable& cells) {
    std::lock_guard<std::mutex> lock(m_mutex);
    //auto ts = Time::now_ns();
    //size_t added = m_cells.size;

    cells.scan(m_interval, m_cells);
    /*
    added = m_cells.size - added;
    auto took = Time::now_ns()-ts;
    std::cout << "Cells::Block::load_cells(cells)"
              << " synced=0"
              << " avail=" << cells.size 
              << " added=" << added 
              << " skipped=" << cells.size-added
              << " avg=" << (added>0 ? took / added : 0)
              << " took=" << took
              << " " << m_cells.to_string() << "\n";
    */
  }

  size_t load_cells(const uint8_t* rbuf, size_t remain, 
                    size_t avail, bool& was_splitted) {
    Cell cell;
    size_t count = 0;
    size_t added = 0;
    uint32_t sz = 0;
    
    //auto ts = Time::now_ns();
    std::lock_guard<std::mutex> lock(m_mutex);

    bool synced = !m_cells.size;
    while(remain) {
      try {
        cell.read(&rbuf, &remain);
        count++;
      } catch(std::exception) {
        HT_ERRORF(
          "Cell trunclated at count=%llu remain=%llu %s, %s", 
          count, avail-count, 
          cell.to_string().c_str(),  m_interval.to_string().c_str());
        break;
      }

      if(!m_interval.key_begin.empty() 
          && m_interval.key_begin.compare(cell.key) == Condition::LT)
        continue;
      if(!m_interval.key_end.empty() 
          && m_interval.key_end.compare(cell.key) == Condition::GT)
        break;
      
      //if(!m_interval.consist(cell.key))
        //continue;

      if(synced)
        m_cells.push_back(cell);
      else
        m_cells.add(cell);
      
      added++;

      if((sz = m_cells.size) < 200000)
        continue;

      splitter();
      if(!was_splitted && sz != m_cells.size)
        was_splitted = true;
    }
    /*
    auto took = Time::now_ns()-ts;
    std::cout << "Cells::Block::load_cells(rbuf)"
              << " synced=" << synced 
              << " avail=" << avail 
              << " added=" << added 
              << " skipped=" << avail-added
              << " avg=" << (added>0 ? took / added : 0)
              << " took=" << took
              << " " << m_cells.to_string() << "\n";
    */          
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

  const std::string to_string() {
    std::string s("Cells::Block(");
    s.append(m_interval.to_string());
    s.append(" ");
    s.append(m_cells.to_string());
    s.append(")");
    return s;
  }
  
  protected:
  std::mutex              m_mutex;
  Interval                m_interval;
  Mutable                 m_cells;
};

}}}

#endif