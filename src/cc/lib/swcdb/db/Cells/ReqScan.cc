/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace DB { namespace Cells {


ReqScan::ReqScan() : ResponseCallback(nullptr, nullptr), 
                     only_keys(false), offset(0) {
}

ReqScan::ReqScan(const DB::Specs::Interval& spec)
                : ResponseCallback(nullptr, nullptr), 
                  spec(spec),
                  only_keys(spec.flags.is_only_keys()),
                  offset(spec.flags.offset) {
}

ReqScan::ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
                 const DB::Specs::Interval& spec)
                : ResponseCallback(conn, ev), 
                  spec(spec),
                  only_keys(spec.flags.is_only_keys()), 
                  offset(spec.flags.offset) {
}

ReqScan::~ReqScan() { }

ReqScan::Ptr ReqScan::get_req_scan() {
  return std::dynamic_pointer_cast<ReqScan>(shared_from_this());
}

bool ReqScan::offset_adjusted() {
  if(offset) {
    --offset;
    return true;
  }
  return false;
}

bool ReqScan::selector(const KeyComp* key_comp,  
                       const DB::Cells::Cell& cell, bool& stop) {
  return spec.is_matching(key_comp, cell);
}


std::string ReqScan::to_string() const {
  std::string s(spec.to_string());
  s.append(" state(offset=");
  s.append(std::to_string(offset));
  s.append(")");
  return s;
}


}}}
