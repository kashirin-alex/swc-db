/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_Compaction_h
#define swcdb_lib_db_Columns_Rgr_Compaction_h

#include "Columns.h"


namespace SWC { namespace server { namespace Rgr {

class Compaction {
  public:

  typedef std::shared_ptr<Compaction> Ptr;

  Compaction(uint32_t workers=1): workers(workers), running(0), m_idx_cid(0), m_idx_rid(0)   {}

  virtual ~Compaction(){}
  
  void run() {
    
    if(running == workers)
      return;
    running++;

    ColumnPtr col   = nullptr;
    RangePtr  range = nullptr;

    for(;;) {
      { 
        std::lock_guard<std::mutex> lock(m_mutex);
        col = Env::RgrColumns::get()->get_next(m_idx_cid);
        if(col == nullptr) {
          m_idx_cid = 0;
          return;
        }

        if(col->removing()){
          m_idx_cid++;
          continue;
        }

        range = col->get_next(m_idx_rid);    
        if(range == nullptr) {
          m_idx_cid++;
          continue;
        }

        if(!range->is_loaded())
          continue;
      }
    
      compact(range);
    }

    running--;
  }

  void compact(RangePtr range) {

  }

  private:
  std::atomic<uint32_t>   running;
  std::mutex              m_mutex;
  uint32_t                m_workers;

  size_t                  m_idx_cid;
  size_t                  m_idx_rid;
};



}}}
#endif