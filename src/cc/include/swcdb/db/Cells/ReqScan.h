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

  ReqScan(const DB::Specs::Interval& spec, DB::Cells::Result& cells, 
          uint32_t limit_buffer=0);

  ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
          const DB::Specs::Interval& spec, DB::Cells::Result& cells, 
          uint32_t limit_buffer=0);

  virtual ~ReqScan();

  Ptr get_req_scan();

  virtual bool selector(const DB::Cells::Cell& cell, bool& stop);

  virtual bool reached_limits();

  std::string to_string() const;

  DB::Specs::Interval   spec;
  Result                cells;
  uint64_t              offset;
  uint32_t              limit_buffer;

};

}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/ReqScan.cc"
#endif 

#endif // swcdb_db_Cells_ReqScan_h