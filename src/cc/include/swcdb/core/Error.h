/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_Error_h
#define swc_core_Error_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/String.h"
#include "swcdb/core/Logger.h"
#include <ostream>
#include <stdexcept>

namespace SWC {

namespace Error {

#define SYS_BEGIN     0
#define SYS_END       2048  

#define FUTURE_BEGIN  2049
#define FUTURE_END    2059

#define APP_BEGIN     3000
#define APP_END       INT32_MAX  

enum Code {
  EXCEPTION_BAD                                = -4,
  EXCEPTION_UNKNOWN                            = -3,
  UNPOSSIBLE                                   = -2,
  FAILED_EXPECTATION                           = -1,

  OK                                           = 0,

  IO_ERROR                                     = APP_BEGIN + 0,
  BAD_MEMORY_ALLOCATION                        = APP_BEGIN + 1,
  BAD_FUNCTION                                 = APP_BEGIN + 2,
  BAD_POINTER                                  = APP_BEGIN + 3,
  BAD_CAST                                     = APP_BEGIN + 4,
  BAD_FORMAT                                   = APP_BEGIN + 5,
  BAD_REGEXP                                   = APP_BEGIN + 6,
  BAD_LOGIC                                    = APP_BEGIN + 7,

  PROTOCOL_ERROR                               = APP_BEGIN + 10,
  REQUEST_TRUNCATED_HEADER                     = APP_BEGIN + 11,
  REQUEST_TRUNCATED_PAYLOAD                    = APP_BEGIN + 12,
  REQUEST_TIMEOUT                              = APP_BEGIN + 13,
  REQUEST_MALFORMED                            = APP_BEGIN + 14,
  NOT_IMPLEMENTED                              = APP_BEGIN + 15,
  VERSION_MISMATCH                             = APP_BEGIN + 16,
  CHECKSUM_MISMATCH                            = APP_BEGIN + 17,

  MNGR_NOT_ACTIVE                              = APP_BEGIN + 20,
  MNGR_NOT_INITIALIZED                         = APP_BEGIN + 21,
  RS_NOT_READY                                 = APP_BEGIN + 22,
  RS_NOT_LOADED_RANGE                          = APP_BEGIN + 23,
  RS_DELETED_RANGE                             = APP_BEGIN + 24,
      
  ENCODER_ENCODE                               = APP_BEGIN + 30,
  ENCODER_DECODE                               = APP_BEGIN + 31,

  BLOCK_COMPRESSOR_UNSUPPORTED_TYPE            = APP_BEGIN + 35,
  BLOCK_COMPRESSOR_TRUNCATED                   = APP_BEGIN + 36,
  BLOCK_COMPRESSOR_BAD_HEADER                  = APP_BEGIN + 37,
  BLOCK_COMPRESSOR_BAD_MAGIC                   = APP_BEGIN + 38,
  BLOCK_COMPRESSOR_CHECKSUM_MISMATCH           = APP_BEGIN + 39,
  BLOCK_COMPRESSOR_INIT_ERROR                  = APP_BEGIN + 40,
  BLOCK_COMPRESSOR_INVALID_ARG                 = APP_BEGIN + 41,
      
      

  CANCELLED                                    = APP_BEGIN + 50,
  DOUBLE_UNGET                                 = APP_BEGIN + 51,
  NO_RESPONSE                                  = APP_BEGIN + 52,
  NOT_ALLOWED                                  = APP_BEGIN + 53,
  INDUCED_FAILURE                              = APP_BEGIN + 54,
  SERVER_SHUTTING_DOWN                         = APP_BEGIN + 55,
  ALREADY_EXISTS                               = APP_BEGIN + 56,
  CLOSED                                       = APP_BEGIN + 57,
  DUPLICATE_RANGE                              = APP_BEGIN + 58,
  INVALID_ARGUMENT                             = APP_BEGIN + 60,
  INVALID_OPERATION                            = APP_BEGIN + 61,
  UNSUPPORTED_OPERATION                        = APP_BEGIN + 62,
  NOTHING_TO_DO                                = APP_BEGIN + 63,

  INCOMPATIBLE_OPTIONS                         = APP_BEGIN + 64,
  BAD_VALUE                                    = APP_BEGIN + 65,
  SCHEMA_GENERATION_MISMATCH                   = APP_BEGIN + 66,
  INVALID_METHOD_IDENTIFIER                    = APP_BEGIN + 67,
  SERVER_NOT_READY                             = APP_BEGIN + 68,

