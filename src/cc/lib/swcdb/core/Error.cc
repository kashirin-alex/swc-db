/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Compat.h"
#include "swcdb/core/Error.h"
#include <map>
#include <future>
#include <netdb.h>



namespace SWC {


namespace {

std::map<const int, const char *> text_map {

  { Error::EXCEPTION_BAD,                  "Bad Exception" },
  { Error::EXCEPTION_UNKNOWN,              "Unknown Exception" },
  { Error::UNPOSSIBLE,                     "But that's unpossible!" },
  { Error::FAILED_EXPECTATION,             "Failed Expectation" },

  { Error::OK,                             "ok" },

  { Error::IO_ERROR,                        "i/o error" },
  { Error::BAD_MEMORY_ALLOCATION,           "bad memory allocation"},
  { Error::BAD_FUNCTION,                    "bad function"},
  { Error::BAD_POINTER,                     "bad pointer"},
  { Error::BAD_CAST,                        "bad cast"},
  { Error::BAD_FORMAT,                      "bad format"},
  { Error::BAD_REGEXP,                      "bad regexp"},
  { Error::BAD_LOGIC,                       "bad logic"},
  
  { Error::CANCELLED,                       "cancelled"},
  { Error::NOT_ALLOWED,                     "not allowed" },
  { Error::INVALID_ARGUMENT,                "invalid argument" },
  { Error::INCOMPATIBLE_OPTIONS,            "incompatible options" },

  { Error::SERIALIZATION_INPUT_OVERRUN,     "SERIALIZATION input overrun" },
  { Error::CHECKSUM_MISMATCH,               "checksum mismatch" },

  { Error::COMM_NOT_CONNECTED,              "COMM not connected" },
  { Error::COMM_CONNECT_ERROR,              "COMM connect error" },
  { Error::COMM_SEND_ERROR,                 "COMM send error" },
  { Error::COMM_HEADER_CHECKSUM_MISMATCH,   "COMM header checksum mismatch" },
  { Error::COMM_BAD_HEADER,                 "COMM bad header" },

  { Error::PROTOCOL_ERROR,                  "protocol error" },
  { Error::REQUEST_TRUNCATED_HEADER,        "request truncated header" },
  { Error::REQUEST_TRUNCATED_PAYLOAD,       "request truncated payload" },
  { Error::REQUEST_TIMEOUT,                 "request timeout" },
  { Error::NOT_IMPLEMENTED,                 "not implemented"},

  { Error::ENCODER_ENCODE,                  "encoder decode-error" },
  { Error::ENCODER_DECODE,                  "encoder encode-error" },

  { Error::CONFIG_BAD_CFG_FILE,             "bad cfg file"},
  { Error::CONFIG_GET_ERROR,                "failed to get config value"},
  { Error::CONFIG_BAD_VALUE,                "bad config value"},

  { Error::SERVER_NOT_READY,                "server not ready" },
  { Error::SERVER_SHUTTING_DOWN,            "server shutting down" },

  { Error::MNGR_NOT_ACTIVE,                 "Manager not active for the duty" },
  { Error::MNGR_NOT_INITIALIZED,            "Manager is initializing" },

  { Error::RGR_NOT_READY,                   "Ranger is not ready, lacks id"},
  { Error::RGR_NOT_LOADED_RANGE,            "Ranger range id not loaded"},
  { Error::RGR_DELETED_RANGE,               "Ranger range state deleted"},

  { Error::FS_BAD_FILE_HANDLE,              "FS bad file handle" },
  { Error::FS_PERMISSION_DENIED,            "FS permission denied" },
  { Error::FS_EOF,                          "FS end of file" },
  { Error::FS_PATH_NOT_FOUND,               "FS destination path" },

  { Error::COLUMN_NOT_READY,                "Column is not ready"},
  { Error::COLUMN_NOT_EXISTS,               "Column does not exist"},
  { Error::COLUMN_REACHED_ID_LIMIT,         "Columd ID max-reached"},
  { Error::COLUMN_MARKED_REMOVED,           "Column is being removed"},
  { Error::COLUMN_UNKNOWN_GET_FLAG,         "unknown get column flag!"},
    
  { Error::COLUMN_SCHEMA_NAME_EXISTS,       "Schema column name already exists!"},
  { Error::COLUMN_SCHEMA_NAME_NOT_EXISTS,   "Schema column name doesn't exist!"},
  { Error::COLUMN_SCHEMA_BAD_SAVE,          "Schema for save not matches saved"},
  { Error::COLUMN_SCHEMA_NAME_EMPTY,        "Schema's column name cannot be empty"},
  { Error::COLUMN_SCHEMA_NAME_NOT_CORRES,   "Schema's column name not correspond"},
  { Error::COLUMN_SCHEMA_ID_EMPTY,          "Schema's column id cannot be empty"},
  { Error::COLUMN_SCHEMA_NOT_DIFFERENT,     "Schemas expected to be different"},
  { Error::COLUMN_SCHEMA_MISSING,           "cid's Schema is missing"},
  { Error::COLUMN_SCHEMA_IS_SYSTEM,         "Action not allowed on system schema"},
  { Error::COLUMN_CHANGE_INCOMPATIBLE,      "Schema seq/type/sysname incompatible"},
  
  { Error::RANGE_NOT_FOUND,                 "No corresponding range"},
  { Error::RANGE_CELLSTORES,                "Cellstores had Errors"},
  { Error::RANGE_COMMITLOG,                 "Commitlog had Errors"},
  { Error::RANGE_BAD_INTERVAL,              "Partial Write, bad range interval begin /& end"},
  { Error::RANGE_BAD_CELLS_INPUT,           "Malformed Cells-Input"},

  { Error::SQL_PARSE_ERROR,                 "SQL parse error" },
  { Error::SQL_BAD_LOAD_FILE_FORMAT,        "SQL bad load file format" },

  { Error::CLIENT_DATA_REMAINED,            "Data remained on client input" },

};

static const char ERROR_NOT_REGISTERED[] = "ERROR NOT REGISTERED";

} // local namespace



SWC_SHOULD_NOT_INLINE
const char* Error::get_text(const int err) {
  const char* text = nullptr;
  switch(err) {
    case SWC_ERRNO_SYS_BEGIN ... SWC_ERRNO_SYS_END:
       text = strerror(err);
       break;
    case SWC_ERRNO_FUTURE_BEGIN ... SWC_ERRNO_FUTURE_END:
       text =  std::future_error(
         std::future_errc(err - SWC_ERRNO_FUTURE_BEGIN)).what();
       break;
    case SWC_ERRNO_EAI_BEGIN ... SWC_ERRNO_EAI_END:
       text = gai_strerror(-(err - SWC_ERRNO_EAI_BEGIN));
       break;
    case SWC_ERRNO_APP_BEGIN ... SWC_ERRNO_APP_END:
       text = text_map[err];
       break;
    default:
      break;
  }
  return text ? text : ERROR_NOT_REGISTERED;
}

SWC_SHOULD_NOT_INLINE
void Error::print(std::ostream& out, int err) {
  out << "error=" << err << '(' << get_text(err) << ')';
}


}
