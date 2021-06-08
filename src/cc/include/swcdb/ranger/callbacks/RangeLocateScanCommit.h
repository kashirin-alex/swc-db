/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeLocateScanCommit_h
#define swcdb_ranger_callbacks_RangeLocateScanCommit_h

#include "swcdb/ranger/callbacks/RangeLocateScan.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeLocateScanCommit : public RangeLocateScan {
  public:

  typedef std::shared_ptr<RangeLocateScanCommit> Ptr;

  SWC_CAN_INLINE
  RangeLocateScanCommit(const Comm::ConnHandlerPtr& conn,
                        const Comm::Event::Ptr& ev,
                        const DB::Cell::Key& range_begin,
                        const RangePtr& range,
                        uint8_t flags)
                        : RangeLocateScan(
                            conn, ev,
                            range_begin, DB::Cell::Key(), range, flags) {
  }

  virtual ~RangeLocateScanCommit() { }

  bool selector(const DB::Types::KeySeq key_seq,
                const DB::Cells::Cell& cell, bool& stop) override {
    if(any_is &&
       DB::KeySeq::compare_upto(key_seq, range_begin, cell.key, any_is)
        != Condition::EQ)
      return false;

    StaticBuffer v;
    cell.get_value(v);
    const uint8_t* ptr = v.base;
    size_t remain = v.size;

    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    DB::Cell::Key key_end;
    key_end.decode(&ptr, &remain, false);
    bool match = key_end.count == any_is ||
                 DB::KeySeq::compare(key_seq, key_end, range_begin)
                  != Condition::GT;
    if(match) {
      stop = true;
      uint8_t after_idx = range->cfg->range_type == DB::Types::Range::MASTER;
      params.range_begin.copy(after_idx, cell.key);
      params.range_end.copy(after_idx, key_end);
      DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
      params.rid = Serialization::decode_vi64(&ptr, &remain);
    }
    return match;
  }


};


}}}
#endif // swcdb_ranger_callbacks_RangeLocateScanCommit_h
