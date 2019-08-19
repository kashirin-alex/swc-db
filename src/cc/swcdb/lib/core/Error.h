/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
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

#include "String.h"
#include <ostream>
#include <stdexcept>

namespace SWC {

  /** @addtogroup Common
   *  @{
   */

  namespace Error {

 	// error codes staring 2048
  #define CODE_START 0x800  
    
    enum Code {
      UNPOSSIBLE                                   = -3,
      EXTERNAL                                     = -2,
      FAILED_EXPECTATION                           = -1,
      OK                                           = 0,

      PROTOCOL_ERROR                               = CODE_START+1,
      REQUEST_TRUNCATED                            = CODE_START+2,
      RESPONSE_TRUNCATED                           = CODE_START+3,
      REQUEST_TIMEOUT                              = CODE_START+4,


      LOCAL_IO_ERROR                               = CODE_START+5,
      BAD_SCHEMA                                   = CODE_START+7,
      BAD_KEY                                      = CODE_START+9,      

      BLOCK_COMPRESSOR_UNSUPPORTED_TYPE            = CODE_START+13,
      BLOCK_COMPRESSOR_INVALID_ARG                 = CODE_START+14,
      BLOCK_COMPRESSOR_TRUNCATED                   = CODE_START+15,
      BLOCK_COMPRESSOR_BAD_HEADER                  = CODE_START+16,
      BLOCK_COMPRESSOR_BAD_MAGIC                   = CODE_START+17,
      BLOCK_COMPRESSOR_CHECKSUM_MISMATCH           = CODE_START+18,
      BLOCK_COMPRESSOR_DEFLATE_ERROR               = CODE_START+19,
      BLOCK_COMPRESSOR_INFLATE_ERROR               = CODE_START+20,
      BLOCK_COMPRESSOR_INIT_ERROR                  = CODE_START+21,
      
      NOT_LOADED_RANGE                             = CODE_START+22,

      MALFORMED_REQUEST                            = CODE_START+23,
      TOO_MANY_COLUMNS                             = CODE_START+24,
      BAD_DOMAIN_NAME                              = CODE_START+25,
      COMMAND_PARSE_ERROR                          = CODE_START+26,
      CONNECT_ERROR_MASTER                         = CODE_START+27,
      CONNECT_ERROR_HYPERSPACE                     = CODE_START+28,
      BAD_MEMORY_ALLOCATION                        = CODE_START+29,
      BAD_SCAN_SPEC                                = CODE_START+30,
      NOT_IMPLEMENTED                              = CODE_START+31,
      VERSION_MISMATCH                             = CODE_START+32,
      CANCELLED                                    = CODE_START+33,
      SCHEMA_PARSE_ERROR                           = CODE_START+34,
      SYNTAX_ERROR                                 = CODE_START+35,
      DOUBLE_UNGET                                 = CODE_START+36,
      EMPTY_BLOOMFILTER                            = CODE_START+37,
      BLOOMFILTER_CHECKSUM_MISMATCH                = CODE_START+38,
      NAME_ALREADY_IN_USE                          = CODE_START+39,
      NAMESPACE_DOES_NOT_EXIST                     = CODE_START+40,
      BAD_NAMESPACE                                = CODE_START+41,
      NAMESPACE_EXISTS                             = CODE_START+42,
      NO_RESPONSE                                  = CODE_START+43,
      NOT_ALLOWED                                  = CODE_START+44,
      INDUCED_FAILURE                              = CODE_START+45,
      SERVER_SHUTTING_DOWN                         = CODE_START+46,
      LOCATION_UNASSIGNED                          = CODE_START+47,
      ALREADY_EXISTS                               = CODE_START+48,
      CHECKSUM_MISMATCH                            = CODE_START+49,
      CLOSED                                       = CODE_START+50,
      RANGESERVER_NOT_FOUND                        = CODE_START+51,
      CONNECTION_NOT_INITIALIZED                   = CODE_START+52,
      DUPLICATE_RANGE                              = CODE_START+53,
      INVALID_PSEUDO_TABLE_NAME                    = CODE_START+54,
      BAD_FORMAT                                   = CODE_START+55,
      INVALID_ARGUMENT                             = CODE_START+56,
      INVALID_OPERATION                            = CODE_START+57,
      UNSUPPORTED_OPERATION                        = CODE_START+58,
      COLUMN_FAMILY_NOT_FOUND                      = CODE_START+59,
      NOTHING_TO_DO                                = CODE_START+60,
      INCOMPATIBLE_OPTIONS                         = CODE_START+61,
      BAD_VALUE                                    = CODE_START+62,
      SCHEMA_GENERATION_MISMATCH                   = CODE_START+63,
      INVALID_METHOD_IDENTIFIER                    = CODE_START+64,
      SERVER_NOT_READY                             = CODE_START+65,

