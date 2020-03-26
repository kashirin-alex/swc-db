/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_ranger_callbacks_RangeQueryUpdate_h
#define swc_ranger_callbacks_RangeQueryUpdate_h

#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeQueryUpdate : public ResponseCallback {
  public:
  typedef std::shared_ptr<RangeQueryUpdate> Ptr;

  RangeQueryUpdate(ConnHandlerPtr conn, Event::Ptr ev)
                  : ResponseCallback(conn, ev) {
  }

  virtual ~RangeQueryUpdate() { }

  void response(int &err) override {
    response(Protocol::Rgr::Params::RangeQueryUpdateRsp(err));
  }

  void response(const int& err, const DB::Cell::Key& range_prev_end, 
                                const DB::Cell::Key& range_end) {
    response(
      Protocol::Rgr::Params::RangeQueryUpdateRsp(
        err, range_prev_end, range_end));
  }

  const bool expired(const int64_t within=0) const {
    return (m_ev != nullptr && m_ev->expired(within)) || 
           (m_conn != nullptr && !m_conn->is_open()) ;
  }
  
  void response(const Protocol::Rgr::Params::RangeQueryUpdateRsp &params) {
    try {
      auto cbp = CommBuf::make(params);
      cbp->header.initialize_from_request_header(m_ev->header);
      m_conn->send_response(cbp);
    } catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
  }


};


}}}
#endif // swc_ranger_callbacks_RangeQueryUpdate_h
