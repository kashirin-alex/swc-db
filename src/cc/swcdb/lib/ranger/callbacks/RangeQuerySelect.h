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

  RangeQuerySelect(ConnHandlerPtr conn, EventPtr ev, 
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
    
    CommBufPtr cbp;
    CommHeader header;
    header.initialize_from_request_header(m_ev->header);
    Protocol::Rgr::Params::RangeQuerySelectRsp params(err);

    if(cells->size() > 0) {
      DynamicBuffer buffer;
      cells->write(buffer);

      params.reached_limit = limit_buffer_sz <= cells->size_bytes();
      params.size = buffer.fill();
      
      StaticBuffer sndbuf(buffer);
      cbp = std::make_shared<CommBuf>(header, params.encoded_length(), sndbuf);

      // temp checkup
      const uint8_t* ptr = buffer.base;
      size_t remainp = buffer.size;
      DB::Cells::Cell cell;
      while(remainp) {
        cell.read(&ptr, &remainp);
        if(cell.flag == DB::Cells::NONE) {
          std::cerr << "RangeQuerySelect remainp=" << remainp << " FLAG::NONE " << cell.to_string() << "\n";
          exit(1);
        }
      }
    } else {
      cbp = std::make_shared<CommBuf>(header, params.encoded_length());
    }
    
    std::cout << "RangeQuerySelect, rsp " << to_string() << "\n";
    std::cout << params.to_string() << "\n";

    try {
      params.encode(cbp->get_data_ptr_address());
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
