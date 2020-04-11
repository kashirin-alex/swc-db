/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace DB { namespace Cells {

ReqScan::Config::Config(
  Types::Column col_type, 
  uint32_t cell_versions, 
  uint64_t cell_ttl, 
  uint32_t buffer
  ) : col_type(col_type), 
      cell_versions(cell_versions), 
      cell_ttl(cell_ttl), 
      buffer(buffer) {
}

std::string ReqScan::Config::to_string() const {
  std::string s("cfg(");
  s.append("type=");
  s.append(Types::to_string(col_type));
  s.append(" versions=");
  s.append(std::to_string(cell_versions));
  s.append(" ttl=");
  s.append(std::to_string(cell_ttl));
  s.append(" buffer=");
  s.append(std::to_string(buffer));
  s.append(")");
  return s;
}


ReqScan::ReqScan() : ResponseCallback(nullptr, nullptr), 
                     offset(0) {
}

ReqScan::ReqScan(const ReqScan::Config& cfg) 
                : ResponseCallback(nullptr, nullptr),
                  cfg(cfg), offset(0) { 
}

ReqScan::ReqScan(const DB::Specs::Interval& spec, const ReqScan::Config& cfg)
                : ResponseCallback(nullptr, nullptr), 
                  spec(spec), cfg(cfg), 
                  only_keys(spec.flags.is_only_keys()),
                  offset(spec.flags.offset) {
}

ReqScan::ReqScan(ConnHandlerPtr conn, Event::Ptr ev, 
                 const DB::Specs::Interval& spec, const ReqScan::Config& cfg)
                : ResponseCallback(conn, ev), 
                  spec(spec), cfg(cfg), 
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

bool ReqScan::selector(const DB::Cells::Cell& cell, bool& stop) {
  return spec.is_matching(cell, cfg.col_type);
}


std::string ReqScan::to_string() const {
  std::string s(spec.to_string());
  s.append(" ");
  s.append(cfg.to_string());
  s.append(" state(offset=");
  s.append(std::to_string(offset));
  s.append(")");
  return s;
}


}}}
