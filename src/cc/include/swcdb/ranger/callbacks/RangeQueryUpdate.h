/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQueryUpdate_h
#define swcdb_ranger_callbacks_RangeQueryUpdate_h


#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"


namespace SWC { namespace Ranger { namespace Callback {



struct RangeQueryUpdate : Core::QueuePointer<RangeQueryUpdate*>::Pointer {

  Comm::ConnHandlerPtr  conn;
  Comm::Event::Ptr      ev;
  Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp  rsp;

  SWC_CAN_INLINE
  RangeQueryUpdate(const Comm::ConnHandlerPtr& conn,
                   const Comm::Event::Ptr& ev)
                  : conn(conn), ev(ev), rsp(Error::OK) {
  }

  SWC_CAN_INLINE
  ~RangeQueryUpdate() { }

  SWC_CAN_INLINE
  bool expired() const {
    return ev->expired(ev->data_ext.size/100000) || !conn->is_open();
  }

  SWC_CAN_INLINE
  void response() {
    conn->send_response(Comm::Buffers::make(ev, rsp));
  }


};



}}}


#endif // swcdb_ranger_callbacks_RangeQueryUpdate_h
