/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_MNGR_Column_h
#define swcdb_lib_db_Columns_MNGR_Column_h

#include "Range.h"

#include <mutex>
#include <memory>
#include <unordered_map>

namespace SWC { namespace server { namespace Mngr {

typedef std::unordered_map<int64_t, RangePtr> RangesMap;
typedef std::pair<int64_t, RangePtr> RangesMapPair;


class Column : public std::enable_shared_from_this<Column> {
  
  public:

  enum State {
    OK,
    DELETED
  };

  inline static const std::string deleted_file = "deleted.mark";

  static bool exists(int64_t id) {
    return Env::FsInterface::interface()->exists(Range::get_column_path(id));
  }
  
  static bool create(int64_t id) {
    Env::FsInterface::interface()->mkdirs(Range::get_column_path(id));
    return true;
  }

  static void mark_deleted(int &err, int64_t cid) {
    std::string filepath(Range::get_column_path(cid));
    filepath.append("/");
    filepath.append(deleted_file);

    FS::SmartFdPtr smartfd = 
      FS::SmartFd::make_ptr(filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE);

    StaticBuffer send_buf;
    for(;;) {
      err = Error::OK;
      Env::FsInterface::fs()->write(err, smartfd, -1, -1, send_buf);
      if (err == Error::OK)
        return;
      else if(err == Error::FS_FILE_NOT_FOUND 
              || err == Error::FS_PERMISSION_DENIED)
        return;

      HT_DEBUGF("mark_deleted, retrying to err=%d(%s)", 
                err, Error::get_text(err));
    }
  }

  static bool is_marked_deleted(int64_t cid) {
    std::string chk_file(Range::get_column_path(cid));
    chk_file.append("/");
    chk_file.append(deleted_file);
    return Env::FsInterface::interface()->exists(chk_file);
  }

  static void clear_marked_deleted(int64_t cid) {
    std::string chk_file(Range::get_column_path(cid));
    chk_file.append("/");
    chk_file.append(deleted_file);
    Env::FsInterface::interface()->remove(chk_file);
  }



  Column(int64_t id) 
        : cid(id), m_ranges(std::make_shared<RangesMap>()),
          m_state(State::OK) {
  }

  virtual ~Column(){}

  bool exists_range_path() {
    return Env::FsInterface::interface()->exists(Range::get_path(cid));
  }

  bool create() {
    Env::FsInterface::interface()->mkdirs(Range::get_path(cid));
    return true;
  }

  bool deleted(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::DELETED;
  }

  void ranges_by_fs(int &err, FS::IdEntries_t &entries){
    Env::FsInterface::interface()->get_structured_ids(err, Range::get_path(cid), entries);
  }

  RangePtr get_range(int64_t rid, bool initialize=false){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_ranges->find(rid);
    if (it != m_ranges->end())
      return it->second;

    if(initialize) {
      RangePtr range = std::make_shared<Range>(cid, rid);
      m_ranges->insert(RangesMapPair(rid, range));
      return range;
    }
    return nullptr;
  }
  
  RangePtr get_next_unassigned(){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      if(it->second->need_assign())
        return it->second;
    }
    return nullptr;
  }

  void set_rs_unassigned(uint64_t rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      if(it->second->get_rs_id() == rs_id){
        it->second->set_state(Range::State::NOTSET, 0);
      }
    }
  }

  void change_rs(uint64_t rs_id_old, uint64_t rs_id){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      if(it->second->get_rs_id() == rs_id_old)
        it->second->set_rs_id(rs_id);
    }
  }

  int64_t get_next_rid(){
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t rid = 0;
    for(;;){
      auto it = m_ranges->find(++rid);
      if(it == m_ranges->end())
        break;
      if(it->second->deleted())
        break;
    }
    return rid;
  }
  
  void assigned(std::vector<uint64_t> &rs_ids){
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t rs_id;
    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      rs_id = it->second->get_rs_id();
      if(rs_id == 0)
        continue;
      if(std::find_if(rs_ids.begin(), rs_ids.end(), [rs_id]
      (const uint64_t& rs_id2){return rs_id == rs_id2;}) == rs_ids.end())
        rs_ids.push_back(rs_id);
    }
  }

  bool do_remove(){
    bool was;
    std::lock_guard<std::mutex> lock(m_mutex);
    was = m_state == State::DELETED;
    if(!was){
      m_state = State::DELETED;
      for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it)
        it->second->set_state(Range::State::DELETED);
    }
    return !was;
  }

  bool finalize_remove(uint64_t rs_id=0){
    bool empty;
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      for(;;){
        auto it = m_ranges->begin();
        if(it == m_ranges->end())
          break;
        if(rs_id == 0 
          || it->second->get_rs_id() == rs_id
          || it->second->get_rs_id() == 0)
          m_ranges->erase(it);
      }
      empty = m_ranges->empty();
      if(empty)
        Env::FsInterface::interface()->rmdir(Range::get_path(cid));
    }

    if(empty)
      HT_DEBUGF("FINALIZED REMOVE %s", to_string().c_str());
    return empty;
  }

  std::string to_string(){
    std::string s("[cid=");
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      s.append(std::to_string(cid));
    }

    s.append(", next-rid=");
    s.append(std::to_string(get_next_rid()));
    s.append(", ranges=(");
    
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
        s.append(it->second->to_string());
        s.append(",");
      }
    }
    s.append(")]");
    return s;
  }

  private:

  std::mutex                 m_mutex;
  int64_t                    cid;
  std::shared_ptr<RangesMap> m_ranges;
  State                      m_state;

};
typedef std::shared_ptr<Column> ColumnPtr;

}}}

#endif