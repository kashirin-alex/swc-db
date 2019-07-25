/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Callbacks_h
#define swc_lib_fs_Callbacks_h

#include <functional>

namespace SWC{ namespace FS {

namespace Callback {

  typedef std::function<void(int&, bool)>         ExistsCb_t;
  typedef std::function<void(int&)>               MkdirsCb_t;
  typedef std::function<void(int&, DirentList)>   ReaddirCb_t;

}

}}

#endif  // swc_lib_fs_Callbacks_h