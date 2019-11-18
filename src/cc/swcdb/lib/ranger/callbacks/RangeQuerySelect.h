/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangeQuerySelect_h
#define swc_lib_ranger_callbacks_RangeQuerySelect_h

#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/db/Cells/ReqScan.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeQuerySelect : public DB::Cells::ReqScan {
  public:

  RangeQuerySelect(ConnHandlerPtr conn, Event::Ptr ev, 
                   DB::Specs::Interval::Ptr spec, 
                   DB::Cells::Mutable::Ptr cells, 
                   Range::Ptr range, uint32_t limit_buffer)
                  : DB::Cells::ReqScan(conn, ev, spec, cells), 
                    range(range) {
    limit_buffer_sz = limit_buffer;
  }

  virtual ~RangeQuerySelect() { }

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
    
    Protocol::Rgr::Params::RangeQuerySelectRsp params(
      err,  
      limit_buffer_sz <= cells->size_bytes
    );

    CommBuf::Ptr cbp;
    if(cells->size > 0) {
      DynamicBuffer buffer;
      cells->write(buffer);
      StaticBuffer sndbuf(buffer);
      cbp = CommBuf::make(params, sndbuf);
    } else {
      cbp = CommBuf::make(params);
    }
    cbp->header.initialize_from_request_header(m_ev->header);
    
    //std::cout << "RangeQuerySelect, rsp " << to_string() << "\n";
    //std::cout << params.to_string() << "\n";

    try {
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
