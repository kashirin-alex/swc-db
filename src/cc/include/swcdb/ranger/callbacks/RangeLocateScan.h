/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangeLocateScan_h
#define swc_lib_ranger_callbacks_RangeLocateScan_h

#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Cells/ReqScan.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeLocateScan : public DB::Cells::ReqScan {
  public:

  RangeLocateScan(ConnHandlerPtr conn, Event::Ptr ev, 
                  const DB::Specs::Interval& spec, 
                  DB::Cells::Mutable& cells,
                  Range::Ptr range, uint8_t flags)
                  : DB::Cells::ReqScan(conn, ev, spec, cells), 
                    range(range), flags(flags),
                    any_is(range->type != Types::Range::DATA) {
    has_selector = true;
  }

  virtual ~RangeLocateScan() { }

  const bool selector(const DB::Cells::Cell& cell, bool& stop) override {
    /*
    std::cout << "range_begin: " << spec.range_begin.to_string() << "\n";
    std::cout << "range___end: " << spec.range_end.to_string() << "\n";
    std::cout << "key___begin: " << cell.key.to_string() << "\n";
    */
    if(flags & Protocol::Rgr::Params::RangeLocateReq::COMMIT) {
      if(any_is && spec.range_begin.compare(cell.key, any_is) != Condition::EQ)
        return false;
    } else {
      if(spec.range_end.count > any_is
          && spec.range_end.compare(cell.key, spec.range_end.count) 
                                                    == Condition::GT) {
        stop = true;
        return false;
      }
      stop = cells.size() == 1;
      if(spec.range_begin.compare(cell.key) == Condition::LT)
        return false;
    }

    size_t remain = cell.vlen;
    const uint8_t * ptr = cell.value;
    DB::Cell::Key key_end;
    key_end.decode(&ptr, &remain);
    
    //std::cout << "key_____end: " << key_end.to_string() << "\n";
    //std::cout << "range____id: " 
    //          << Serialization::decode_vi64(&ptr, &remain) << "\n";

    bool match;
    if(flags & Protocol::Rgr::Params::RangeLocateReq::COMMIT) {
      match = key_end.count == any_is ||
        key_end.compare(spec.range_begin, key_end.count) != Condition::GT;
      if(match)
        stop = true;
    } else {
      match = key_end.count == any_is ||
              spec.range_end.count == any_is ||
              spec.range_end.compare(key_end) != Condition::GT;
      if(match && !(flags & Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE))
        stop = true;
    }
    return match;
  }

  void response(int &err) override {
    if(!DB::Cells::ReqScan::ready(err))
      return;
      
    if(!err) {
      if(RangerEnv::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      if(range->deleted())
        err = Error::COLUMN_MARKED_REMOVED;
    }
    if(err == Error::COLUMN_MARKED_REMOVED)
      cells.free();
    
    //SWC_LOG_OUT(LOG_INFO) << spec.to_string() << SWC_LOG_OUT_END;

    Protocol::Rgr::Params::RangeLocateRsp params(err);
    if(!err) {
      if(cells.size()) {

        if(flags & Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE && 
           cells.size() != 2) {        
          params.err = Error::RANGE_NOT_FOUND;
        } else {

          DB::Cells::Cell cell;
          cells.get(
            flags & Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE ? 1 : 0,
            cell
          );
  
          params.range_begin.copy(cell.key);
        
          std::string id_name(params.range_begin.get_string(0));
          params.cid = (int64_t)strtoll(id_name.c_str(), NULL, 0);

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
        }
        
      } else if(spec.range_begin.count > 1) {
        spec.range_begin.remove(spec.range_begin.count-1, true);
        range->scan(get_req_scan());
        return;

      } else {
        params.err = Error::RANGE_NOT_FOUND;
      }
    }

    SWC_LOG_OUT(LOG_DEBUG) 
      << params.to_string() << " flags=" << (int)flags 
      << SWC_LOG_OUT_END;
    
    try {
      auto cbp = CommBuf::make(params);
      cbp->header.initialize_from_request_header(m_ev->header);
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
    
  }

  Range::Ptr  range;
  uint8_t     flags;
  uint32_t    any_is;

};


}
}}}
#endif // swc_lib_ranger_callbacks_RangeLocateScan_h
