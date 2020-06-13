/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/** @file
 * Error codes, Exception handling, error logging.
 *
 * This file contains all error codes used in Hypertable, the Exception
 * base class and macros for logging and error handling.
 */

#ifndef swc_core_ERROR_H
#define swc_core_ERROR_H

#include "swcdb/core/Compat.h"
#include "swcdb/core/String.h"
#include "swcdb/core/Logger.h"
#include <ostream>
#include <stdexcept>

namespace SWC {

namespace Error {

// error codes staring 2048
#define CODE_START 0x800  
    
enum Code {
  UNPOSSIBLE                                   = -3,
  EXTERNAL                                     = -2,
  FAILED_EXPECTATION                           = -1,
  OK                                           = 0,

  IO_ERROR                                     = CODE_START+0x1,
  BAD_MEMORY_ALLOCATION                        = CODE_START+0x2,

  PROTOCOL_ERROR                               = CODE_START+0x3,
  REQUEST_TRUNCATED_HEADER                     = CODE_START+0x4,
  REQUEST_TRUNCATED_PAYLOAD                    = CODE_START+0x5,
  REQUEST_TIMEOUT                              = CODE_START+0x6,
  REQUEST_MALFORMED                            = CODE_START+0x7,
  NOT_IMPLEMENTED                              = CODE_START+0x8,
  VERSION_MISMATCH                             = CODE_START+0x9,
  CHECKSUM_MISMATCH                            = CODE_START+0xa,

  MNGR_NOT_ACTIVE                              = CODE_START+0x0f+0x1,
  MNGR_NOT_INITIALIZED                         = CODE_START+0x0f+0x2,
  RS_NOT_READY                                 = CODE_START+0x0f+0x3,
  RS_NOT_LOADED_RANGE                          = CODE_START+0x0f+0x4,
  RS_DELETED_RANGE                             = CODE_START+0x0f+0x5,
      
  ENCODER_ENCODE                               = CODE_START+0x1f+0x6,
  ENCODER_DECODE                               = CODE_START+0x1f+0x7,

  BLOCK_COMPRESSOR_UNSUPPORTED_TYPE            = CODE_START+0x1f+0x1,
  BLOCK_COMPRESSOR_TRUNCATED                   = CODE_START+0x1f+0x2,
  BLOCK_COMPRESSOR_BAD_HEADER                  = CODE_START+0x1f+0x3,
  BLOCK_COMPRESSOR_BAD_MAGIC                   = CODE_START+0x1f+0x4,
  BLOCK_COMPRESSOR_CHECKSUM_MISMATCH           = CODE_START+0x1f+0x5,
  BLOCK_COMPRESSOR_INIT_ERROR                  = CODE_START+0x1f+0x8,
  BLOCK_COMPRESSOR_INVALID_ARG                 = CODE_START+0x1f+0x9,
      
      

  CANCELLED                                    = CODE_START+0x2f+0x1,
  DOUBLE_UNGET                                 = CODE_START+0x2f+0x2,
  NO_RESPONSE                                  = CODE_START+0x2f+0x3,
  NOT_ALLOWED                                  = CODE_START+0x2f+0x4,
  INDUCED_FAILURE                              = CODE_START+0x2f+0x5,
  SERVER_SHUTTING_DOWN                         = CODE_START+0x2f+0x6,
  ALREADY_EXISTS                               = CODE_START+0x2f+0x7,
  CLOSED                                       = CODE_START+0x2f+0x8,
  DUPLICATE_RANGE                              = CODE_START+0x2f+0x9,
  BAD_FORMAT                                   = CODE_START+0x2f+0xa,
  INVALID_ARGUMENT                             = CODE_START+0x2f+0xb,
  INVALID_OPERATION                            = CODE_START+0x2f+0xc,
  UNSUPPORTED_OPERATION                        = CODE_START+0x2f+0xd,
  NOTHING_TO_DO                                = CODE_START+0x2f+0xf,

  INCOMPATIBLE_OPTIONS                         = CODE_START+0x3f+0x1,
  BAD_VALUE                                    = CODE_START+0x3f+0x2,
  SCHEMA_GENERATION_MISMATCH                   = CODE_START+0x3f+0x3,
  INVALID_METHOD_IDENTIFIER                    = CODE_START+0x3f+0x4,
  SERVER_NOT_READY                             = CODE_START+0x3f+0x5,

