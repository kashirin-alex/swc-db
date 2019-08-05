/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_MNGR_Column_h
#define swcdb_lib_db_Columns_MNGR_Column_h

#include "swcdb/lib/db/Columns/Schema.h"
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
    std::string col_range_path(Range::get_column_path(id));
    int err = Error::OK;
    return EnvFsInterface::fs()->exists(err, col_range_path) && err == Error::OK;
  }

  Column(int64_t id) : cid(id), m_ranges(std::make_shared<RangesMap>()) {

    int err = Error::OK;
    std::string chk_file(Range::get_column_path(id));
    chk_file.append("/");
    chk_file.append(deleted_file);
    if(EnvFsInterface::fs()->exists(err, chk_file)){
      m_state = State::DELETED;
    } else {
      m_state = State::OK;
    }
  }

  bool create() {
    std::string col_range_path(Range::get_path(cid));
    int err = Error::OK;
    if(!EnvFsInterface::fs()->exists(err, col_range_path)) {
      EnvFsInterface::fs()->mkdirs(err, col_range_path);
      if(err == 17)
        err = Error::OK;
    }
    return err == Error::OK;
  }

  virtual ~Column(){}

  bool deleted(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::DELETED;
  }

  void ranges_by_fs(int &err, FS::IdEntries_t &entries){
    EnvFsInterface::interface()->get_structured_ids(err, Range::get_path(cid), entries);
  }

  RangePtr get_range(int64_t rid, bool initialize=false){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_ranges->find(rid);
    if (it != m_ranges->end())
      return it->second;

    if(initialize) {
      RangePtr range = std::make_shared<Range>(cid, rid, m_schema);
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
  SchemaPtr                  m_schema;
  std::shared_ptr<RangesMap> m_ranges;
  State                      m_state;

};
typedef std::shared_ptr<Column> ColumnPtr;

}}}

#endif