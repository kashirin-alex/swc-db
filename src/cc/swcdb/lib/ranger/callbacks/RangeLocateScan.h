/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangeLocateScan_h
#define swc_lib_ranger_callbacks_RangeLocateScan_h

#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/db/Cells/ReqScan.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeLocateScan : public DB::Cells::ReqScan {
  public:

  RangeLocateScan(ConnHandlerPtr conn, Event::Ptr ev, 
                  DB::Specs::Interval::Ptr spec, 
                  DB::Cells::Mutable::Ptr cells,
                  Range::Ptr range)
                  : DB::Cells::ReqScan(conn, ev, spec, cells), 
                    range(range) {
    has_selector = true;
  }

  virtual ~RangeLocateScan() { }

  bool selector(const DB::Cells::Cell& cell) override {  // ref bool stop
    if(!spec->key_start.is_matching(cell.key)) 
      return cells->size() == 1; // next_key

    size_t remain = cell.vlen;
    const uint8_t * ptr = cell.value;
    Serialization::decode_vi64(&ptr, &remain);
    DB::Cell::Key key_end;
    key_end.decode(&ptr, &remain);
    std::cout << "cell begin: "<< cell.key.to_string() << "\n";
    std::cout << "spec begin: " << spec->key_start.to_string() << "\n";
    std::cout << "cell end:   "<< key_end.to_string() << "\n";
    std::cout << "spec end:   " << spec->key_finish.to_string() << "\n";
    return key_end.empty() || 
            spec->key_finish.is_matching(key_end);
  }

  void response(int &err) override {
    if(!DB::Cells::ReqScan::ready(err))
      return;
      
    if(err == Error::OK) {
      if(Env::RgrData::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      if(range->deleted())
        err = Error::COLUMN_MARKED_REMOVED;
    }
    if(err == Error::COLUMN_MARKED_REMOVED)
      cells->free();


    Protocol::Rgr::Params::RangeLocateRsp params(err);
    if(err == Error::OK) {
      if(cells->size() > 0) {

        DB::Cells::Cell cell;
        cells->get(0, cell);
        
        std::string id_name(cell.key.get_string(0));
        params.cid = (int64_t)strtoll(id_name.c_str(), NULL, 0);

        const uint8_t* ptr = cell.value;
        size_t remain = cell.vlen;
        params.rid = Serialization::decode_vi64(&ptr, &remain);
        params.key_end.decode(&ptr, &remain, true);
        if(range->type == Types::Range::MASTER) 
          params.key_end.remove(0);
        params.key_end.remove(0);

        params.next_key = cells->size() > 1;
        
      } else  {
        // range->cid == 1 || 2
        if(spec->key_start.count > 1) {
          spec->offset_key.free();
          spec->key_finish.copy(spec->key_start);
          spec->key_finish.set(-1, Condition::LE);
          if(range->type != Types::Range::DATA)
            spec->key_finish.set(0, Condition::EQ);
          spec->key_start.remove(spec->key_start.count-1, true);
          range->scan(get_req_scan());
          return;
        } else {
          params.err = Error::RANGE_NOT_FOUND;
        }
      }
    }

    std::cout << "RangeLocateScan, rsp " << to_string() << "\n";    
    std::cout << params.to_string() << "\n";
    
    try {
      auto cbp = CommBuf::make(params);
      cbp->header.initialize_from_request_header(m_ev->header);
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
    
  }

  Range::Ptr              range;

};


}
}}}
#endif // swc_lib_ranger_callbacks_RangeLocateScan_h
