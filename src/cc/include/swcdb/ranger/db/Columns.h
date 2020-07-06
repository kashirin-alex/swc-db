/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_Columns_h
#define swcdb_ranger_db_Columns_h

#include "swcdb/db/Columns/Schema.h"
#include "swcdb/core/comm/ResponseCallback.h"

#include "swcdb/ranger/db/ColumnCfg.h"
#include "swcdb/ranger/db/Callbacks.h"
#include "swcdb/ranger/db/Column.h"
#include "swcdb/ranger/db/ColumnsReqDelete.h"

#include <unordered_map>

namespace SWC { namespace Ranger {


class Columns final : private std::unordered_map<cid_t, Column::Ptr> {

  public:

  enum State {
    NONE,
    OK
  };

  typedef Columns* Ptr;

  explicit Columns()  { } //: m_state(State::OK)

  ~Columns() { }

  
  Column::Ptr initialize(int &err, const cid_t cid, 
                         const DB::Schema& schema) {
    Column::Ptr col = nullptr;
    if(RangerEnv::is_shuttingdown()) {
      err = Error::SERVER_SHUTTING_DOWN;
      return col;
    }
    
    Mutex::scope lock(m_mutex);
    auto it = find(cid);
    if(it != end())
      (col = it->second)->cfg.update(schema);
    else
      emplace(cid, col = std::make_shared<Column>(cid, schema));
    return col;
  }

  Column::Ptr get_column(int &err, const cid_t cid) {
    Mutex::scope lock(m_mutex);
    auto it = find(cid);
    return it == end() ? nullptr : it->second;
  }
  
  Column::Ptr get_next(size_t& idx) {
    Mutex::scope lock(m_mutex);
    if(size() > idx) {
      auto it = begin();
      for(int i=idx; i; --i, ++it);
      return it->second;
    }
    idx = 0;
    return nullptr;
  }

  RangePtr get_range(int &err, const cid_t cid, const rid_t rid) {
    Column::Ptr col = get_column(err, cid);
    if(!col) 
      return nullptr;
    if(col->removing()) 
      err = Error::COLUMN_MARKED_REMOVED;
    return err ? nullptr : col->get_range(err, rid, false);
  }
 
  void load_range(int &err, const cid_t cid, const rid_t rid, 
                  const DB::Schema& schema, 
                  const ResponseCallback::Ptr& cb) {
    RangePtr range;
    auto col = initialize(err, cid, schema);
    if(!err) {

      if(col->removing()) 
        err = Error::COLUMN_MARKED_REMOVED;

      else if(RangerEnv::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      
      if(!err)
        range = col->get_range(err, rid, true);
    }
    if(err)
      cb->response(err);
    else 
      range->load(cb);
  }

  void unload_range(int &err, const cid_t cid, const rid_t rid,
                    const Callback::RangeUnloaded_t& cb) {
    Column::Ptr col = get_column(err, cid);
    if(col) {
      col->unload(rid, cb);
    } else {
      cb(err);
    }
  }

  void unload_all(bool validation) {
    std::vector<std::function<bool(cid_t)>> order = {
      Types::MetaColumn::is_data,
      Types::MetaColumn::is_meta,
      Types::MetaColumn::is_master 
    };

    std::atomic<int> to_unload = 0;
    Column::Ptr col;
    uint8_t meta;
    iterator it;
    for(auto& chk : order) {
      {
        Mutex::scope lock(m_mutex);
        for(it = begin(); it != end(); ++it) {
          if(chk(it->first))
            ++to_unload;
        }
      }
      if(!to_unload)
        continue;

      std::promise<void>  r_promise;
      Callback::RangeUnloaded_t cb 
        = [&to_unload, &r_promise](int err){
          if(--to_unload == 0)
            r_promise.set_value();
      };
    
      for(meta=0;;) {
        {
          Mutex::scope lock(m_mutex);
          it = begin();
          for(uint8_t n=0; n<meta; ++n, ++it);
          if(it == end())
            break;
          for(;!chk(it->first) && size() > ++meta; ++it);
          if(size() == meta)
            break;
          col = it->second;
          erase(it);
        }
        if(validation)
          SWC_LOGF(LOG_WARN, 
            "Unload-Validation cid=%lu remained", col->cfg.cid);
        col->unload_all(to_unload, cb);
      }
      r_promise.get_future().wait();
    }
  }

  void remove(ColumnsReqDelete* req) {
    if(!m_q_remove.push_and_is_1st(req))
      return;

    int err;
    Column::Ptr col;
    iterator it;
    do {
      if(!(req = m_q_remove.front())->ev->expired()) {
        if((col = get_column(err = Error::OK, req->cid))) {
          col->remove_all(err);
          {
            Mutex::scope lock(m_mutex);
            if((it = find(req->cid)) != end()) 
              erase(it);
          }
        }
        req->cb(err);
      }
      delete req;
    } while(m_q_remove.pop_and_more());
  }
  
  size_t release(size_t bytes=0) {
    size_t released = 0;
    Column::Ptr col;
    iterator it;
    for(size_t offset = 0; ; ++offset) {
      {
        Mutex::scope lock(m_mutex);
        it = begin();
        for(size_t i=0; i<offset && it != end(); ++it, ++i);
        if(it == end())
          break;
        if(!Types::MetaColumn::is_data(it->first))
          continue;
        col = it->second;
      }
      released += col->release(bytes ? bytes-released : bytes);
      if(bytes && released >= bytes)
        break;
    }
    return released;
  }

  std::string to_string() {
    std::string s("columns=(");
    Mutex::scope lock(m_mutex);

    for(auto it = begin(); it != end(); ++it){
      s.append(it->second->to_string());
      s.append(",");
    }
    s.append(")");
    return s;
  }

  private:
  Mutex                           m_mutex;
  // State                           m_state;
  QueueSafe<ColumnsReqDelete*>    m_q_remove;

};

}} // namespace SWC::Ranger

#endif