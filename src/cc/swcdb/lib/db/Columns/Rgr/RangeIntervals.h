/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_RangeIntervals_h
#define swcdb_lib_db_Columns_Rgr_RangeIntervals_h

#include "Interval.h"


namespace SWC { namespace server { namespace Rgr {
  
class RangeInterval: public std::enable_shared_from_this<RangeInterval> {

  public:
  typedef std::shared_ptr<RangeInterval>  Ptr;
  DB::Cells::Mutable::Ptr                 cells;
  CellStore::Read::Ptr                    cellstore;

      
  RangeInterval(Files::CellStore::Read::Ptr cellstore)
               : cellstore(cellstore) {
  }

  virtual ~RangeInterval() { }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("RangeInterval(");
    if(cellstore != nullptr)
      s.append(cellstore->to_string());
    return s;
  }

  void scan(Specs::Interval::Ptr intval, cb) {
    
    if(cellstore != nullptr)
      if(!m_block->loaded())
        m_block->load([intval, ptr=shared_from_this()](){ 
          ptr->scan(intval);
        });
    
    for(auto& frag : m_fragments) {
      if(!frag->interval->consist(intval))
        continue;

      if(!frag->loaded())
        frag->load([intval, ptr=shared_from_this()](){ 
          ptr->scan(intval);
        });
    }

    cells->scan(intval, result)
  }

  void 

  private:

  std::mutex                 m_mutex;

  std::vector<Fragment::Ptr> m_fragments;

  //  1 --- 999 ,cs-blk,[log-fragment]

};

class RangeIntervals {

  public:

  typedef std::shared_ptr<RangeIntervals> Ptr;

  RangeIntervals(const DB::RangeBase::Ptr& range, 
                 const Files::CellStore::ReadersPtr& cellstores) 
                : range(range),
                  log(std::make_shared<CommitLog>(m_range)) { 
  
    for(auto& cs : m_cellstores) {
      //sorted
    }              
  }

  virtual ~RangeIntervals() { }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("Intervals([");
    for(auto& intval : m_intervals){
      s.append(intval->to_string());
      s.append(", ");
    }
    return s;
  }
  

  void expand(DB::Cells::Interval::Ptr interval) {
    std::lock_guard<std::mutex> lock(m_mutex);

    bool init = false;
    for(auto& cs : m_cellstores) {
      interval->expand(cs->interval, init);
      init=true;
    }
  }
  
  void add(const Cell& cell) {
    log->add(cell);

    Files::CellStore::Read::Ptr cellstore =  = nullptr;
    
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto& cs : m_cellstores) {
        if(!cs->includes(cell))
          continue;
        cellstore = cs;
        break;
      }
    }
    DB::Cells::Interval::Ptr interval = cellstore->get_block_interval(cell);

    RangeInterval::Ptr interval = nullptr

    if(interval == nullptr) {
      std::lock_guard<std::mutex> lock(m_mutex);
      interval = m_intervals->back();
      HT_WARNF("Using last RangeIntervals %s for cell %s", 
                interval->to_string().c_str(), 
                cell.key.to_string().c_str());
    }

    interval->cells->add(cell);
  }

  private:
  const DB::RangeBase::Ptr          range
  CommitLog::Ptr                    log;

  std::mutex                        m_mutex;
  std::vector<RangeInterval::Ptr>   m_intervals;
  std::vector<BlocksInterval::Ptr>  m_intervals;

};


/*
Range::intervals >>
  Interval cs

*/
}}}

#endif