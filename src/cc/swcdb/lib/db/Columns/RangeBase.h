/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RangeBase_h
#define swcdb_lib_db_Columns_RangeBase_h

#include <mutex>

#include "swcdb/lib/db/Files/RsData.h"


namespace SWC { namespace DB {

class RangeBase;
typedef std::shared_ptr<RangeBase> RangeBasePtr;



class RangeBase : public std::enable_shared_from_this<RangeBase> {
  public:
  
  inline static const std::string range_dir = "/range"; // .../a-cid/range/a-rid/(types)
  inline static const std::string rs_data_file = "last_rs.data";

  inline static std::string get_column_path(int64_t cid){
    std::string s;
    FS::set_structured_id(std::to_string(cid), s);
    return s;
  }
  inline static std::string get_path(int64_t cid, int64_t rid=0){
    std::string s(get_column_path(cid));
    s.append(range_dir);
    if(rid > 0) {
      s.append("/");
      FS::set_structured_id(std::to_string(rid), s);
      s.append("/");
    }
    return s;
  }

  const int64_t     cid;
  const int64_t     rid;
  std::mutex        m_mutex;

  RangeBase(int64_t cid, int64_t rid): 
            cid(cid), rid(rid),
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

  Files::RsDataPtr get_last_rs(int &err){
    return Files::RsData::get_rs(err, get_path(rs_data_file));
  }

  std::string to_string(){
    std::string s("cid=");
    s.append(std::to_string(cid));
    s.append(", rid=");
    s.append(std::to_string(rid));
    return s;
  }

  private:
  const std::string m_path;

  
};




}}
#endif