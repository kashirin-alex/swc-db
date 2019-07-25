/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Protocol_callbacks_RspExists_h
#define swc_lib_fs_Protocol_callbacks_RspExists_h

#include <AsyncComm/CommBuf.h>
#include <AsyncComm/ResponseCallback.h>

#include <Common/Error.h>


namespace SWC {
namespace FS {
namespace Callback {

  class RspExists : public ResponseCallback {

  public:
  
    RspExists(ResponseCallback *cb) : cb(cb) {}

    RspExists(Comm *comm, EventPtr &event) : ResponseCallback(comm, event) { }

    virtual int response(bool exists) {
      cb->
    }
    ResponseCallback* cb;
  };


}}}


#endif // swc_lib_fs_Protocol_callbacks_RspExists_h
