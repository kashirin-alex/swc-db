/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace DB { namespace Cells {


ReqScan::ReqScan()  : ResponseCallback(nullptr, nullptr), 
                      offset(0), limit_buffer(0) {
}

ReqScan::ReqScan(const DB::Specs::Interval& spec, DB::Cells::Result& cells, 
                 uint32_t limit_buffer)
                : ResponseCallback(nullptr, nullptr), 
                  spec(spec), cells(cells),
                  offset(spec.flags.offset), limit_buffer(limit_buffer) {
}

ReqScan::ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
                 const DB::Specs::Interval& spec, DB::Cells::Result& cells, 
                 uint32_t limit_buffer)
                : ResponseCallback(conn, ev), 
                  spec(spec), cells(cells),
                  offset(spec.flags.offset), limit_buffer(limit_buffer) {
}

ReqScan::~ReqScan() { }

ReqScan::Ptr ReqScan::get_req_scan() {
  return std::dynamic_pointer_cast<ReqScan>(shared_from_this());
}

bool ReqScan::selector(const DB::Cells::Cell& cell, bool& stop) {
  return spec.is_matching(cell, cells.type);
}

bool ReqScan::reached_limits() {
  return (spec.flags.limit && spec.flags.limit <= cells.size()) 
         || 
         (limit_buffer && limit_buffer <= cells.size_bytes());
}

std::string ReqScan::to_string() const {
  std::string s("ReqScan(");
  s.append(spec.to_string());
  s.append(" limit_buffer=");
  s.append(std::to_string(limit_buffer));

  s.append(" ");
  s.append(cells.to_string());
  s.append(" state-offset=");
  s.append(std::to_string(offset));
  return s;
}


}}}
