/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RangeBase_h
#define swcdb_lib_db_Columns_RangeBase_h

#include "Schema.h"
#include <mutex>

#include "swcdb/lib/db/Files/RsData.h"


namespace SWC { namespace DB {

class RangeBase;
typedef std::shared_ptr<RangeBase> RangeBasePtr;

enum RangeState{
  NOTLOADED,
  LOADED,
  DELETED
};

class RangeBase : public std::enable_shared_from_this<RangeBase> {
  public:
  
  inline static const std::string range_dir = "/range"; // .../a-cid/range/a-rid/(types)
  inline static const std::string rs_data_file = "last_rs.data";

  inline static std::string get_path(int64_t cid, int64_t rid=0){
    std::string s;
    FS::set_structured_id(std::to_string(cid), s);
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
            m_path(get_path(cid, rid)), 
            state(RangeState::NOTLOADED) { }

  virtual ~RangeBase(){}
  
  RangeBasePtr shared() {
    return shared_from_this();
  }

  std::string get_path(std::string suff){
    std::string s(m_path);
    s.append(suff);
    return s;
  }

  Files::RsDataPtr get_last_rs(){
    return Files::RsData::get_rs(get_path(rs_data_file));
  }

  bool is_loaded(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return state == RangeState::LOADED;
  }

  bool deleted(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return state == RangeState::DELETED;
  }

  void set_loaded(RangeState new_state){
    std::lock_guard<std::mutex> lock(m_mutex);
    state = new_state;
  }

  private:
  const std::string m_path;

  RangeState state;
  
};




}}
#endif