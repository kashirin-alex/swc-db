/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_RangeBlock_h
#define swcdb_db_Cells_RangeBlock_h

#include "swcdb/db/Cells/Mutable.h"
#include "swcdb/db/Cells/Interval.h"


namespace SWC { namespace DB { namespace Cells {  

class RangeBlock {
  public:
  typedef RangeBlock* Ptr;
  
  RangeBlock(const Interval& interval, const Schema::Ptr s) 
        : m_interval(interval),  
          m_cells(Mutable(0, s->cell_versions, s->cell_ttl, s->col_type)) {
  }

  RangeBlock(const Interval& interval, 
        uint32_t cell_versions, uint32_t cell_ttl, Types::Column col_type)
        : m_interval(interval),  
          m_cells(Mutable(0, cell_versions, cell_ttl, col_type)) {
  }

  virtual ~RangeBlock() { }

  virtual Ptr ptr() {
    return this;
  }

  virtual const bool _is_gt_prev_end(const DB::Cell::Key& key) = 0;

  virtual const bool is_consist(const Interval& intval) = 0;
  
  virtual const bool splitter() = 0;

  virtual void loaded_cellstores(int err) = 0;

  virtual void loaded_logs(int err) = 0;
  
  const bool is_in_end(const DB::Cell::Key& key) {
    std::shared_lock lock(m_mutex);
    return m_interval.is_in_end(key);
  }

  const bool is_gt_end(const DB::Cell::Key& key) {
    std::shared_lock lock(m_mutex);
    return m_interval.key_end.compare(key) == Condition::GT;
  }

  const bool is_next(const Specs::Interval& spec) {
    std::shared_lock lock(m_mutex);
    return (spec.offset_key.empty() || m_interval.is_in_end(spec.offset_key))
            && m_interval.includes(spec);
  }

  const bool includes(const Specs::Interval& spec) {
    std::shared_lock lock(m_mutex);
    return m_interval.includes(spec);
  }

  const size_t size() {
    std::shared_lock lock(m_mutex);
    return m_cells.size();
  }

  const size_t _size() {
    return m_cells.size();
  }
  
  const size_t size_bytes() {
    std::shared_lock lock(m_mutex);
    return m_cells.size_bytes() + m_cells.size() * m_cells._cell_sz;
  }
  
  void load_cells(const Mutable& cells) {
    std::scoped_lock lock(m_mutex);
    auto ts = Time::now_ns();
    size_t added = m_cells.size();
    
    if(cells.size())
      cells.scan(m_interval, m_cells);
    
    if(m_cells.size() && !m_interval.key_begin.empty())
      m_cells.expand_begin(m_interval);

    added = m_cells.size() - added;
    auto took = Time::now_ns()-ts;
    std::cout << "RangeBlock::load_cells(cells)"
              << " synced=0"
              << " avail=" << cells.size() 
              << " added=" << added 
              << " skipped=" << cells.size()-added
              << " avg=" << (added>0 ? took / added : 0)
              << " took=" << took
              << std::flush << " " << m_cells.to_string() << "\n";
    
  }

  const size_t load_cells(const uint8_t* buf, size_t remain, 
                          size_t avail, bool& was_splitted) {
    auto ts = Time::now_ns();
    Cell cell;
    size_t count = 0;
    size_t added = 0;
    
    const uint8_t** rbuf = &buf;
    size_t* remainp = &remain;

    std::scoped_lock lock(m_mutex);
    bool synced = !m_cells.size();
    
    while(remain) {
      try {
        cell.read(rbuf, remainp);
        count++;
      } catch(std::exception) {
        SWC_LOGF(LOG_ERROR, 
          "Cell trunclated at count=%llu remain=%llu %s, %s", 
          count, avail-count, 
          cell.to_string().c_str(),  m_interval.to_string().c_str());
        break;
      }
      
      if(!_is_gt_prev_end(cell.key))
        continue;
      if(!m_interval.key_end.empty() 
          && m_interval.key_end.compare(cell.key) == Condition::GT)
        break;

      if(synced)
        m_cells.push_back(cell);
      else
        m_cells.add(cell);
      
      added++;

      if(splitter() && !was_splitted)
        was_splitted = true;
    }
    
    if(m_cells.size() && !m_interval.key_begin.empty())
      m_cells.expand_begin(m_interval);
    
    auto took = Time::now_ns()-ts;
    std::cout << "RangeBlock::load_cells(rbuf)"
              << " synced=" << synced 
              << " avail=" << avail 
              << " added=" << added 
              << " skipped=" << avail-added
              << " avg=" << (added>0 ? took / added : 0)
              << " took=" << took
              << std::flush << " " << m_cells.to_string() << "\n";
             
    return added;
  }

  void free_key_begin() {
    std::scoped_lock lock(m_mutex);
    m_interval.key_begin.free();
  }

  void free_key_end() {
    std::scoped_lock lock(m_mutex);
    m_interval.key_end.free();
  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);
    std::string s("RangeBlock(");
    s.append(m_interval.to_string());
    s.append(" ");
    s.append(m_cells.to_string());
    s.append(")");
    return s;
  }
  
  protected:
  std::shared_mutex       m_mutex;
  Interval                m_interval;
  Mutable                 m_cells;

};

}}}

#endif