  CONFIG_BAD_ARGUMENT                          = APP_BEGIN + 69,
  CONFIG_BAD_CFG_FILE                          = APP_BEGIN + 70,
  CONFIG_GET_ERROR                             = APP_BEGIN + 71,
  CONFIG_BAD_VALUE                             = APP_BEGIN + 72,

  COLUMN_SCHEMA_NAME_EXISTS                    = APP_BEGIN + 80,
  COLUMN_SCHEMA_NAME_NOT_EXISTS                = APP_BEGIN + 81,
  COLUMN_UNKNOWN_GET_FLAG                      = APP_BEGIN + 82,
  COLUMN_REACHED_ID_LIMIT                      = APP_BEGIN + 83,
  COLUMN_SCHEMA_BAD_SAVE                       = APP_BEGIN + 84,
  COLUMN_SCHEMA_NAME_EMPTY                     = APP_BEGIN + 85,
  COLUMN_SCHEMA_NAME_NOT_CORRES                = APP_BEGIN + 86,
  COLUMN_SCHEMA_ID_EMPTY                       = APP_BEGIN + 87,
  COLUMN_SCHEMA_NOT_DIFFERENT                  = APP_BEGIN + 88,
  COLUMN_SCHEMA_MISSING                        = APP_BEGIN + 89,
  COLUMN_MARKED_REMOVED                        = APP_BEGIN + 90,
  COLUMN_NOT_EXISTS                            = APP_BEGIN + 91,
  COLUMN_NOT_READY                             = APP_BEGIN + 92,
  COLUMN_CHANGE_INCOMPATIBLE                   = APP_BEGIN + 93,
  COLUMN_SCHEMA_IS_SYSTEM                      = APP_BEGIN + 94,

  SYNTAX_ERROR                                 = APP_BEGIN + 100,
  COMMAND_PARSE_ERROR                          = APP_BEGIN + 101,
  SCHEMA_PARSE_ERROR                           = APP_BEGIN + 102,
  BAD_SCAN_SPEC                                = APP_BEGIN + 103,
  BAD_SCHEMA                                   = APP_BEGIN + 104,
  BAD_KEY                                      = APP_BEGIN + 105,
      
  RANGE_NOT_FOUND                              = APP_BEGIN + 110,
  RANGE_CS_BAD                                 = APP_BEGIN + 111,
  RANGE_CELLSTORES                             = APP_BEGIN + 112,
  RANGE_COMMITLOG                              = APP_BEGIN + 113,
  RANGE_BAD_INTERVAL                           = APP_BEGIN + 114,

  COMM_NOT_CONNECTED                           = APP_BEGIN + 120,
  COMM_BROKEN_CONNECTION                       = APP_BEGIN + 121,
  COMM_CONNECT_ERROR                           = APP_BEGIN + 122,
  COMM_ALREADY_CONNECTED                       = APP_BEGIN + 123,

  COMM_SEND_ERROR                              = APP_BEGIN + 124,
  COMM_RECEIVE_ERROR                           = APP_BEGIN + 125,
  COMM_POLL_ERROR                              = APP_BEGIN + 126,
  COMM_CONFLICTING_ADDRESS                     = APP_BEGIN + 127,
  COMM_SOCKET_ERROR                            = APP_BEGIN + 128,
  COMM_BIND_ERROR                              = APP_BEGIN + 129,
  COMM_LISTEN_ERROR                            = APP_BEGIN + 130,
  COMM_HEADER_CHECKSUM_MISMATCH                = APP_BEGIN + 131,
  COMM_PAYLOAD_CHECKSUM_MISMATCH               = APP_BEGIN + 132,
  COMM_BAD_HEADER                              = APP_BEGIN + 133,
  COMM_INVALID_PROXY                           = APP_BEGIN + 134,


  SERIALIZATION_INPUT_OVERRUN                  = APP_BEGIN + 140,
  SERIALIZATION_VERSION_MISMATCH               = APP_BEGIN + 141,

  FS_BAD_FILE_HANDLE                           = APP_BEGIN + 150,
  FS_IO_ERROR                                  = APP_BEGIN + 151,
  FS_FILE_NOT_FOUND                            = APP_BEGIN + 152,
  FS_BAD_FILENAME                              = APP_BEGIN + 153,
  FS_PERMISSION_DENIED                         = APP_BEGIN + 154,
  FS_INVALID_ARGUMENT                          = APP_BEGIN + 155,
  FS_INVALID_CONFIG                            = APP_BEGIN + 156,
  FS_EOF                                       = APP_BEGIN + 157,
  FS_PATH_NOT_FOUND                            = APP_BEGIN + 158,
      
