/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Callbacks_h
#define swcdb_fs_Callbacks_h


#include "swcdb/core/Buffer.h"


namespace SWC { namespace FS {


//! The SWC-DB Callback C++ namespace 'SWC::FS::Callback'
namespace Callback {


  typedef std::function<void(int, bool)>         ExistsCb_t;
  typedef std::function<void(int)>               RemoveCb_t;
  typedef std::function<void(int, size_t)>       LengthCb_t;

  typedef std::function<void(int)>               MkdirsCb_t;
  typedef std::function<void(int, DirentList&&)> ReaddirCb_t;
  typedef std::function<void(int)>               RmdirCb_t;
  typedef std::function<void(int)>               RenameCb_t;


  typedef std::function<void(int, SmartFd::Ptr)>                    WriteCb_t;
  typedef std::function<void(int, const std::string&,
                             StaticBuffer::Ptr)>                    ReadAllCb_t;
  typedef std::function<void(int, SmartFd::Ptr,
                             StaticBuffer::Ptr)>                    CombiPreadCb_t;

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

#endif // swcdb_fs_Callbacks_h
