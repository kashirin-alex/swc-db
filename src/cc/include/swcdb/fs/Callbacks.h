/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Callbacks_h
#define swc_lib_fs_Callbacks_h

#include <functional>
#include "swcdb/core/StaticBuffer.h"

namespace SWC{ namespace FS {

namespace Callback {

  typedef std::function<void(int, bool)>         ExistsCb_t;
  typedef std::function<void(int)>               RemoveCb_t;
  typedef std::function<void(int, size_t)>       LengthCb_t;

  typedef std::function<void(int)>               MkdirsCb_t;
  typedef std::function<void(int, DirentList)>   ReaddirCb_t;
  typedef std::function<void(int)>               RmdirCb_t;
  typedef std::function<void(int)>               RenameCb_t;
  

  typedef std::function<void(int, SmartFd::Ptr)>                    WriteCb_t;
  typedef std::function<void(int, SmartFd::Ptr)>                    CreateCb_t;
  typedef std::function<void(int, SmartFd::Ptr)>                    OpenCb_t;
  typedef std::function<void(int, SmartFd::Ptr, StaticBuffer::Ptr)> ReadCb_t;
  typedef std::function<void(int, SmartFd::Ptr, StaticBuffer::Ptr)> PreadCb_t;
  typedef std::function<void(int, SmartFd::Ptr, size_t)>            AppendCb_t;
  typedef std::function<void(int, SmartFd::Ptr)>                    SeekCb_t;
  typedef std::function<void(int, SmartFd::Ptr)>                    FlushCb_t;
  typedef std::function<void(int, SmartFd::Ptr)>                    SyncCb_t;
  typedef std::function<void(int, SmartFd::Ptr)>                    CloseCb_t;
 
}

}}

#endif  // swc_lib_fs_Callbacks_h