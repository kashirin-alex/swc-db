/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RangeBase_h
#define swcdb_lib_db_Columns_RangeBase_h

#include <mutex>

#include "swcdb/lib/db/Files/RgrData.h"
#include "swcdb/lib/db/Cells/Intervals.h"


namespace SWC { namespace DB {

class RangeBase;
typedef std::shared_ptr<RangeBase> RangeBasePtr;

class RangeBase : public std::enable_shared_from_this<RangeBase> {
  public:
  
  inline static const std::string column_dir = "col"; 
  inline static const std::string range_dir = "/range"; 
  // (swc.fs.path.data)+column_dir+/+{cid}+/range_dir+/+{rid}+/+(types)
  inline static const std::string rs_data_file = "ranger.data";

  inline static const std::string get_column_path(){
    std::string s(column_dir);
    return s;
  }

  inline static const std::string get_column_path(int64_t cid){
    std::string s(get_column_path());
    //if(!s.empty())
    s.append("/");
    FS::set_structured_id(std::to_string(cid), s);
    return s;
  }

  inline static const std::string get_path(int64_t cid){
    std::string s(get_column_path(cid));
    s.append(range_dir);
    return s;
  }
  
  inline static const std::string get_path(int64_t cid, int64_t rid){
    std::string s(get_path(cid));
    s.append("/");
    FS::set_structured_id(std::to_string(rid), s);
    s.append("/");
    return s;
  }

  const int64_t     cid;
  const int64_t     rid;

  RangeBase(int64_t cid, int64_t rid): 
            cid(cid), rid(rid),
            m_path(get_path(cid, rid))  { }

  RangeBase(int64_t cid, int64_t rid, Cells::Intervals::Ptr intvals): 
            cid(cid), rid(rid), m_intervals(intvals),
            m_path(get_path(cid, rid))  { }

  virtual ~RangeBase(){}
  
  RangeBasePtr shared() {
    return shared_from_this();
  }

  std::string get_path(std::string suff){
    std::string s(m_path);
    s.append(suff);
    return s;
  }

  Files::RgrDataPtr get_last_rgr(int &err){
    return Files::RgrData::get_rgr(err, get_path(rs_data_file));
  }

  const Cells::Intervals::Ptr& get_intervals() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_intervals;
  }

  std::string to_string(){
    std::string s("cid=");
    s.append(std::to_string(cid));
    s.append(", rid=");
    s.append(std::to_string(rid));
    if(m_intervals != nullptr) {
      s.append(", ");
      s.append(m_intervals->to_string());
    }
    return s;
  }

  
  private:
  const std::string         m_path;

  protected:
  std::mutex                m_mutex;
  Cells::Intervals::Ptr     m_intervals;
};

}}
#endif