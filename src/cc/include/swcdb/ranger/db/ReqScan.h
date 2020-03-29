/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_ranger_db_ReqScan_h
#define swcdb_ranger_db_ReqScan_h


#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Cells/Result.h"

namespace SWC { namespace Ranger {
  
class ReqScan  : public ResponseCallback {

  public:
  enum Type {
    QUERY,
    BLK_PRELOAD
  };
  typedef std::shared_ptr<ReqScan>  Ptr;
  typedef std::function<void()>     NextCall_t;

  ReqScan(Type type=Type::QUERY) 
          : ResponseCallback(nullptr, nullptr), 
            limit_buffer_sz(0), offset(0), 
            drop_caches(false), type(type) {
  }

  ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
          const DB::Specs::Interval& spec, DB::Cells::Result& cells)
          : ResponseCallback(conn, ev), spec(spec), 
            cells(cells),
            offset(spec.flags.offset), limit_buffer_sz(0), 
            drop_caches(false), type(Type::QUERY) {
  }

  virtual ~ReqScan() { }

  Ptr get_req_scan() {
    return std::dynamic_pointer_cast<ReqScan>(shared_from_this());
  }

  virtual const DB::Cells::Mutable::Selector_t selector() {
    return [req=get_req_scan()] 
            (const DB::Cells::Cell& cell, bool& stop) 
            { return req->selector(cell, stop); };
  }

  const bool selector(const DB::Cells::Cell& cell, bool& stop) const {
    return spec.is_matching(cell, cells.type);
  }
  
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
    return (spec.flags.limit && spec.flags.limit <= cells.size()) 
           || 
           (limit_buffer_sz && limit_buffer_sz <= cells.size_bytes());
  }
  
  const bool expired() const {
    return (m_ev != nullptr && m_ev->expired()) || 
           (m_conn != nullptr && !m_conn->is_open()) ;
  }

  const std::string to_string() const {
    std::string s("ReqScan(");
    s.append(spec.to_string());
    s.append(" ");
    s.append(cells.to_string());
    s.append(" limit_buffer_sz=");
    s.append(std::to_string(limit_buffer_sz));
    s.append(" state-offset=");
    s.append(std::to_string(offset));
    return s;
  }

  DB::Specs::Interval   spec;
  DB::Cells::Result     cells;

  uint32_t          limit_buffer_sz;
  bool              drop_caches;

  NextCall_t        next_call = 0;
  
  // state of a scan
  size_t            offset;
  Type              type;
};

class ReqScanTest : public ReqScan {
  public:
  
  typedef std::shared_ptr<ReqScanTest>  Ptr;

  static Ptr make() { return std::make_shared<ReqScanTest>(); }

  ReqScanTest() {}
  virtual ~ReqScanTest() { }

  void response(int &err) override {
    if(!ReqScan::ready(err))
      return;
    cb(err);
  }

  std::function<void(int)> cb;
};

}}

#endif // swcdb_ranger_db_ReqScan_h