  CONFIG_BAD_ARGUMENT                          = CODE_START+0x4f+0x1,
  CONFIG_BAD_CFG_FILE                          = CODE_START+0x4f+0x2,
  CONFIG_GET_ERROR                             = CODE_START+0x4f+0x3,
  CONFIG_BAD_VALUE                             = CODE_START+0x4f+0x4,

  COLUMN_SCHEMA_NAME_EXISTS                    = CODE_START+0x5f+0x1,
  COLUMN_SCHEMA_NAME_NOT_EXISTS                = CODE_START+0x5f+0x2,
  COLUMN_UNKNOWN_GET_FLAG                      = CODE_START+0x5f+0x3,
  COLUMN_REACHED_ID_LIMIT                      = CODE_START+0x5f+0x4,
  COLUMN_SCHEMA_BAD_SAVE                       = CODE_START+0x5f+0x5,
  COLUMN_SCHEMA_NAME_EMPTY                     = CODE_START+0x5f+0x6,
  COLUMN_SCHEMA_NAME_NOT_CORRES                = CODE_START+0x5f+0x7,
  COLUMN_SCHEMA_ID_EMPTY                       = CODE_START+0x5f+0x8,
  COLUMN_SCHEMA_NOT_DIFFERENT                  = CODE_START+0x5f+0x9,
  COLUMN_SCHEMA_MISSING                        = CODE_START+0x5f+0xA,
  COLUMN_MARKED_REMOVED                        = CODE_START+0x5f+0xB,
  COLUMN_NOT_EXISTS                            = CODE_START+0x5f+0xC,
  COLUMN_NOT_READY                             = CODE_START+0x5f+0xD,
  COLUMN_CHANGE_INCOMPATIBLE                   = CODE_START+0x5f+0xE,
  COLUMN_SCHEMA_IS_SYSTEM                      = CODE_START+0x5f+0xF,

  SYNTAX_ERROR                                 = CODE_START+0x6f+0x1,
  COMMAND_PARSE_ERROR                          = CODE_START+0x6f+0x2,
  SCHEMA_PARSE_ERROR                           = CODE_START+0x6f+0x3,
  BAD_SCAN_SPEC                                = CODE_START+0x6f+0x4,
  BAD_SCHEMA                                   = CODE_START+0x6f+0x5,
  BAD_KEY                                      = CODE_START+0x6f+0xf,
      
  RANGE_NOT_FOUND                              = CODE_START+0x7f+0x1,
  RANGE_CS_BAD                                 = CODE_START+0x7f+0x2,
  RANGE_CELLSTORES                             = CODE_START+0x7f+0x3,
  RANGE_COMMITLOG                              = CODE_START+0x7f+0x4,
  RANGE_BAD_INTERVAL                           = CODE_START+0x7f+0x5,

  COMM_NOT_CONNECTED                           = 0x00010001,
  COMM_BROKEN_CONNECTION                       = 0x00010002,
  COMM_CONNECT_ERROR                           = 0x00010003,
  COMM_ALREADY_CONNECTED                       = 0x00010004,

  COMM_SEND_ERROR                              = 0x00010006,
  COMM_RECEIVE_ERROR                           = 0x00010007,
  COMM_POLL_ERROR                              = 0x00010008,
  COMM_CONFLICTING_ADDRESS                     = 0x00010009,
  COMM_SOCKET_ERROR                            = 0x0001000A,
  COMM_BIND_ERROR                              = 0x0001000B,
  COMM_LISTEN_ERROR                            = 0x0001000C,
  COMM_HEADER_CHECKSUM_MISMATCH                = 0x0001000D,
  COMM_PAYLOAD_CHECKSUM_MISMATCH               = 0x0001000E,
  COMM_BAD_HEADER                              = 0x0001000F,
  COMM_INVALID_PROXY                           = 0x00010010,


  SERIALIZATION_INPUT_OVERRUN                  = 0x00080001,
  SERIALIZATION_VERSION_MISMATCH               = 0x00080004,

  FS_BAD_FILE_HANDLE                           = 0x00020001,
  FS_IO_ERROR                                  = 0x00020002,
  FS_FILE_NOT_FOUND                            = 0x00020003,
  FS_BAD_FILENAME                              = 0x00020004,
  FS_PERMISSION_DENIED                         = 0x00020005,
  FS_INVALID_ARGUMENT                          = 0x00020006,
  FS_INVALID_CONFIG                            = 0x00020007,
  FS_EOF                                       = 0x00020008,
  FS_PATH_NOT_FOUND                            = 0x00020009,
      