      CONFIG_BAD_ARGUMENT                          = CODE_START+1001,
      CONFIG_BAD_CFG_FILE                          = CODE_START+1002,
      CONFIG_GET_ERROR                             = CODE_START+1003,
      CONFIG_BAD_VALUE                             = CODE_START+1004,

      SCHEMA_COL_NAME_EXISTS                       = CODE_START+2001,

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
      SERIALIZATION_BAD_VINT                       = 0x00080002,
      SERIALIZATION_BAD_VSTR                       = 0x00080003,
      SERIALIZATION_VERSION_MISMATCH               = 0x00080004,

      FS_BAD_FILE_HANDLE                    = 0x00020001,
      FS_IO_ERROR                           = 0x00020002,
      FS_FILE_NOT_FOUND                     = 0x00020003,
      FS_BAD_FILENAME                       = 0x00020004,
      FS_PERMISSION_DENIED                  = 0x00020005,
      FS_INVALID_ARGUMENT                   = 0x00020006,
      FS_INVALID_CONFIG                     = 0x00020007,
      FS_EOF                                = 0x00020008,

      HQL_PARSE_ERROR                              = 0x00060001,
      HQL_BAD_LOAD_FILE_FORMAT                     = 0x00060002,
      HQL_BAD_COMMAND                              = 0x00060003,

      MASTER_TABLE_EXISTS                          = 0x00040001,
      MASTER_BAD_SCHEMA                            = 0x00040002,
      MASTER_NOT_RUNNING                           = 0x00040003,
      MASTER_NO_RANGESERVERS                       = 0x00040004,
      MASTER_FILE_NOT_LOCKED                       = 0x00040005,
      MASTER_RANGESERVER_ALREADY_REGISTERED        = 0x00040006,
      MASTER_BAD_COLUMN_FAMILY                     = 0x00040007,
      MASTER_SCHEMA_GENERATION_MISMATCH            = 0x00040008,
      MASTER_LOCATION_ALREADY_ASSIGNED             = 0x00040009,
      MASTER_LOCATION_INVALID                      = 0x0004000A,
      MASTER_OPERATION_IN_PROGRESS                 = 0x0004000B,
      MASTER_RANGESERVER_IN_RECOVERY               = 0x0004000C,
      MASTER_BALANCE_PREVENTED                     = 0x0004000D,

      RANGESERVER_GENERATION_MISMATCH              = 0x00050001,
      RANGESERVER_RANGE_ALREADY_LOADED             = 0x00050002,
      RANGESERVER_RANGE_MISMATCH                   = 0x00050003,
      RANGESERVER_NONEXISTENT_RANGE                = 0x00050004,
      RANGESERVER_OUT_OF_RANGE                     = 0x00050005,
      RANGESERVER_RANGE_NOT_FOUND                  = 0x00050006,
      RANGESERVER_INVALID_SCANNER_ID               = 0x00050007,
      RANGESERVER_SCHEMA_PARSE_ERROR               = 0x00050008,
      RANGESERVER_SCHEMA_INVALID_CFID              = 0x00050009,
      RANGESERVER_INVALID_COLUMNFAMILY             = 0x0005000A,
      RANGESERVER_TRUNCATED_COMMIT_LOG             = 0x0005000B,
      RANGESERVER_NO_METADATA_FOR_RANGE            = 0x0005000C,
      RANGESERVER_SHUTTING_DOWN                    = 0x0005000D,
      RANGESERVER_CORRUPT_COMMIT_LOG               = 0x0005000E,
      RANGESERVER_UNAVAILABLE                      = 0x0005000F,
      RANGESERVER_REVISION_ORDER_ERROR             = 0x00050010,
      RANGESERVER_ROW_OVERFLOW                     = 0x00050011,
      RANGESERVER_BAD_SCAN_SPEC                    = 0x00050013,
      RANGESERVER_CLOCK_SKEW                       = 0x00050014,
      RANGESERVER_BAD_CELLSTORE_FILENAME           = 0x00050015,
      RANGESERVER_CORRUPT_CELLSTORE                = 0x00050016,
      RANGESERVER_TABLE_DROPPED                    = 0x00050017,
      RANGESERVER_UNEXPECTED_TABLE_ID              = 0x00050018,
      RANGESERVER_RANGE_BUSY                       = 0x00050019,
      RANGESERVER_BAD_CELL_INTERVAL                = 0x0005001A,
      RANGESERVER_SHORT_CELLSTORE_READ             = 0x0005001B,
      RANGESERVER_RANGE_NOT_ACTIVE                 = 0x0005001C,
      RANGESERVER_FRAGMENT_ALREADY_PROCESSED       = 0x0005001D,
      RANGESERVER_RECOVERY_PLAN_GENERATION_MISMATCH = 0x0005001E,
      RANGESERVER_PHANTOM_RANGE_MAP_NOT_FOUND      = 0x0005001F,
      RANGESERVER_RANGES_ALREADY_LIVE              = 0x00050020,
      RANGESERVER_RANGE_NOT_YET_ACKNOWLEDGED       = 0x00050021,
      RANGESERVER_SERVER_IN_READONLY_MODE          = 0x00050022,
      RANGESERVER_RANGE_NOT_YET_RELINQUISHED       = 0x00050023,
    

