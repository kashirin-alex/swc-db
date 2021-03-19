/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Error_h
#define swcdb_core_Error_h


#include <ostream>



namespace SWC {


/**
 * @brief The SWC-DB Error C++ namespace 'SWC::Error'
 *
 * \ingroup Core
 */
namespace Error {


#define SWC_ERRNO_SYS_BEGIN     0
#define SWC_ERRNO_SYS_END       2048

#define SWC_ERRNO_FUTURE_BEGIN  2049
#define SWC_ERRNO_FUTURE_END    2059

#define SWC_ERRNO_EAI_BEGIN     2060
#define SWC_ERRNO_EAI_END       2080

#define SWC_ERRNO_APP_BEGIN     3000
#define SWC_ERRNO_APP_END       3100


enum Code : int32_t {

  EXCEPTION_BAD                                = -4,
  EXCEPTION_UNKNOWN                            = -3,
  UNPOSSIBLE                                   = -2,
  FAILED_EXPECTATION                           = -1,

  OK                                           = 0,

  IO_ERROR                                     = SWC_ERRNO_APP_BEGIN + 0,
  BAD_MEMORY_ALLOCATION                        = SWC_ERRNO_APP_BEGIN + 1,
  BAD_FUNCTION                                 = SWC_ERRNO_APP_BEGIN + 2,
  BAD_POINTER                                  = SWC_ERRNO_APP_BEGIN + 3,
  BAD_CAST                                     = SWC_ERRNO_APP_BEGIN + 4,
  BAD_FORMAT                                   = SWC_ERRNO_APP_BEGIN + 5,
  BAD_REGEXP                                   = SWC_ERRNO_APP_BEGIN + 6,
  BAD_LOGIC                                    = SWC_ERRNO_APP_BEGIN + 7,

  CANCELLED                                    = SWC_ERRNO_APP_BEGIN + 8,
  NOT_ALLOWED                                  = SWC_ERRNO_APP_BEGIN + 9,
  INVALID_ARGUMENT                             = SWC_ERRNO_APP_BEGIN + 10,
  INCOMPATIBLE_OPTIONS                         = SWC_ERRNO_APP_BEGIN + 11,

  SERIALIZATION_INPUT_OVERRUN                  = SWC_ERRNO_APP_BEGIN + 12,
  CHECKSUM_MISMATCH                            = SWC_ERRNO_APP_BEGIN + 13,

  COMM_NOT_CONNECTED                           = SWC_ERRNO_APP_BEGIN + 14,
  COMM_CONNECT_ERROR                           = SWC_ERRNO_APP_BEGIN + 15,
  COMM_SEND_ERROR                              = SWC_ERRNO_APP_BEGIN + 16,
  COMM_HEADER_CHECKSUM_MISMATCH                = SWC_ERRNO_APP_BEGIN + 17,
  COMM_BAD_HEADER                              = SWC_ERRNO_APP_BEGIN + 18,

  PROTOCOL_ERROR                               = SWC_ERRNO_APP_BEGIN + 19,
  REQUEST_TRUNCATED_HEADER                     = SWC_ERRNO_APP_BEGIN + 20,
  REQUEST_TRUNCATED_PAYLOAD                    = SWC_ERRNO_APP_BEGIN + 21,
  REQUEST_TIMEOUT                              = SWC_ERRNO_APP_BEGIN + 22,
  NOT_IMPLEMENTED                              = SWC_ERRNO_APP_BEGIN + 23,

  ENCODER_ENCODE                               = SWC_ERRNO_APP_BEGIN + 24,
  ENCODER_DECODE                               = SWC_ERRNO_APP_BEGIN + 25,

  CONFIG_BAD_CFG_FILE                          = SWC_ERRNO_APP_BEGIN + 26,
  CONFIG_GET_ERROR                             = SWC_ERRNO_APP_BEGIN + 27,
  CONFIG_BAD_VALUE                             = SWC_ERRNO_APP_BEGIN + 28,

