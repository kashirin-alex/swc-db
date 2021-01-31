/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace DB { namespace Cells {


ReqScan::ReqScan() noexcept
                : Comm::ResponseCallback(nullptr, nullptr),
                  only_keys(false), offset(0) {
}

ReqScan::ReqScan(const DB::Specs::Interval& spec)
                : Comm::ResponseCallback(nullptr, nullptr),
                  spec(spec),
                  only_keys(spec.flags.is_only_keys()),
                  offset(spec.flags.offset) {
}

ReqScan::ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
                 const DB::Specs::Interval& spec)
                : Comm::ResponseCallback(conn, ev),
                  spec(spec),
                  only_keys(spec.flags.is_only_keys()),
                  offset(spec.flags.offset) {
}

ReqScan::ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
                 const DB::Cell::Key& range_begin,
                 const DB::Cell::Key& range_end)
                : Comm::ResponseCallback(conn, ev),
                  spec(range_begin, range_end),
                  only_keys(false), offset(0) {
}

ReqScan::~ReqScan() { }

ReqScan::Ptr ReqScan::get_req_scan() noexcept {
  return std::dynamic_pointer_cast<ReqScan>(shared_from_this());
}

bool ReqScan::offset_adjusted() noexcept {
  if(offset) {
    --offset;
    return true;
  }
  return false;
}

bool ReqScan::selector(const Types::KeySeq key_seq,
                       const DB::Cells::Cell& cell, bool& stop) {
  return spec.is_matching(key_seq, cell, stop);
}

void ReqScan::print(std::ostream& out) const {
  spec.print(out);
  out << " state(offset=" << offset << ')';
}


}}}
