/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_ranger_callbacks_RangesScan_h
#define swc_lib_ranger_callbacks_RangesScan_h

#include "swcdb/lib/core/comm/ResponseCallback.h"
#include "swcdb/lib/db/Cells/Mutable.h"
//#include "swcdb/lib/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC {
namespace server {
namespace Rgr {
namespace Callback {


class RangeScan : public ResponseCallback {
  public:
  typedef std::shared_ptr<RangeScan>  Ptr;
  std::atomic<int> count=0;
  int id=0;
  int64_t took;
  RangeScan(ConnHandlerPtr conn, EventPtr ev,
            DB::Specs::Interval::Ptr spec, DB::Cells::Mutable::Ptr cells) 
            : ResponseCallback(conn, ev), spec(spec), cells(cells) {
    took = SWC::Time::now_ns();
    id = ++count;
  }
  
  virtual ~RangeScan() { }

  void response(int &err) override {

  
    try {
      /*
      Protocol::Rgr::Params::RangeLoaded params(range->get_interval());
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(
        header, 4+params.encoded_length());
      cbp->append_i32(err);
      params.encode(cbp->get_data_ptr_address());

      m_conn->send_response(cbp);
      return;
      */
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = Error::COMM_SEND_ERROR;
      
    }
    
    std::cout << " chk=" << id << " took=" <<  SWC::Time::now_ns()-took 
              << " cells:" << cells->to_string() << "\n";
    
  }

  DB::Specs::Interval::Ptr   spec;
  DB::Cells::Mutable::Ptr    cells;
};


}
}}}
#endif // swc_lib_ranger_callbacks_RangesScan_h
