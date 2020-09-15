/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_ranger_callbacks_RangeLocateScanCommit_h
#define swc_ranger_callbacks_RangeLocateScanCommit_h

#include "swcdb/ranger/callbacks/RangeLocateScan.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeLocateScanCommit : public RangeLocateScan {
  public:

  typedef std::shared_ptr<RangeLocateScanCommit> Ptr;

  RangeLocateScanCommit(const ConnHandlerPtr& conn, const Event::Ptr& ev, 
                        const DB::Cell::Key& range_begin,
                        const RangePtr& range, uint8_t flags)
                        : RangeLocateScan(
                            conn, ev, 
                            range_begin, DB::Cell::Key(), range, flags) {
  }

  virtual ~RangeLocateScanCommit() { }

  bool selector(const Types::KeySeq key_seq, 
                const DB::Cells::Cell& cell, bool& stop) override {
    if(any_is && 
      DB::KeySeq::compare(key_seq, 
        range_begin, cell.key, any_is) != Condition::EQ)
      return false;

    size_t remain = cell.vlen;
    const uint8_t * ptr = cell.value;
    DB::Cell::Key key_end;
    key_end.decode(&ptr, &remain, false);
    bool match;
    if((match = key_end.count == any_is || 
                DB::KeySeq::compare(key_seq,
                  key_end, range_begin) != Condition::GT))
      stop = true;
    return match;
  }


};


}}}
#endif // swc_ranger_callbacks_RangeLocateScanCommit_h
