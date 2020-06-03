/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_Cells_ReqScan_h
#define swcdb_db_Cells_ReqScan_h


#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Cells/Result.h"


namespace SWC { namespace DB { namespace Cells {
  
class ReqScan : public ResponseCallback {
  public:

  typedef std::shared_ptr<ReqScan>  Ptr;

  ReqScan();

  ReqScan(const DB::Specs::Interval& spec);

  ReqScan(const ConnHandlerPtr& conn, const Event::Ptr& ev, 
          const DB::Specs::Interval& spec);

  ReqScan(const ReqScan&) = delete;

  ReqScan(const ReqScan&&) = delete;

  ReqScan& operator=(const ReqScan&) = delete;

  virtual ~ReqScan();

  Ptr get_req_scan();

  bool offset_adjusted();

  virtual bool selector(const Types::KeySeq key_seq, const Cell& cell, 
                        bool& stop);

  virtual bool reached_limits() = 0;
  
  virtual bool add_cell_and_more(const Cell& cell) = 0;

  virtual std::string to_string() const;

  DB::Specs::Interval   spec;
  bool                  only_keys;

  uint64_t              offset;

  struct Profile {
    const int64_t ts_start    = Time::now_ns();
    int64_t ts_finish;
    uint64_t cells_count      = 0;
    uint64_t cells_bytes      = 0;
    uint64_t cells_skipped    = 0;
    uint64_t blocks_scanned   = 0;
    uint64_t block_time_scan  = 0;
    uint64_t blocks_loaded    = 0;
    uint64_t block_time_load  = 0;

    void finished() {
      ts_finish = Time::now_ns();
    }

    void add_cell(uint32_t bytes) {
      ++cells_count;
      cells_bytes += bytes;
    }

    void skip_cell() {
      ++cells_skipped;
    }
    
    void add_block_scan(int64_t ts) {
      ++blocks_scanned;
      block_time_scan += Time::now_ns() - ts;
    }

    void add_block_load(int64_t ts) {
      ++blocks_loaded;
      block_time_load += Time::now_ns() - ts;
    }

    std::string to_string() const {
      std::string s("profile(took=");
      s.append(std::to_string(ts_finish - ts_start));
      s.append("ns cells(count=");
      s.append(std::to_string(cells_count));
      s.append(" bytes=");
      s.append(std::to_string(cells_bytes));
      s.append(" skip=");
      s.append(std::to_string(cells_skipped));
      s.append(") blocks(loaded=");
      s.append(std::to_string(blocks_loaded));
      s.append(" scanned=");
      s.append(std::to_string(blocks_scanned));
      s.append(") block(load=");
      s.append(std::to_string(block_time_load));
      s.append("ns scan=");
      s.append(std::to_string(block_time_scan));
      s.append("ns))");
      return s;
    }
  };
  Profile profile;
};



class ReqScanTest : public ReqScan {
  public:
  
  typedef std::shared_ptr<ReqScanTest>  Ptr;

  static Ptr make() { return std::make_shared<ReqScanTest>(); }

  ReqScanTest() { }

  bool reached_limits() override {
    return (spec.flags.limit && spec.flags.limit <= cells.size()) || 
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

  Result                   cells;
  std::function<void(int)> cb;
};

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/ReqScan.cc"
#endif 

#endif // swcdb_db_Cells_ReqScan_h