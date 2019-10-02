/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_CommitLog_h
#define swcdb_lib_db_Columns_Rgr_CommitLog_h

#include "swcdb/lib/db/Files/CommitLogFragment.h"

namespace SWC { namespace server { namespace Rgr {


class CommitLog {
  public:

  typedef std::shared_ptr<CommitLog>  Ptr;
  DB::Cells::Interval::Ptr           interval;


  CommitLog(const DB::RangeBasePtr& range)
            : m_range(range), m_next_frag(0),
              interval(std::make_shared<DB::Cells::Interval>()) { }

  virtual ~CommitLog(){}

  
  const std::string get_log(int64_t log){
    std::string s(m_range->get_path("log"));
    s.append("/");
    FS::set_structured_id(std::to_string(log), s);
    s.append(".frag");
    return s;
  }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("CommitLog(");
    s.append(interval->to_string());
    s.append(" fragments=[");
    for(auto frag : m_fragments){
      s.append(std::to_string(frag));
      s.append(",");
    }
    s.append("]");

    if(m_smartfd != nullptr) { // current fragment
      s.append(" ");
      s.append(m_smartfd->to_string());
    }
    s.append(")");
    return s;
  } 


  void load(int &err) {
    FS::IdEntries_t entries;
    Env::FsInterface::interface()->get_structured_ids(
      err, m_range->get_path("log"), entries);
    for(auto frag : entries) {
      if(frag > m_next_frag)
        m_next_frag = frag;

      Files::CommitLogFragment::load(err, FS::SmartFd::make_ptr(get_log(frag), 0));
      if(!err)
        m_fragments.push_back(frag);
      else 
        err = Error::OK;
    }
    
  }

  void remove(int &err) {}


  private:
  
  std::mutex              m_mutex;
  const DB::RangeBasePtr  m_range;

  std::vector<int64_t>    m_fragments;
  int64_t                 m_next_frag;

  FS::SmartFdPtr          m_smartfd = nullptr;

};


}}}
#endif