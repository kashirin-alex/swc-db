/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_Commands_h
#define swc_lib_fs_Broker_Protocol_Commands_h

namespace SWC{ namespace FS { namespace Protocol {
  
  enum Cmd {
    FUNCTION_OPEN     = 0,    ///< Open
    FUNCTION_CREATE   = 1,    ///< Create
    FUNCTION_CLOSE    = 2,    ///< Close
    FUNCTION_READ     = 3,    ///< Read
    FUNCTION_APPEND   = 4,    ///< Append
    FUNCTION_SEEK     = 5,    ///< Seek
    FUNCTION_REMOVE   = 6,    ///< Remove
    FUNCTION_SHUTDOWN = 7,    ///< Shutdown
    FUNCTION_LENGTH   = 8,    ///< Length
    FUNCTION_PREAD    = 9,    ///< Pread
    FUNCTION_MKDIRS   = 10,   ///< Mkdirs
    FUNCTION_STATUS   = 11,   ///< Status
    FUNCTION_FLUSH    = 12,   ///< Flush
    FUNCTION_RMDIR    = 13,   ///< Rmdir
    FUNCTION_READDIR  = 14,   ///< Readdir
    FUNCTION_EXISTS   = 15,   ///< Exists
    FUNCTION_RENAME   = 16,   ///< Rename
    FUNCTION_DEBUG    = 17,   ///< Debug
    FUNCTION_SYNC     = 18,   ///< Sync
    FUNCTION_WRITE    = 19,   ///< Write
    FUNCTION_MAX ///< Maximum code marker
  };
}

}}

#endif  // swc_lib_fs_Broker_Protocol_Commands_h