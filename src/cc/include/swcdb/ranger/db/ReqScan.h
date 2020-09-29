/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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

  ReqScan(Type type=Type::QUERY, bool release_block=false, uint8_t readahead=1)
          : type(type), 
            release_block(release_block), readahead(readahead), 
            block(nullptr) {
  }
        
  ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev, 
          const DB::Cell::Key& range_begin, const DB::Cell::Key& range_end)
          : DB::Cells::ReqScan(conn, ev, range_begin, range_end),
            type(Type::QUERY), 
            release_block(false), readahead(0), 
            block(nullptr) {
  }

  ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev, 
          const DB::Specs::Interval& spec)
          : DB::Cells::ReqScan(conn, ev, spec),
            type(Type::QUERY), 
            release_block(false), 
            readahead((!spec.flags.limit || spec.flags.offset)//?>block_cells
                        ? 2 : spec.flags.limit > 1),
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

  bool add_cell_and_more(const DB::Cells::Cell&) override {
    return !reached_limits();
  }
};



class ReqScanTest : public ReqScan {
  public:
  
  typedef std::shared_ptr<ReqScanTest>  Ptr;

  static Ptr make() { return std::make_shared<ReqScanTest>(); }

  ReqScanTest() { }

  bool reached_limits() override {
    return (spec.flags.limit && spec.flags.limit <= cells.size())   || 
      (spec.flags.max_buffer && spec.flags.max_buffer <= cells.size_bytes());
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    cells.add(cell, only_keys);
    return !reached_limits();
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