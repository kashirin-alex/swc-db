/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol { namespace FsBroker {


const char* Commands::to_string(uint8_t cmd) noexcept {
  switch(cmd) {
    case FUNCTION_OPEN:
      return "OPEN";
    case FUNCTION_CREATE:
      return "CREATE";
    case FUNCTION_CLOSE:
      return "CLOSE";
    case FUNCTION_READ:
      return "READ";
    case FUNCTION_APPEND:
      return "APPEND";
    case FUNCTION_SEEK:
      return "SEEK";
    case FUNCTION_REMOVE:
      return "REMOVE";
    case FUNCTION_LENGTH:
      return "LENGTH";
    case FUNCTION_PREAD:
      return "PREAD";
    case FUNCTION_MKDIRS:
      return "MKDIRS";
    case FUNCTION_FLUSH:
      return "FLUSH";
    case FUNCTION_RMDIR:
      return "RMDIR";
    case FUNCTION_READDIR:
      return "READDIR";
    case FUNCTION_EXISTS:
      return "EXISTS";
    case FUNCTION_RENAME:
      return "RENAME";
    case FUNCTION_SYNC:
      return "SYNC";
    case FUNCTION_WRITE:
      return "WRITE";
    case FUNCTION_READ_ALL:
      return "READ_ALL";
    case FUNCTION_COMBI_PREAD:
      return "COMBI_PREAD";
    default:
      return "NOIMPL";
  }
}

}}}}
