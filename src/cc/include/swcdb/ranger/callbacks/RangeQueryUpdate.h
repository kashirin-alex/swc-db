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
  StaticBuffer          input;
  Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp  rsp;

  RangeQueryUpdate(const Comm::ConnHandlerPtr& conn,
                   const Comm::Event::Ptr& ev,
                   StaticBuffer& input)
                  : conn(conn), ev(ev), input(input), rsp(Error::OK) {
    Env::Rgr::res().more_mem_usage(size_of());
  }

  ~RangeQueryUpdate() {
    Env::Rgr::res().less_mem_usage(size_of());
  }

  size_t size_of() const {
    return sizeof(this) + sizeof(*this) + input.size;
  }

  bool expired() const {
    return  (ev && ev->expired(input.size/100000)) ||
            (conn && !conn->is_open());
  }

  void response() {
    conn->send_response(Comm::Buffers::make(ev, rsp));
  }


};



}}}


#endif // swcdb_ranger_callbacks_RangeQueryUpdate_h
