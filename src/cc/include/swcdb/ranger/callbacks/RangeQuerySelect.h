/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_ranger_callbacks_RangeQuerySelect_h
#define swc_ranger_callbacks_RangeQuerySelect_h

#include "swcdb/core/comm/ResponseCallback.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelect : public ReqScan {
  public:

  RangeQuerySelect(ConnHandlerPtr conn, Event::Ptr ev, 
                   const DB::Specs::Interval& spec, 
                   DB::Cells::Result& cells, 
                   RangePtr range, uint32_t limit_buffer)
                  : ReqScan(conn, ev, spec, cells), 
                    range(range) {
    limit_buffer_sz = limit_buffer;
  }

  virtual ~RangeQuerySelect() { }

  void response(int &err) override {
    if(!ReqScan::ready(err))
      return;
      
    if(err == Error::OK) {
      if(RangerEnv::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      if(range->deleted())
        err = Error::COLUMN_MARKED_REMOVED;
    }
    if(err == Error::COLUMN_MARKED_REMOVED)
      cells.free();
    
    Protocol::Rgr::Params::RangeQuerySelectRsp params(
      err,  
      limit_buffer_sz <= cells.size_bytes(),
      offset,
      upto_revision
    );

    CommBuf::Ptr cbp;
    if(!cells.empty()) {
      DynamicBuffer buffer;
      cells.write(buffer);
      StaticBuffer sndbuf(buffer);
      cbp = CommBuf::make(params, sndbuf);
    } else {
      cbp = CommBuf::make(params);
    }
    cbp->header.initialize_from_request_header(m_ev->header);
    
    try {
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
    
  }

  RangePtr  range;

};


}}}
#endif // swc_ranger_callbacks_RangeLocateScan_h
