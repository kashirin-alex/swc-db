/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_Commands_h
#define swcdb_fs_Broker_Protocol_Commands_h


#include "swcdb/core/Compat.h"


namespace SWC { namespace Comm { namespace Protocol {



/**
 * @brief The SWC-DB FsBroker Communications Protocol C++ namespace 'SWC::Comm::Protocol::FsBroker'
 *
 * \ingroup FileSystem
 */
namespace FsBroker {

  // FsBroker Protocol Commands
  enum Command : uint8_t {
    NOT_IMPLEMENTED       = 0x00,   ///< Not Implemented
    FUNCTION_OPEN         = 0x01,   ///< Open
    FUNCTION_CREATE       = 0x02,   ///< Create
    FUNCTION_CLOSE        = 0x03,   ///< Close
    FUNCTION_READ         = 0x04,   ///< Read
    FUNCTION_APPEND       = 0x05,   ///< Append
    FUNCTION_SEEK         = 0x06,   ///< Seek
    FUNCTION_REMOVE       = 0x07,   ///< Remove
    FUNCTION_LENGTH       = 0x08,   ///< Length
    FUNCTION_PREAD        = 0x09,   ///< Pread
    FUNCTION_MKDIRS       = 0x0a,   ///< Mkdirs
    FUNCTION_FLUSH        = 0x0b,   ///< Flush
    FUNCTION_RMDIR        = 0x0c,   ///< Rmdir
    FUNCTION_READDIR      = 0x0d,   ///< Readdir
    FUNCTION_EXISTS       = 0x0e,   ///< Exists
    FUNCTION_RENAME       = 0x0f,   ///< Rename
    FUNCTION_SYNC         = 0x10,   ///< Sync
    FUNCTION_WRITE        = 0x11,   ///< Write
    FUNCTION_READ_ALL     = 0x12,   ///< ReadAll
    FUNCTION_COMBI_PREAD  = 0x13,   ///< CombiPread open+pread+close

    FUNCTION_DEBUG        = 0x14,   ///< Debug
    FUNCTION_STATUS       = 0x14,   ///< Status
    FUNCTION_SHUTDOWN     = 0x14,   ///< Shutdown
    MAX_CMD               = 0x14    ///< Maximum code marker
  };

  struct Commands {
    static constexpr const uint8_t MAX = MAX_CMD;

    static const char* to_string(uint8_t cmd) noexcept;
  };

}

}}}



#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/Commands.cc"
#endif


#endif // swcdb_fs_Broker_Protocol_Commands_h
