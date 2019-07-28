/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Range_h
#define swcdb_lib_db_Columns_Range_h

#include "Schema.h"

#include <mutex>
#include <memory>
#include <unordered_map>


namespace SWC {

struct RsData {
  EndPoints endpoints;
};

class Range : public std::enable_shared_from_this<Range> {
  public:
  
  inline static const std::string range_dir = "/range"; // .../a-cid/range/a-rid/(types)

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

  Range(FS::InterfacePtr fs, 
        int64_t cid, int64_t rid, SchemaPtr schema): 
        m_fs(fs), cid(cid), rid(rid),
        m_path(get_path(cid, rid)),
        m_schema(schema), loaded(false){
  }

  virtual ~Range(){}

  bool load(){    
    std::cout << "LOAD RANGE cid=" << cid << " rid=" << rid << "\n";
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
      m_err = 0;
      // create rs/last.data
    }
    set_loaded(m_err == 0);
    return m_err == 0;
  }

  RsData get_last_rs(){
    RsData rs_data;
    if(!m_fs->get_fs()->exists(m_err, get_path("rs/last.data")))
      return rs_data;
    
    return rs_data;
  }

  void set_loaded(bool state){
    std::lock_guard<std::mutex> lock(m_mutex);
    loaded = state;
  }
  bool is_loaded(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return loaded;
  }
  
  int has_err(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_err;
  }

  std::string get_path(std::string suff){
    std::string s(m_path);
    s.append(suff);
    return s;
  }

  private:

  std::mutex        m_mutex;
  int               m_err;
  FS::InterfacePtr  m_fs;
  const std::string m_path;
  SchemaPtr         m_schema;
  bool              loaded;
};
typedef std::shared_ptr<Range> RangePtr;




}
#endif