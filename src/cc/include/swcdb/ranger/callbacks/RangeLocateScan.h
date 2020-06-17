/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_ranger_callbacks_RangeLocateScan_h
#define swc_ranger_callbacks_RangeLocateScan_h

#include "swcdb/ranger/db/ReqScan.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeLocateScan : public ReqScan {
  public:

  typedef std::shared_ptr<RangeLocateScan> Ptr;

  RangeLocateScan(ConnHandlerPtr conn, Event::Ptr ev, 
                  const DB::Cell::Key& range_begin, 
                  const DB::Cell::Key& range_end, 
                  RangePtr range, uint8_t flags)
                  : ReqScan(conn, ev, range_begin, range_end), 
                    range(range), flags(flags),
                    any_is(range->type != Types::Range::DATA),
                    range_begin(range_begin, false) {
    auto c = range->known_interval_count();
    spec.range_begin.remove(c ? c : (uint24_t)1, true);
  }

  virtual ~RangeLocateScan() { }

  bool selector(const Types::KeySeq key_seq, 
                const DB::Cells::Cell& cell, bool& stop) override {
    //SWC_PRINT << "---------------------"
    //  << "  cell.key: " << cell.key.to_string() << SWC_PRINT_CLOSE;
    if(any_is && 
        DB::KeySeq::compare(key_seq, spec.range_begin, cell.key, any_is)
          != Condition::EQ)
      return false;

    if(flags & Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE && 
        DB::KeySeq::compare(key_seq, spec.range_offset, cell.key)
          != Condition::GT)
      return false;

    if(cell.key.count > any_is && spec.range_end.count > any_is && 
       !spec.is_matching_end(key_seq, cell.key)) {
      stop = true;
      //SWC_PRINT << "-- KEY-BEGIN NO MATCH STOP --" << SWC_PRINT_CLOSE;
      return false;
    }

    size_t remain = cell.vlen;
    const uint8_t * ptr = cell.value;
    DB::Cell::Key key_end;
    key_end.decode(&ptr, &remain);

    if(key_end.count > any_is && range_begin.count > any_is && 
       !spec.is_matching_begin(key_seq, key_end)) {
      //SWC_PRINT << "-- KEY-END NO MATCH --" << SWC_PRINT_CLOSE;
      return false;
    }
    //return true; // without aligned min-max

    rid_t rid = Serialization::decode_vi64(&ptr, &remain); // rid

    DB::Cell::KeyVec aligned_min;
    aligned_min.decode(&ptr, &remain);
    DB::Cell::KeyVec aligned_max;
    aligned_max.decode(&ptr, &remain);
    /*
    SWC_PRINT 
      << "range_begin: " << range_begin.to_string() << "\n"
      << "  key_begin: " << cell.key.to_string() << "\n"
      << "aligned_min: " << aligned_min.to_string() << "\n"
      << "  range_end: " << spec.range_end.to_string() << "\n"
      << "    key_end: " << key_end.to_string() << "\n"
      << "aligned_max: " << aligned_max.to_string() << "\n"
      << "        rid: " << rid << SWC_PRINT_CLOSE;
    */
    if(range_begin.count == any_is || aligned_max.empty() || 
        DB::KeySeq::compare(key_seq, 
         range_begin, aligned_max, 
         Condition::LT, range_begin.count, true)) {
      if(spec.range_end.count == any_is || aligned_min.empty() || 
          DB::KeySeq::compare(key_seq, 
           spec.range_end, aligned_min, 
           Condition::GT, spec.range_end.count, true)) {
        //SWC_PRINT << "-- ALIGNED MATCH  --" << SWC_PRINT_CLOSE;
        return true;
      }
    }
    //SWC_PRINT << "-- ALIGNED NO MATCH -------" << SWC_PRINT_CLOSE;
    return false;
  }

  bool reached_limits() override {
    return params.cid && params.rid;
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    profile.add_cell(cell.encoded_length());

    params.range_begin.copy(cell.key); 
    std::string id_name(params.range_begin.get_string(0));
    params.cid = (cid_t)strtoll(id_name.c_str(), NULL, 0);

    const uint8_t* ptr = cell.value;
    size_t remain = cell.vlen;
    params.range_end.decode(&ptr, &remain, true);
    params.rid = Serialization::decode_vi64(&ptr, &remain);

    if(range->type == Types::Range::MASTER) {
      params.range_begin.remove(0);
      params.range_end.remove(0);
    }
    params.range_begin.remove(0);
    params.range_end.remove(0);
    return false;
  }

  
  void response(int &err) override {

    if(err)
      params.err = err;
    else if(RangerEnv::is_shuttingdown())
      params.err = Error::SERVER_SHUTTING_DOWN;
    else if(range->deleted())
      params.err = Error::COLUMN_MARKED_REMOVED;
    else if(!params.cid || !params.rid)
      params.err = Error::RANGE_NOT_FOUND;
    
    try {
      auto cbp = CommBuf::make(params);
      cbp->header.initialize_from_request_header(m_ev->header);
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
    
    profile.finished();
    SWC_LOGF(LOG_DEBUG, "Range(%lld/%lld) err=%d(%s) flags(%d) Locator-%s", 
      range->cfg->cid, range->rid, err, Error::get_text(err),
      (int)flags, profile.to_string().c_str());
  }

  RangePtr              range;
  uint8_t               flags;
  uint32_t              any_is;
  const DB::Cell::Key   range_begin;

  Protocol::Rgr::Params::RangeLocateRsp params;

};


}}}
#endif // swc_ranger_callbacks_RangeLocateScan_h
