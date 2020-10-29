/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQueryUpdate_h
#define swcdb_ranger_callbacks_RangeQueryUpdate_h

#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeQueryUpdate : public Comm::ResponseCallback {
  public:
  typedef std::shared_ptr<RangeQueryUpdate> Ptr;

  RangeQueryUpdate(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev)
                  : Comm::ResponseCallback(conn, ev) {
  }

  virtual ~RangeQueryUpdate() { }

  void response(int &err) override {
    response(Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp(err));
  }

  void response(const int& err, const DB::Cell::Key& range_prev_end, 
                                const DB::Cell::Key& range_end) {
    response(
      Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp(
        err, range_prev_end, range_end));
  }

  void response(const Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp& params) {
    m_conn->send_response(Comm::Buffers::make(params), m_ev);
  }


};


}}}
#endif // swcdb_ranger_callbacks_RangeQueryUpdate_h
