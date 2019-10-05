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


  CommitLog(const DB::RangeBasePtr& range) : m_range(range) { }

  virtual ~CommitLog(){}

  void add(const DB::Cells::Cell& cell) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_frag_current->add(cell);
  }
  
  const std::string get_log_fragment(int64_t frag) {
    std::string s(m_range->get_path("log"));
    s.append("/");
    s.append(std::to_string(frag));
    s.append(".frag");
    return s;
  }
  const std::string get_log_fragment(const std::string& frag) {
    std::string s(m_range->get_path("log"));
    s.append("/");
    s.append(frag);
    return s;
  }

  void load(int &err) {
    //log.data >> file.frag(intervals)

    err = Error::OK;
    FS::DirentList fragments;
    Env::FsInterface::fs()->readdir(err, m_range->get_path("log"), fragments);
    if(err)
      return;

    for(auto entry : fragments) {

      Files::CommitLogFragment::load(
        err, FS::SmartFd::make_ptr(get_log_fragment(entry.name), 0));
      //if(!err)
        //m_fragments.push_back(frag);
      //else 
        //err = Error::OK;
    }
    
  }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("CommitLog(");
    s.append(" fragments=[");
    for(auto frag : m_fragments){
      s.append(frag->to_string());
      s.append(",");
    }
    s.append("]");
    s.append(")");
    return s;
  } 

  void remove(int &err) {}


  private:
  
  std::mutex                  m_mutex;
  const DB::RangeBasePtr      m_range;

  Files::CommitLogFragment::Ptr m_frag_current;

  std::vector<Files::CommitLogFragment::Ptr>  m_fragments;
};


}}}
#endif