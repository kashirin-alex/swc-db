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



#define SWC_ERRNO_SYS_BEGIN     0
#define SWC_ERRNO_SYS_END       2048  

#define SWC_ERRNO_FUTURE_BEGIN  2049
#define SWC_ERRNO_FUTURE_END    2059

#define SWC_ERRNO_EAI_BEGIN     2060
#define SWC_ERRNO_EAI_END       2080

#define SWC_ERRNO_APP_BEGIN     3000
#define SWC_ERRNO_APP_END       3100  



namespace SWC {

namespace Error {

enum Code {
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
      
  MNGR_NOT_ACTIVE                              = SWC_ERRNO_APP_BEGIN + 31,
  MNGR_NOT_INITIALIZED                         = SWC_ERRNO_APP_BEGIN + 32,

  RGR_NOT_READY                                = SWC_ERRNO_APP_BEGIN + 33,
  RGR_NOT_LOADED_RANGE                         = SWC_ERRNO_APP_BEGIN + 34,
  RGR_DELETED_RANGE                            = SWC_ERRNO_APP_BEGIN + 35,

  FS_BAD_FILE_HANDLE                           = SWC_ERRNO_APP_BEGIN + 36,
  FS_PERMISSION_DENIED                         = SWC_ERRNO_APP_BEGIN + 37,
  FS_EOF                                       = SWC_ERRNO_APP_BEGIN + 38,
  FS_PATH_NOT_FOUND                            = SWC_ERRNO_APP_BEGIN + 39,

  COLUMN_NOT_READY                             = SWC_ERRNO_APP_BEGIN + 40,
  COLUMN_NOT_EXISTS                            = SWC_ERRNO_APP_BEGIN + 41,
  COLUMN_REACHED_ID_LIMIT                      = SWC_ERRNO_APP_BEGIN + 42,
  COLUMN_MARKED_REMOVED                        = SWC_ERRNO_APP_BEGIN + 43,
  COLUMN_UNKNOWN_GET_FLAG                      = SWC_ERRNO_APP_BEGIN + 44,
  COLUMN_SCHEMA_NAME_EXISTS                    = SWC_ERRNO_APP_BEGIN + 45,
  COLUMN_SCHEMA_NAME_NOT_EXISTS                = SWC_ERRNO_APP_BEGIN + 46,
  COLUMN_SCHEMA_BAD_SAVE                       = SWC_ERRNO_APP_BEGIN + 47,
  COLUMN_SCHEMA_NAME_EMPTY                     = SWC_ERRNO_APP_BEGIN + 48,
  COLUMN_SCHEMA_NAME_NOT_CORRES                = SWC_ERRNO_APP_BEGIN + 49,
  COLUMN_SCHEMA_ID_EMPTY                       = SWC_ERRNO_APP_BEGIN + 50,
  COLUMN_SCHEMA_NOT_DIFFERENT                  = SWC_ERRNO_APP_BEGIN + 51,
  COLUMN_SCHEMA_MISSING                        = SWC_ERRNO_APP_BEGIN + 52,
  COLUMN_SCHEMA_IS_SYSTEM                      = SWC_ERRNO_APP_BEGIN + 53,
  COLUMN_CHANGE_INCOMPATIBLE                   = SWC_ERRNO_APP_BEGIN + 54,

  RANGE_NOT_FOUND                              = SWC_ERRNO_APP_BEGIN + 55,
  RANGE_CELLSTORES                             = SWC_ERRNO_APP_BEGIN + 56,
  RANGE_COMMITLOG                              = SWC_ERRNO_APP_BEGIN + 57,
  RANGE_BAD_INTERVAL                           = SWC_ERRNO_APP_BEGIN + 58,

  SQL_PARSE_ERROR                              = SWC_ERRNO_APP_BEGIN + 59,
  SQL_BAD_LOAD_FILE_FORMAT                     = SWC_ERRNO_APP_BEGIN + 60,

  CLIENT_DATA_REMAINED                         = SWC_ERRNO_APP_BEGIN + 61,

};

const char* get_text(const int err);

void print(std::ostream& out, int err);

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

  void print(std::ostream& out) const;

  void print_base(std::ostream& out) const;

  friend std::ostream& operator<<(std::ostream& out, const SWC::Exception& e) {
    e.print(out);
    return out;
  }

  private:

  const int           _code;
  const std::string   _msg;
  const int           _line; 
  const char*         _func; 
  const char*         _file;
  const std::string   _inner_msg;
  mutable const  Exception*  _prev;

};


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
  SWC_LOG_OUT(::SWC::LOG_ERROR, \
    SWC_LOG_OSTREAM << SWC_CURRENT_EXCEPTION(_s_); );


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
