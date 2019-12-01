/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_ReqScan_h
#define swcdb_db_Cells_ReqScan_h


#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Cells/Mutable.h"

namespace SWC { namespace DB { namespace Cells {
  
class ReqScan  : public ResponseCallback {

  public:
  enum Type {
    QUERY,
    BLK_PRELOAD
  };
  typedef std::shared_ptr<ReqScan>  Ptr;
  typedef std::function<void()>     NextCall_t;

  ReqScan(Type type=Type::QUERY) : ResponseCallback(nullptr, nullptr), 
              limit_buffer_sz(0), offset(0), 
              has_selector(false), drop_caches(false), 
              type(type) {          
  }

  ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
          Specs::Interval::Ptr spec, Mutable::Ptr cells)
          : ResponseCallback(conn, ev), spec(spec), cells(cells),
            offset(spec->flags.offset), limit_buffer_sz(0), 
            has_selector(false), drop_caches(false), type(Type::QUERY) {
  }

  inline Ptr get_req_scan() {
    return std::dynamic_pointer_cast<ReqScan>(shared_from_this());
  }

  virtual bool selector(const DB::Cells::Cell& cell) { return true; }
  
  bool ready(int& err) {
    auto call = next_call;
    next_call = 0;
    if(!err && call && !reached_limits()){
      call();
      return false;
    }
    return true;
  }

  virtual bool reached_limits() {
    return (spec->flags.limit > 0 && spec->flags.limit == cells->size) 
           || 
           (limit_buffer_sz > 0 && limit_buffer_sz <= cells->size_bytes);
  }
 
  virtual ~ReqScan() { }

  const std::string to_string() const {
    std::string s("ReqScan(");
    s.append(spec->to_string());
    s.append(" has_selector=");
    s.append(has_selector?"true":"false");
    s.append(" ");
    s.append(cells->to_string());
    s.append(" limit_buffer_sz=");
    s.append(std::to_string(limit_buffer_sz));
    s.append(" state-offset=");
    s.append(std::to_string(offset));
    return s;
  }

  Specs::Interval::Ptr    spec;
  Mutable::Ptr            cells;

  uint32_t                limit_buffer_sz;
  bool                    has_selector;
  bool                    drop_caches;

  NextCall_t              next_call = 0;
  
  // state of a scan
  size_t                  offset;
  Type                    type;
};

class ReqScanTest : public ReqScan {
  public:
  
  typedef std::shared_ptr<ReqScanTest>  Ptr;

  static Ptr make() { return std::make_shared<ReqScanTest>(); }

  ReqScanTest() {}
  virtual ~ReqScanTest() { }

  void response(int &err) override {
    if(!DB::Cells::ReqScan::ready(err))
      return;
    cb(err);
  }

  std::function<void(int)> cb;
};

}}}

#endif