  SQL_PARSE_ERROR                              = APP_BEGIN + 159,
  SQL_BAD_LOAD_FILE_FORMAT                     = APP_BEGIN + 160,
  SQL_BAD_COMMAND                              = APP_BEGIN + 161,

  CLIENT_DATA_REMAINED                         = APP_BEGIN + 170,

};

const char* get_text(const int& err);

}




class Exception : public std::exception {
  public:

  static const Exception make(const std::exception_ptr& eptr,
                              const std::string& msg, 
                              const Exception* prev = nullptr);

  Exception(int code, const std::string& msg,
            int line = 0, const char* func = 0, const char* file = 0,
            const std::string& inner_msg = "");

  Exception(int code, const std::string& msg, const Exception* prev,
            const std::string& inner_msg);

  Exception(int code, const std::string& msg, const Exception* prev,
            int line = 0, const char* func = 0, const char* file = 0,
            const std::string& inner_msg = "");

  Exception(int code, const std::string& msg, const Exception& prev,
            int line = 0, const char* func = 0, const char* file = 0,
            const std::string& inner_msg = "");

  Exception(const Exception& other);

  const Exception& operator=(const Exception& ) = delete;

  ~Exception();

  int code() const noexcept {
    return _code;
  }

  virtual const char* what() const noexcept override {
    return _msg.c_str();
  }

  int line() const noexcept {
    return _line;
  }

  const char* func() const noexcept {
    return _func;
  }

  const char* file() const noexcept {
    return _file;
  }

  const char* inner_what() const noexcept {
    return _inner_msg.c_str();
  }

  std::string message() const;

  void render(std::ostream& out) const;

  void render_base(std::ostream& out) const;

  private:

  const int           _code;
  const std::string   _msg;
  const int           _line; 
  const char*         _func; 
  const char*         _file;
  const std::string   _inner_msg;
  mutable const  Exception*  _prev;

};


SWC_CAN_INLINE
std::ostream& operator<<(std::ostream& out, const SWC::Exception& e) {
  e.render(out);
  return out;
}


} //  namespace SWC



// EXCEPTION HELPERS
#define SWC_EXCEPTION(_code_, _msg_) \
  ::SWC::Exception(\
    _code_, _msg_, __LINE__, __PRETTY_FUNCTION__, __FILE__)
  
#define SWC_EXCEPTION2(_code_, _ex_, _msg_) \
  ::SWC::Exception(\
    _code_, _msg_, _ex_, __LINE__, __PRETTY_FUNCTION__, __FILE__)


#define SWC_CURRENT_EXCEPTION(_msg_) \
  ::SWC::Exception::make(std::current_exception(), _msg_)


/* preferred:
try {
  --code--
} catch (...) {
  const Exception& e = SWC_CURRENT_EXCEPTION("INFO..");
  err = e.code();
}
*/


// THROW HELPERS
#define SWC_THROW(_code_, _msg_) throw SWC_EXCEPTION(_code_, _msg_)

#define SWC_THROWF(_code_, _fmt_, ...) \
  throw SWC_EXCEPTION(_code_, ::SWC::format(_fmt_, __VA_ARGS__))

#define SWC_THROW2F(_code_, _ex_, _fmt_, ...) \
  throw SWC_EXCEPTION2(_code_, _ex_, ::SWC::format(_fmt_, __VA_ARGS__))


// LOG HELPERS
#define SWC_LOG_CURRENT_EXCEPTION(_s_) \
  SWC_LOG_OUT(::SWC::LOG_ERROR) << SWC_CURRENT_EXCEPTION(_s_) \
                                << SWC_LOG_OUT_END


// TRY HELPERS
#define SWC_TRY(_s_, _code_) \
  try { _code_; } catch(...) { throw SWC_CURRENT_EXCEPTION(_s_); }

#define SWC_TRY_OR_LOG(_s_, _code_) \
  try { _code_; } catch(...) { SWC_LOG_CURRENT_EXCEPTION(_s_); }


// CONDITION HELPERS
#define SWC_EXPECT(_e_, _code_) \
  if (!(_e_)) { \
    if (_code_ == ::SWC::Error::FAILED_EXPECTATION) \
      SWC_LOG_FATAL("failed expectation: " #_e_); \
    SWC_THROW(_code_, "failed expectation: " #_e_); \
  }

#define SWC_ASSERT(_e_) SWC_EXPECT(_e_, ::SWC::Error::FAILED_EXPECTATION)




#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Error.cc"
#endif 


#endif // swc_core_Error_h
