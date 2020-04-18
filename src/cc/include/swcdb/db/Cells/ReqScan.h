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

  ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
          const DB::Specs::Interval& spec);

  virtual ~ReqScan();

  Ptr get_req_scan();

  bool offset_adjusted();

  virtual bool selector(const Cell& cell, bool& stop);

  virtual bool reached_limits() = 0;
  
  virtual bool add_cell_and_more(const Cell& cell) = 0;
  
  virtual bool add_cell_set_last_and_more(const Cell& cell) = 0;
                 
  virtual bool matching_last(const DB::Cell::Key& key) = 0;

  virtual std::string to_string() const;

  DB::Specs::Interval   spec;
  bool                  only_keys;

  uint64_t              offset;

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

  Result                   cells;
  std::function<void(int)> cb;
};

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/ReqScan.cc"
#endif 

#endif // swcdb_db_Cells_ReqScan_h