/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RangeBase_h
#define swcdb_lib_db_Columns_RangeBase_h

#include <mutex>

#include "swcdb/lib/db/Files/RgrData.h"
#include "swcdb/lib/db/Cells/Interval.h"


namespace SWC { namespace DB {


class RangeBase : public std::enable_shared_from_this<RangeBase> {
  public:
  typedef std::shared_ptr<RangeBase> Ptr;
  
  inline static const std::string column_dir = "C"; 
  inline static const std::string range_dir = "/R"; 
  // (swc.fs.path.data)+column_dir+/+{cid}+/range_dir+/+{rid}+/+(types)
  inline static const std::string ranger_data_file = "ranger.data";
  inline static const std::string range_data_file = "range.data";

  inline static const std::string cellstores_dir = "cs";
  inline static const std::string log_dir = "log"; 

  inline static const std::string 
  get_column_path() {
    std::string s(column_dir);
    return s;
  }

  inline static const std::string 
  get_column_path(const int64_t cid) {
    std::string s(get_column_path());
    //if(!s.empty())
    s.append("/");
    FS::set_structured_id(std::to_string(cid), s);
    return s;
  }

  inline static const std::string 
  get_path(const int64_t cid) {
    std::string s(get_column_path(cid));
    s.append(range_dir);
    return s;
  }
  
  inline static const std::string 
  get_path(const int64_t cid, const int64_t rid) {
    std::string s(get_path(cid));
    s.append("/");
    FS::set_structured_id(std::to_string(rid), s);
    s.append("/");
    return s;
  }

  const int64_t     cid;
  const int64_t     rid;

  RangeBase(const int64_t cid, const int64_t rid): 
            cid(cid), rid(rid),
            m_path(get_path(cid, rid))  { }

  RangeBase(const int64_t cid, const int64_t rid, 
            const Cells::Interval& intval)
            : cid(cid), rid(rid), m_interval(intval),
              m_path(get_path(cid, rid))  { }

  virtual ~RangeBase(){}
  
  Ptr shared() {
    return shared_from_this();
  }

  const std::string get_path(const std::string suff) const {
    std::string s(m_path);
    s.append(suff);
    return s;
  }

  const std::string get_path_cs(const int64_t cs_id) const {
    std::string s(m_path);
    s.append(cellstores_dir);
    s.append("/");
    FS::set_structured_id(std::to_string(cs_id), s);
    s.append(".cs");
    return s;
  }

  Files::RgrDataPtr get_last_rgr(int &err) {
    return Files::RgrData::get_rgr(err, get_path(ranger_data_file));
  }

  const Cells::Interval& get_interval() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_interval; // ?return copy
  }

  const std::string to_string() const {
    std::string s("cid=");
    s.append(std::to_string(cid));
    s.append(", rid=");
    s.append(std::to_string(rid));
    s.append(", ");
    s.append(m_interval.to_string());
    return s;
  }

  
  private:
  const std::string         m_path;

  protected:
  std::mutex                m_mutex;
  Cells::Interval           m_interval;
};

}}
#endif