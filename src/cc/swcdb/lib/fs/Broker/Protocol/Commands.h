/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_Commands_h
#define swc_lib_fs_Broker_Protocol_Commands_h

namespace SWC{ namespace FS { namespace Protocol {
  
  enum Cmd {
    FUNCTION_OPEN     = 0x00,    ///< Open
    FUNCTION_CREATE   = 0x01,    ///< Create
    FUNCTION_CLOSE    = 0x02,    ///< Close
    FUNCTION_READ     = 0x03,    ///< Read
    FUNCTION_APPEND   = 0x04,    ///< Append
    FUNCTION_SEEK     = 0x05,    ///< Seek
    FUNCTION_REMOVE   = 0x06,    ///< Remove
    FUNCTION_SHUTDOWN = 0x07,    ///< Shutdown
    FUNCTION_LENGTH   = 0x08,    ///< Length
    FUNCTION_PREAD    = 0x09,    ///< Pread
    FUNCTION_MKDIRS   = 0x0a,   ///< Mkdirs
    FUNCTION_STATUS   = 0x0b,   ///< Status
    FUNCTION_FLUSH    = 0x0c,   ///< Flush
    FUNCTION_RMDIR    = 0x0d,   ///< Rmdir
    FUNCTION_READDIR  = 0x0f,   ///< Readdir
    FUNCTION_EXISTS   = 0x10,   ///< Exists
    FUNCTION_RENAME   = 0x11,   ///< Rename
    FUNCTION_DEBUG    = 0x12,   ///< Debug
    FUNCTION_SYNC     = 0x14,   ///< Sync
    FUNCTION_WRITE    = 0x15,   ///< Write
    FUNCTION_MAX      = 0x16 ///< Maximum code marker
  };
}

}}

#endif  // swc_lib_fs_Broker_Protocol_Commands_h