      THRIFTBROKER_BAD_SCANNER_ID                  = 0x00090001,
      THRIFTBROKER_BAD_MUTATOR_ID                  = 0x00090002,
      THRIFTBROKER_BAD_NAMESPACE_ID                = 0x00090003,
      THRIFTBROKER_BAD_FUTURE_ID                   = 0x00090004
    };

    /** Returns a descriptive error message
     *
     * @param error The error code
     * @return The descriptive error message of this code
     */
    const char *get_text(int error);

    /** Generates and print the error documentation as html
     *
     * @param out The ostream which is used for printing
     */
    void generate_html_error_code_documentation(std::ostream &out);

  } // namespace Error


  class Exception;

  /** Helper class to render an exception message a la IO manipulators */
  struct ExceptionMessageRenderer {
    ExceptionMessageRenderer(const Exception &e) : ex(e) { }

    std::ostream &render(std::ostream &out) const;

    const Exception &ex;
  };

  /** Helper class to render an exception message a la IO manipulators
   *
   * When printing an Exception, this class also appends a separator. This
   * is used for printing chained Exceptions
   */
  struct ExceptionMessagesRenderer {
    ExceptionMessagesRenderer(const Exception &e, const char *sep = ": ")
      : ex(e), separator(sep) { }

    std::ostream &render(std::ostream &out) const;

    const Exception &ex;
    const char *separator;
  };

  /**
   * This is a generic exception class for Hypertable.  It takes an error code
   * as a constructor argument and translates it into an error message.
   * Exceptions can be "chained".
   */
  class Exception : public std::runtime_error {
    /** Do not allow assignments */
    const Exception &operator=(const Exception &);

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
     *
     * @param error The error code
     * @param l The source code line
     * @param fn The function name
     * @param fl The file name
     */
    Exception(int error, int l = 0, const char *fn = 0, const char *fl = 0)
      : Parent(""), m_error(error), m_line(l), m_func(fn), m_file(fl), prev(0) {
    }

    /** Constructor
     *
     * @param error The error code
     * @param msg An additional error message
     * @param l The source code line
     * @param fn The function name
     * @param fl The file name
     */
    Exception(int error, const String &msg, int l = 0, const char *fn = 0,
            const char *fl = 0)
      : Parent(msg), m_error(error), m_line(l), m_func(fn), m_file(fl),
      prev(0) {
    }

    /** Constructor
     *
     * @param error The error code
     * @param msg An additional error message
     * @param ex The previous exception in the exception chain
     * @param l The source code line
     * @param fn The function name
     * @param fl The file name
     */
    Exception(int error, const String &msg, const Exception &ex, int l = 0,
            const char *fn = 0, const char *fl = 0)
      : Parent(msg), m_error(error), m_line(l), m_func(fn), m_file(fl),
        prev(new Exception(ex)) {
    }

    /** Copy constructor
     *
     * @param ex The exception that is copied
     */
    Exception(const Exception &ex)
      : Parent(ex), m_error(ex.m_error), m_line(ex.m_line), m_func(ex.m_func),
      m_file(ex.m_file) {
      prev = ex.prev ? new Exception(*ex.prev) : 0;
    }

    /** Destructor */
    ~Exception() throw() { delete prev; prev = 0; }

    /** Returns the error code
     *
     * @return The error code of this exception.
     * @sa Error::get_text to retrieve a descriptive error string
     */
    int code() const { return m_error; }

    /** Returns the source code line number where the exception was thrown
     *
     * @return The line number
     */
    int line() const { return m_line; }

    /** Returns the name of the function which threw the Exception
     *
     * @return The function name
     */
    const char *func() const { return m_func; }

    /** Returns the source code line number where the exception was thrown
     *
     * @return The file name
     */
    const char *file() const { return m_file; }

    /** Renders an Exception to an ostream
     *
     * @param out Reference to the ostream
     */
    virtual std::ostream &render_message(std::ostream &out) const {
      return out << what(); // override for custom exceptions
    }