  SQL_PARSE_ERROR                              = 0x00060001,
  SQL_BAD_LOAD_FILE_FORMAT                     = 0x00060002,
  SQL_BAD_COMMAND                              = 0x00060003,

  CLIENT_DATA_REMAINED                         = 0x00070001

};

/** Returns a descriptive error message
*
* @param err The error code
* @return The descriptive error message of this code
*/
const char* get_text(const int& err);

} // namespace Error


class Exception;

/** Helper class to render an exception message a la IO manipulators */
struct ExceptionMessageRenderer final {
  ExceptionMessageRenderer(const Exception& e);

  std::ostream& render(std::ostream& out) const;

  const Exception& ex;
};

/** Helper class to render an exception message a la IO manipulators
  *
  * When printing an Exception, this class also appends a separator. This
  * is used for printing chained Exceptions   
*/
struct ExceptionMessagesRenderer final  {
  ExceptionMessagesRenderer(const Exception& e, const char* sep = ": ");

  std::ostream& render(std::ostream& out) const;

  const Exception& ex;
  const char *separator;
};

/**
  * This is a generic exception class for Hypertable.  It takes an error code
  * as a constructor argument and translates it into an error message.
  * Exceptions can be "chained".
*/
class Exception final : public std::runtime_error {
  /** Do not allow assignments */
  const Exception& operator=(const Exception& );

  /** The error code */
  int m_error;

  /** The source code line where the exception was thrown */
  int m_line;

  /** The function name where the exception was thrown */
  const char *m_func;

  /** The source code file where the exception was thrown */
  const char *m_file;

  public:
  typedef std::runtime_error Parent;

  /** Constructor
    * @param error The error code
    * @param l The source code line
    * @param fn The function name
    * @param fl The file name
  */
  Exception(int error, int l = 0, const char *fn = 0, const char *fl = 0);

  /** Constructor
    * @param error The error code
    * @param msg An additional error message
    * @param l The source code line
    * @param fn The function name
    * @param fl The file name
    */
  Exception(int error, const std::string& msg, int l = 0, const char *fn = 0,
            const char *fl = 0);

  /** Constructor
    * @param error The error code
    * @param msg An additional error message
    * @param ex The previous exception in the exception chain
    * @param l The source code line
    * @param fn The function name
    * @param fl The file name
  */
  Exception(int error, const std::string& msg, const Exception& ex, int l = 0,
            const char *fn = 0, const char *fl = 0);

  /** Copy constructor
    * @param ex The exception that is copied
    */
  Exception(const Exception& ex);

  /** Destructor */
  ~Exception();

  /** Returns the error code
    * @return The error code of this exception.
    * @sa Error::get_text to retrieve a descriptive error string
    */
  int code() const;

  /** Returns the source code line number where the exception was thrown
    * @return The line number
    */
  int line() const;

  /** Returns the name of the function which threw the Exception
    * @return The function name
    */
  const char *func() const;

  /** Returns the source code line number where the exception was thrown
    * @return The file name
    */
  const char *file() const;

  /** Renders an Exception to an ostream
    *
    * @param out Reference to the ostream
    */
  virtual std::ostream& render_message(std::ostream& out) const;

  // render messages for the entire exception chain
  /** Renders multiple Exceptions to an ostream
    * @param out Reference to the ostream
    * @param sep The separator between the Exceptions, i.e. ':'
    */
  virtual std::ostream& render_messages(std::ostream& out,
                                        const char *sep) const;

  /** Retrieves a Renderer for this Exception */
  ExceptionMessageRenderer message() const;

  /** Retrieves a Renderer for chained Exceptions */
  ExceptionMessagesRenderer messages(const char *sep = ": ") const;

  /** The previous exception in the exception chain */
  Exception *prev;
};



/** Global operator to print an Exception to a std::ostream */
std::ostream& 
operator<<(std::ostream& out, const Exception&);

/** Global helper operator to print an Exception to a std::ostream */
std::ostream& 
operator<<(std::ostream& out, const ExceptionMessageRenderer& r);

/** Global helper operator to print an Exception to a std::ostream */
std::ostream& 
operator<<(std::ostream& out, const ExceptionMessagesRenderer& r);

}


