/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RS_Column_h
#define swcdb_lib_db_Columns_RS_Column_h

#include "Range.h"

#include <mutex>
#include <memory>
#include <unordered_map>

namespace SWC { namespace server { namespace RS {

typedef std::unordered_map<int64_t, RangePtr> RangesMap;
typedef std::pair<int64_t, RangePtr> RangesMapPair;


class Column : public std::enable_shared_from_this<Column> {
  
  public:

  Column(int64_t id) 
        : cid(id), m_ranges(std::make_shared<RangesMap>()), 
          m_deleting(false) { }

  void init(int &err) { }

  virtual ~Column(){}

  RangePtr get_range(int &err, int64_t rid, bool initialize=false){
    RangePtr range = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      auto it = m_ranges->find(rid);
      if (it != m_ranges->end())
        return it->second;
      else if(initialize) {
        if(Env::RsData::is_shuttingdown()){
          err = Error::SERVER_SHUTTING_DOWN;
          return range;
        }
        if(m_deleting){
          err = Error::COLUMN_MARKED_REMOVED;
          return range;
        }
        range = std::make_shared<Range>(cid, rid);
        m_ranges->insert(RangesMapPair(rid, range));;
      }
    }
    return range;
  }

  void unload(int64_t rid, Callback::RangeUnloaded_t cb){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_ranges->find(rid);
    if (it != m_ranges->end()){
      it->second->unload(cb, true);
      m_ranges->erase(it);
    }
  }

  void unload_all(std::atomic<int>& unloaded, Callback::RangeUnloaded_t cb){

    for(;;){
      std::lock_guard<std::mutex> lock(m_mutex);
      auto it = m_ranges->begin();
      if(it == m_ranges->end())
        break;
      unloaded++;
      asio::post(
        *Env::IoCtx::io()->ptr(), 
        [cb, range=it->second](){range->unload(cb, false);}
      );
      m_ranges->erase(it);
    }

    cb(Error::OK);
  }
  
  void remove_all(int &err){
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_deleting = true;
    }
      
    for(;;){
      std::lock_guard<std::mutex> lock(m_mutex);
      auto it = m_ranges->begin();
      if(it == m_ranges->end())
        break;
      it->second->remove(err);
      m_ranges->erase(it);
    }

    HT_DEBUGF("REMOVED %s", to_string().c_str());
  }

  bool removing() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return  m_deleting;
  }

  std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("[cid=");
    s.append(std::to_string(cid));

    if(m_deleting){
      s.append(", DELETING");
    }

    s.append(", ranges=(");
    
    for(auto it = m_ranges->begin(); it != m_ranges->end(); ++it){
      s.append(it->second->to_string());
      s.append(",");
    }
    s.append(")]");
    return s;
  }

  private:

  std::mutex                 m_mutex;
  int64_t                    cid;
  std::shared_ptr<RangesMap> m_ranges;
  bool                       m_deleting;
};
typedef std::shared_ptr<Column> ColumnPtr;

}}}

#endif