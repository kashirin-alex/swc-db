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
                  const DB::Specs::Interval& spec, 
                  DB::Cells::Result& cells,
                  RangePtr range, uint8_t flags)
                  : ReqScan(conn, ev, spec, cells), 
                    range(range), flags(flags),
                    any_is(range->type != Types::Range::DATA) {
  }

  virtual ~RangeLocateScan() { }

  bool selector(const DB::Cells::Cell& cell, bool& stop) override {
    if(any_is && spec.range_begin.compare(cell.key, any_is) != Condition::EQ)
      return false;

    if(flags & Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE && 
       spec.range_offset.compare(cell.key) != Condition::GT)
      return false;

    if(cell.key.count > any_is && spec.range_end.count > any_is && 
       !spec.is_matching_end(cell.key)) {
      stop = true;
      //std::cout << "-- KEY-BEGIN NO MATCH STOP --\n";
      return false;
    }

    size_t remain = cell.vlen;
    const uint8_t * ptr = cell.value;
    DB::Cell::Key key_end;
    key_end.decode(&ptr, &remain);

    if(key_end.count > any_is && spec.range_begin.count > any_is && 
       !spec.is_matching_begin(key_end)) {
      //std::cout << "-- KEY-END NO MATCH --\n";
      return false;
    }
    //return true; // without aligned min-max

    int64_t rid = Serialization::decode_vi64(&ptr, &remain); // rid

    DB::Cell::KeyVec aligned_min;
    aligned_min.decode(&ptr, &remain);
    DB::Cell::KeyVec aligned_max;
    aligned_max.decode(&ptr, &remain);
    /*
    std::cout << "---------------------\n";
    std::cout << "range_begin: " << spec.range_begin.to_string() << "\n";
    std::cout << "  key_begin: " << cell.key.to_string() << "\n";
    std::cout << "aligned_min: " << aligned_min.to_string() << "\n";
    std::cout << "  range_end: " << spec.range_end.to_string() << "\n";
    std::cout << "    key_end: " << key_end.to_string() << "\n";
    std::cout << "aligned_max: " << aligned_max.to_string() << "\n";
    std::cout << "        rid: " << rid << "\n";
    */
    if(spec.range_begin.count == any_is || aligned_max.empty() || 
       spec.range_begin.compare(
         aligned_max, Condition::LT, spec.range_begin.count, true)) {
      if(spec.range_end.count == any_is || aligned_min.empty() || 
         spec.range_end.compare(
           aligned_min, Condition::GT, spec.range_end.count, true)) {
        //std::cout << "-- ALIGNED MATCH  --\n";
        return true;
      }
    }
    //std::cout << "-- ALIGNED NO MATCH -------\n";
    return false;
  }

  void response(int &err) override {
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
      if(!cells.empty()) {

        auto cell = cells.front();
  
        params.range_begin.copy(cell->key);
        
        std::string id_name(params.range_begin.get_string(0));
        params.cid = (int64_t)strtoll(id_name.c_str(), NULL, 0);

        const uint8_t* ptr = cell->value;
        size_t remain = cell->vlen;
        params.range_end.decode(&ptr, &remain, true);
        params.rid = Serialization::decode_vi64(&ptr, &remain);

        if(range->type == Types::Range::MASTER) {
          params.range_begin.remove(0);
          params.range_end.remove(0);
        }
        params.range_begin.remove(0);
        params.range_end.remove(0);
      /*
      } else if(spec.range_begin.count > 1) {
        spec.range_begin.remove(spec.range_begin.count-1, true);
        range->scan(get_req_scan());
        return;
      */
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

  RangePtr    range;
  uint8_t     flags;
  uint32_t    any_is;

};


}}}
#endif // swc_ranger_callbacks_RangeLocateScan_h