  SERVER_NOT_READY                             = SWC_ERRNO_APP_BEGIN + 29,
  SERVER_SHUTTING_DOWN                         = SWC_ERRNO_APP_BEGIN + 30,
  SERVER_MEMORY_LOW                            = SWC_ERRNO_APP_BEGIN + 31,

  MNGR_NOT_ACTIVE                              = SWC_ERRNO_APP_BEGIN + 32,
  MNGR_NOT_INITIALIZED                         = SWC_ERRNO_APP_BEGIN + 33,

  RGR_NOT_READY                                = SWC_ERRNO_APP_BEGIN + 34,
  RGR_NOT_LOADED_RANGE                         = SWC_ERRNO_APP_BEGIN + 35,
  RGR_DELETED_RANGE                            = SWC_ERRNO_APP_BEGIN + 36,

  FS_BAD_FILE_HANDLE                           = SWC_ERRNO_APP_BEGIN + 37,
  FS_PERMISSION_DENIED                         = SWC_ERRNO_APP_BEGIN + 38,
  FS_EOF                                       = SWC_ERRNO_APP_BEGIN + 39,
  FS_PATH_NOT_FOUND                            = SWC_ERRNO_APP_BEGIN + 40,

  COLUMN_NOT_READY                             = SWC_ERRNO_APP_BEGIN + 41,
  COLUMN_NOT_EXISTS                            = SWC_ERRNO_APP_BEGIN + 42,
  COLUMN_REACHED_ID_LIMIT                      = SWC_ERRNO_APP_BEGIN + 43,
  COLUMN_MARKED_REMOVED                        = SWC_ERRNO_APP_BEGIN + 44,
  COLUMN_UNKNOWN_GET_FLAG                      = SWC_ERRNO_APP_BEGIN + 45,
  COLUMN_SCHEMA_NAME_EXISTS                    = SWC_ERRNO_APP_BEGIN + 46,
  COLUMN_SCHEMA_NAME_NOT_EXISTS                = SWC_ERRNO_APP_BEGIN + 47,
  COLUMN_SCHEMA_BAD_SAVE                       = SWC_ERRNO_APP_BEGIN + 48,
  COLUMN_SCHEMA_NAME_EMPTY                     = SWC_ERRNO_APP_BEGIN + 49,
  COLUMN_SCHEMA_NAME_NOT_CORRES                = SWC_ERRNO_APP_BEGIN + 50,
  COLUMN_SCHEMA_ID_EMPTY                       = SWC_ERRNO_APP_BEGIN + 51,
  COLUMN_SCHEMA_ID_EXISTS                      = SWC_ERRNO_APP_BEGIN + 52,
  COLUMN_SCHEMA_NOT_DIFFERENT                  = SWC_ERRNO_APP_BEGIN + 53,
  COLUMN_SCHEMA_MISSING                        = SWC_ERRNO_APP_BEGIN + 54,
  COLUMN_SCHEMA_IS_SYSTEM                      = SWC_ERRNO_APP_BEGIN + 55,
  COLUMN_CHANGE_INCOMPATIBLE                   = SWC_ERRNO_APP_BEGIN + 56,

  RANGE_NOT_FOUND                              = SWC_ERRNO_APP_BEGIN + 57,
  RANGE_CELLSTORES                             = SWC_ERRNO_APP_BEGIN + 58,
  RANGE_COMMITLOG                              = SWC_ERRNO_APP_BEGIN + 59,
  RANGE_BAD_INTERVAL                           = SWC_ERRNO_APP_BEGIN + 60,
  RANGE_BAD_CELLS_INPUT                        = SWC_ERRNO_APP_BEGIN + 61,

  SQL_PARSE_ERROR                              = SWC_ERRNO_APP_BEGIN + 62,
  SQL_BAD_LOAD_FILE_FORMAT                     = SWC_ERRNO_APP_BEGIN + 63,

  CLIENT_DATA_REMAINED                         = SWC_ERRNO_APP_BEGIN + 64,


};



const char* get_text(const int err) noexcept;

void print(std::ostream& out, int err);



}} //  namespace SWC::Error



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Error.cc"
#endif


#endif // swcdb_core_Error_h
