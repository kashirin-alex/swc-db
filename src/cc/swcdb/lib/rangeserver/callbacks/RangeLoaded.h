/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_rangeserver_callbacks_RangeLoaded_h
#define swc_lib_rangeserver_callbacks_RangeLoaded_h

#include "swcdb/lib/core/comm/ResponseCallback.h"

namespace SWC {
namespace server {
namespace RS {
namespace Callback {


class RangeLoaded : public ResponseCallback {
  public:

  RangeLoaded(ConnHandlerPtr conn, EventPtr ev, 
              const int64_t cid, const int64_t rid)
              : ResponseCallback(conn, ev), cid(cid), rid(rid) {
    Env::RsData::in_process(1);
  }

  virtual ~RangeLoaded() { }

  void response(int &err) override {
    if(err == Error::OK && Env::RsData::is_shuttingdown()) 
      err = Error::SERVER_SHUTTING_DOWN;

    if(err == Error::OK) {
      m_conn->response_ok(m_ev);
      Env::RsData::in_process(-1);
      return;
    }

    Env::RsColumns::get()->unload_range(err, cid, rid, 
      [berr=err, ptr=shared_from_this()]
      (int err){
        ptr->send_error(berr, "");
        Env::RsData::in_process(-1);
      }
    );
    
  }

  private:
  const int64_t cid;
  const int64_t rid;

};
typedef std::shared_ptr<RangeLoaded> RangeLoadedPtr;


}
}}}
#endif // swc_lib_rangeserver_callbacks_RangeLoaded_h
