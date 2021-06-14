/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace DB { namespace Cells {


bool ReqScan::selector(const Types::KeySeq key_seq,
                       const DB::Cells::Cell& cell, bool& stop) {
  return spec.is_matching(key_seq, cell, stop);
}

void ReqScan::print(std::ostream& out) const {
  spec.print(out);
  out << " state(offset=" << offset << ')';
}


}}}
