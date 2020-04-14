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
    BLK_PRELOAD,
    COMPACTION
  };

  typedef std::shared_ptr<ReqScan>  Ptr;

  ReqScan(Type type=Type::QUERY)
          : type(type), 
            release_block(false), readahead(1), 
            block(nullptr) {
  }

  ReqScan(const DB::Cells::ReqScan::Config& cfg,
          Type type=Type::QUERY, bool release_block=false, uint8_t readahead=1)
          : DB::Cells::ReqScan(cfg),
            type(type), 
            release_block(release_block), readahead(readahead), 
            block(nullptr) {
  }

  ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
          const DB::Specs::Interval& spec)
          : DB::Cells::ReqScan(conn, ev, spec, DB::Cells::ReqScan::Config()),
            type(Type::QUERY), 
            release_block(false), readahead(0), 
            block(nullptr) {
  }

  ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
          const DB::Specs::Interval& spec,
          const DB::Cells::ReqScan::Config& cfg)
          : DB::Cells::ReqScan(conn, ev, spec, cfg),
            type(Type::QUERY), 
            release_block(false), 
            readahead(!spec.flags.limit ? 3 : spec.flags.limit > 1),
            block(nullptr) {
  }

  virtual ~ReqScan() { }

  Ptr get_req_scan() {
    return std::dynamic_pointer_cast<ReqScan>(shared_from_this());
  }

  bool expired() const {
    return (m_ev != nullptr && m_ev->expired()) || 
           (m_conn != nullptr && !m_conn->is_open()) ;
  }

  virtual bool with_block() {
    return false;
  }

  Type    type;
  bool    release_block;
  uint8_t readahead;
  void*   block;
};



class ReqScanBlockLoader : public ReqScan {
  public:
  
  ReqScanBlockLoader() : ReqScan(ReqScan::Type::BLK_PRELOAD) {
  }

  virtual ~ReqScanBlockLoader() { }

  bool reached_limits() override {
    return true;
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    return !reached_limits();
  }

  bool add_cell_set_last_and_more(const DB::Cells::Cell& cell) override {
    return !reached_limits();
  }

  bool matching_last(const DB::Cell::Key& key) override {
    return false;
  }
};



class ReqScanTest : public ReqScan {
  public:
  
  typedef std::shared_ptr<ReqScanTest>  Ptr;

  static Ptr make() { return std::make_shared<ReqScanTest>(); }

  ReqScanTest(const DB::Cells::ReqScan::Config& cfg
                     = DB::Cells::ReqScan::Config())
              : ReqScan(cfg) {
  }

  bool reached_limits() override {
    return (spec.flags.limit && spec.flags.limit <= cells.size()) 
            || 
            (cfg.buffer && cfg.buffer <= cells.size_bytes());
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    cells.add(cell, only_keys);
    return !reached_limits();
  }

  bool add_cell_set_last_and_more(const DB::Cells::Cell& cell) override {
    cells.add(cell, only_keys);
    return !reached_limits();
  }

  bool matching_last(const DB::Cell::Key& key) override {
    return !cells.empty() ? cells.back()->key.equal(key) : false;
  }

  virtual ~ReqScanTest() { }

  void response(int &err) override {
    cb(err);
  }

  DB::Cells::Result        cells;
  std::function<void(int)> cb;
};

}}

#endif // swcdb_ranger_db_ReqScan_h