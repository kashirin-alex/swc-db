/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_Commands_h
#define swcdb_fs_Broker_Protocol_Commands_h

namespace SWC { namespace FsBroker { namespace Protocol {
  
  enum Cmd : uint8_t {
    NOT_IMPLEMENTED   = 0x00,   ///< Not Implemented
    FUNCTION_OPEN     = 0x01,   ///< Open
    FUNCTION_CREATE   = 0x02,   ///< Create
    FUNCTION_CLOSE    = 0x03,   ///< Close
    FUNCTION_READ     = 0x04,   ///< Read
    FUNCTION_APPEND   = 0x05,   ///< Append
    FUNCTION_SEEK     = 0x06,   ///< Seek
    FUNCTION_REMOVE   = 0x07,   ///< Remove
    FUNCTION_LENGTH   = 0x08,   ///< Length
    FUNCTION_PREAD    = 0x09,   ///< Pread
    FUNCTION_MKDIRS   = 0x0a,   ///< Mkdirs
    FUNCTION_FLUSH    = 0x0b,   ///< Flush
    FUNCTION_RMDIR    = 0x0c,   ///< Rmdir
    FUNCTION_READDIR  = 0x0d,   ///< Readdir
    FUNCTION_EXISTS   = 0x0e,   ///< Exists
    FUNCTION_RENAME   = 0x0f,   ///< Rename
    FUNCTION_SYNC     = 0x10,   ///< Sync
    FUNCTION_WRITE    = 0x11,   ///< Write
    FUNCTION_READ_ALL = 0x12,   ///< ReadAll
    
    FUNCTION_DEBUG    = 0x13,   ///< Debug
    FUNCTION_STATUS   = 0x13,   ///< Status
    FUNCTION_SHUTDOWN = 0x13,   ///< Shutdown
    FUNCTION_MAX      = 0x13    ///< Maximum code marker
  };
}

}}

#endif // swcdb_fs_Broker_Protocol_Commands_h