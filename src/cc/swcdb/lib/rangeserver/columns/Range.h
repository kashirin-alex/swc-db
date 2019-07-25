/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_rs_Range_h
#define swcdb_lib_rs_Range_h

#include "Schema.h"

#include <mutex>
#include <memory>
#include <unordered_map>


namespace SWC {


class Range : public std::enable_shared_from_this<Range> {
  public:
  
  inline static const std::string range_dir = "/range"; // .../a-cid/range/a-rid/(types)

  inline static std::string get_path(int64_t cid, int64_t rid=0){
    std::string s(std::to_string(cid));
    s.append(range_dir);
    if(rid > 0) {
      s.append("/");
      s.append(std::to_string(rid));
      s.append("/");
    }
    return s;
  }


  Range(FS::InterfacePtr fs, 
        int64_t cid, int64_t rid, SchemaPtr schema): 
        m_fs(fs), m_cid(cid), m_rid(rid),
        m_path(get_path(cid, rid)),
        m_schema(schema), loaded(false){
    
    m_err = 0;
    if(!m_fs->get_fs()->exists(m_err, m_path)){
      // init - 1st range
      m_fs->get_fs()->mkdirs(m_err, get_path("rs"));
      m_fs->get_fs()->mkdirs(m_err, get_path("log"));
      m_fs->get_fs()->mkdirs(m_err, get_path("cs"));
    } 
    
    if(m_err == 0) {
      if(m_fs->get_fs()->exists(m_err, get_path("rs/last.data"))) {
        // read last fs, 
        // if not this rs(endpoints)  
        //  if online,
              // req. unload (sync)
      } 
      // create rs/last.data
    }

  }

  virtual ~Range(){}

  int has_err(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_err;
  }

  std::string get_path(std::string suff){
    std::string s(m_path);
    s.append(suff);
    return s;
  }

  const int64_t range_id(){
    return m_rid;
  }

  private:

  std::mutex        m_mutex;
  int               m_err;
  FS::InterfacePtr  m_fs;
  const int64_t     m_cid;
  const int64_t     m_rid;
  const std::string m_path;
  SchemaPtr         m_schema;
  bool              loaded;
};
typedef std::shared_ptr<Range> RangePtr;




}
#endif