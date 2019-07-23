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

  RangeLoaded(ConnHandlerPtr conn, EventPtr ev)
              : ResponseCallback(conn, ev) { }

  virtual ~RangeLoaded() { return; }

  void run() override {

  }

};
typedef std::shared_ptr<RangeLoaded> RangeLoadedPtr;


}
}}}
#endif // swc_lib_rangeserver_callbacks_RangeLoaded_h
