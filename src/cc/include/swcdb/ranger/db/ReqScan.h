/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_ranger_db_ReqScan_h
#define swcdb_ranger_db_ReqScan_h


#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace Ranger {
  
class ReqScan  : public DB::Cells::ReqScan {
  public:
  
  enum Type {
    QUERY,
    BLK_PRELOAD
  };

  typedef std::shared_ptr<ReqScan>  Ptr;

  ReqScan(Type type=Type::QUERY)
          : type(type), drop_caches(false) {
  }

  ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
          const DB::Specs::Interval& spec, DB::Cells::Result& cells, 
          uint32_t limit_buffer=0)
          : DB::Cells::ReqScan(conn, ev, spec, cells, limit_buffer),
            type(Type::QUERY), drop_caches(false) {
  }

  virtual ~ReqScan() { }

  Ptr get_req_scan() {
    return std::dynamic_pointer_cast<ReqScan>(shared_from_this());
  }

  bool expired() const {
    return (m_ev != nullptr && m_ev->expired()) || 
           (m_conn != nullptr && !m_conn->is_open()) ;
  }

  Type              type;
  bool              drop_caches;
};



class ReqScanTest : public ReqScan {
  public:
  
  typedef std::shared_ptr<ReqScanTest>  Ptr;

  static Ptr make() { return std::make_shared<ReqScanTest>(); }

  ReqScanTest() {}
  virtual ~ReqScanTest() { }

  void response(int &err) override {
    cb(err);
  }

  std::function<void(int)> cb;
};

}}

#endif // swcdb_ranger_db_ReqScan_h