/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_Commands_h
#define swc_lib_fs_Broker_Protocol_Commands_h

namespace SWC{ namespace FS { namespace Protocol {
  
  enum Cmd {
    FUNCTION_OPEN = 0, ///< Open
    FUNCTION_CREATE,   ///< Create
    FUNCTION_CLOSE,    ///< Close
    FUNCTION_READ,     ///< Read
    FUNCTION_APPEND,   ///< Append
    FUNCTION_SEEK,     ///< Seek
    FUNCTION_REMOVE,   ///< Remove
    FUNCTION_SHUTDOWN, ///< Shutdown
    FUNCTION_LENGTH,   ///< Length
    FUNCTION_PREAD,    ///< Pread
    FUNCTION_MKDIRS,   ///< Mkdirs
    FUNCTION_STATUS,   ///< Status
    FUNCTION_FLUSH,    ///< Flush
    FUNCTION_RMDIR,    ///< Rmdir
    FUNCTION_READDIR,  ///< Readdir
    FUNCTION_EXISTS,   ///< Exists
    FUNCTION_RENAME,   ///< Rename
    FUNCTION_DEBUG,    ///< Debug
    FUNCTION_SYNC,     ///< Sync
    FUNCTION_MAX       ///< Maximum code marker
  };
}

}}

#endif  // swc_lib_fs_Broker_Protocol_Commands_h