    // render messages for the entire exception chain
    /** Renders multiple Exceptions to an ostream
     *
     * @param out Reference to the ostream
     * @param sep The separator between the Exceptions, i.e. ':'
     */
    virtual std::ostream &render_messages(std::ostream &out,
            const char *sep) const;

    /** Retrieves a Renderer for this Exception */
    ExceptionMessageRenderer message() const {
      return ExceptionMessageRenderer(*this);
    }

    /** Retrieves a Renderer for chained Exceptions */
    ExceptionMessagesRenderer messages(const char *sep = ": ") const {
      return ExceptionMessagesRenderer(*this, sep);
    }

    /** The previous exception in the exception chain */
    Exception *prev;
  };

  /** Global operator to print an Exception to a std::ostream */
  std::ostream &operator<<(std::ostream &out, const Exception &);

  /** Global helper function to print an Exception to a std::ostream */
  inline std::ostream &
  ExceptionMessageRenderer::render(std::ostream &out) const {
    return ex.render_message(out);
  }

  /** Global helper function to print an Exception to a std::ostream */
  inline std::ostream &
  ExceptionMessagesRenderer::render(std::ostream &out) const {
    return ex.render_messages(out, separator);
  }

  /** Global helper operator to print an Exception to a std::ostream */
  inline std::ostream &
  operator<<(std::ostream &out, const ExceptionMessageRenderer &r) {
    return r.render(out);
  }

  /** Global helper operator to print an Exception to a std::ostream */
  inline std::ostream &
  operator<<(std::ostream &out, const ExceptionMessagesRenderer &r) {
    return r.render(out);
  }

/* Convenience macro to create an exception stack trace */
#define HT_EXCEPTION(_code_, _msg_) \
  Exception(_code_, _msg_, __LINE__, HT_FUNC, __FILE__)

/* Convenience macro to create an chained exception */
#define HT_EXCEPTION2(_code_, _ex_, _msg_) \
  Exception(_code_, _msg_, _ex_, __LINE__, HT_FUNC, __FILE__)

/* Convenience macro to throw an exception */
#define HT_THROW(_code_, _msg_) throw HT_EXCEPTION(_code_, _msg_)

/* Convenience macro to throw an exception */
#define HT_THROW_(_code_) HT_THROW(_code_, "")

/* Convenience macro to throw a chained exception */
#define HT_THROW2(_code_, _ex_, _msg_) throw HT_EXCEPTION2(_code_, _ex_, _msg_)

/* Convenience macro to throw a chained exception */
#define HT_THROW2_(_code_, _ex_) HT_THROW2(_code_, _ex_, "")

/* Convenience macro to throw an exception with a printf-like message */
#define HT_THROWF(_code_, _fmt_, ...) \
  throw HT_EXCEPTION(_code_, SWC::format(_fmt_, __VA_ARGS__))

/* Convenience macro to throw a chained exception with a printf-like message */
#define HT_THROW2F(_code_, _ex_, _fmt_, ...) \
  throw HT_EXCEPTION2(_code_, _ex_, SWC::format(_fmt_, __VA_ARGS__))

/* Convenience macro to catch and rethrow exceptions with a printf-like
 * message */
#define HT_RETHROWF(_fmt_, ...) \
  catch (Exception &e) { HT_THROW2F(e.code(), e, _fmt_, __VA_ARGS__); } \
  catch (std::bad_alloc &e) { \
    HT_THROWF(Error::BAD_MEMORY_ALLOCATION, _fmt_, __VA_ARGS__); \
  } \
  catch (std::exception &e) { \
    HT_THROWF(Error::EXTERNAL, "caught std::exception: %s " _fmt_,  e.what(), \
              __VA_ARGS__); \
  } \
  catch (...) { \
    HT_ERRORF("caught unknown exception " _fmt_, __VA_ARGS__); \
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
#define HT_LOG_EXCEPTION(_s_) \
  catch (Exception &e) { HT_ERROR_OUT << e <<", "<< _s_ << HT_END; } \
  catch (std::bad_alloc &e) { \
    HT_ERROR_OUT << "Out of memory, " << _s_ << HT_END; } \
  catch (std::exception &e) { \
    HT_ERROR_OUT << "Caught exception: " << e.what() <<", "<< _s_ << HT_END; } \
  catch (...) { \
    HT_ERROR_OUT << "Caught unknown exception, " << _s_ << HT_END; }

/* Convenience macro to execute code and log all exceptions */
#define HT_TRY_OR_LOG(_s_, _code_) do { \
  try { _code_; } \
  HT_LOG_EXCEPTION(_s_) \
} while (0)

/** @} */

}

#endif
