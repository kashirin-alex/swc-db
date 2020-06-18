/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_ranger_callbacks_RangeLocateScanCommit_h
#define swc_ranger_callbacks_RangeLocateScanCommit_h

#include "swcdb/ranger/callbacks/RangeLocateScan.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeLocateScanCommit : public RangeLocateScan {
  public:

  typedef std::shared_ptr<RangeLocateScanCommit> Ptr;

  RangeLocateScanCommit(ConnHandlerPtr conn, Event::Ptr ev, 
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
    key_end.decode(&ptr, &remain);
    bool match;
    if(match = key_end.count == any_is || 
               DB::KeySeq::compare(key_seq, 
                key_end, range_begin) != Condition::GT)
      stop = true;
    return match;
  }


};


}}}
#endif // swc_ranger_callbacks_RangeLocateScanCommit_h
