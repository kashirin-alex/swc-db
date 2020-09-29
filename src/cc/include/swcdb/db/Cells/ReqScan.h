/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Cells_ReqScan_h
#define swcdb_db_Cells_ReqScan_h


#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Cells/Result.h"


namespace SWC { namespace DB { namespace Cells {
  
class ReqScan : public Comm::ResponseCallback {
  public:

  typedef std::shared_ptr<ReqScan>  Ptr;

  ReqScan();

  ReqScan(const DB::Specs::Interval& spec);

  ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev, 
          const DB::Specs::Interval& spec);

  ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev, 
          const DB::Cell::Key& range_begin, const DB::Cell::Key& range_end);
                  
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

  virtual void print(std::ostream& out) const;

  DB::Specs::Interval   spec;
  bool                  only_keys;

  uint64_t              offset;

  struct Profile {
    const int64_t ts_start    = Time::now_ns();
    int64_t ts_finish;
    uint64_t cells_count      = 0;
    uint64_t cells_bytes      = 0;
    uint64_t cells_skipped    = 0;

    uint64_t blocks_located   = 0;
    uint64_t block_time_locate= 0;

    uint64_t blocks_loaded    = 0;
    uint64_t block_time_load  = 0;

    uint64_t blocks_scanned   = 0;
    uint64_t block_time_scan  = 0;

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
    
    void add_block_locate(int64_t ts) {
      ++blocks_located;
      block_time_locate += Time::now_ns() - ts;
    }

    void add_block_load(int64_t ts) {
      ++blocks_loaded;
      block_time_load += Time::now_ns() - ts;
    }

    void add_block_scan(int64_t ts) {
      ++blocks_scanned;
      block_time_scan += Time::now_ns() - ts;
    }

    std::string to_string() const {
      std::stringstream ss;
      print(ss);
      return ss.str();
    }

    void print(std::ostream& out) const {
      out << "profile(took=" 
          << (ts_finish - ts_start)
          << "ns cells(count=" << cells_count
          << " bytes=" << cells_bytes
          << " skip=" << cells_skipped
          << ") blocks(located=" << blocks_located
          << " loaded=" << blocks_loaded
          << " scanned=" << blocks_scanned
          << ") block(locate=" << block_time_locate
          << "ns load=" << block_time_load
          << "ns scan=" << block_time_scan
          << "ns))";
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