/* Convenience macro to create an exception stack trace */
#define HT_EXCEPTION(_code_, _msg_) \
  ::SWC::Exception(_code_, _msg_, __LINE__, __PRETTY_FUNCTION__, __FILE__)

/* Convenience macro to create an chained exception */
#define HT_EXCEPTION2(_code_, _ex_, _msg_) \
  ::SWC::Exception(_code_, _msg_, _ex_, __LINE__, __PRETTY_FUNCTION__, __FILE__)

/* Convenience macro to throw an exception */
#define SWC_THROW(_code_, _msg_) throw HT_EXCEPTION(_code_, _msg_)

/* Convenience macro to throw an exception */
#define HT_THROW_(_code_) SWC_THROW(_code_, "")

/* Convenience macro to throw a chained exception */
#define HT_THROW2(_code_, _ex_, _msg_) throw HT_EXCEPTION2(_code_, _ex_, _msg_)

/* Convenience macro to throw a chained exception */
#define HT_THROW2_(_code_, _ex_) HT_THROW2(_code_, _ex_, "")

/* Convenience macro to throw an exception with a printf-like message */
#define SWC_THROWF(_code_, _fmt_, ...) \
  throw HT_EXCEPTION(_code_, ::SWC::format(_fmt_, __VA_ARGS__))

/* Convenience macro to throw a chained exception with a printf-like message */
#define HT_THROW2F(_code_, _ex_, _fmt_, ...) \
  throw HT_EXCEPTION2(_code_, _ex_, ::SWC::format(_fmt_, __VA_ARGS__))

/* Convenience macro to catch and rethrow exceptions with a printf-like
 * message */
#define HT_RETHROWF(_fmt_, ...) \
  catch (::SWC::Exception& e) { HT_THROW2F(e.code(), e, _fmt_, __VA_ARGS__); } \
  catch (std::bad_alloc& e) { \
    SWC_THROWF(::SWC::Error::BAD_MEMORY_ALLOCATION, _fmt_, __VA_ARGS__); \
  } \
  catch (std::exception& e) { \
    SWC_THROWF(::SWC::Error::EXTERNAL, "caught std::exception: %s " _fmt_,  e.what(), \
              __VA_ARGS__); \
  } \
  catch (...) { \
    SWC_LOGF(::SWC::LOG_ERROR, "caught unknown exception " _fmt_, __VA_ARGS__); \
    throw; \
  }

/* Convenience macro to catch and rethrow exceptions */
#define HT_RETHROW(_s_) HT_RETHROWF("%s", _s_)

/* Convenience macro to execute a code block and rethrow all exceptions */
#define HT_TRY(_s_, _code_) do { \
  try { _code_; } \
  HT_RETHROW(_s_) \
} while (0)

/* Convenience macros for catching and logging exceptions in destructors */
#define SWC_LOG_EXCEPTION(_s_) \
  catch (::SWC::Exception& e) { SWC_LOG_OUT(::SWC::LOG_ERROR) << e <<", "<< _s_ << SWC_LOG_OUT_END; } \
  catch (std::bad_alloc& e) { \
    SWC_LOG_OUT(::SWC::LOG_ERROR) << "Out of memory, " << _s_ << SWC_LOG_OUT_END; } \
  catch (std::exception& e) { \
    SWC_LOG_OUT(::SWC::LOG_ERROR) << "Caught exception: " << e.what() <<", "<< _s_ << SWC_LOG_OUT_END; } \
  catch (...) { \
    SWC_LOG_OUT(::SWC::LOG_ERROR) << "Caught unknown exception, " << _s_ << SWC_LOG_OUT_END; }

/* Convenience macro to execute code and log all exceptions */
#define SWC_TRY_OR_LOG(_s_, _code_) do { \
  try { _code_; } \
  SWC_LOG_EXCEPTION(_s_) \
} while (0)


// Probably should be in its own file, but...
#define SWC_EXPECT(_e_, _code_) do { if (_e_); else { \
    if (_code_ == ::SWC::Error::FAILED_EXPECTATION) \
      SWC_LOG_FATAL("failed expectation: " #_e_); \
    SWC_THROW(_code_, "failed expectation: " #_e_); } \
} while (0)

// A short cut for SWC_EXPECT(expr, ::SWC::Error::FAILED_EXPECTATION)
// unlike assert, it cannot be turned off
#define SWC_ASSERT(_e_) SWC_EXPECT(_e_, ::SWC::Error::FAILED_EXPECTATION)



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Error.cc"
#endif 

#endif
