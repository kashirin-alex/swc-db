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
 * Logging routines and macros.
 * The LogWriter provides facilities to write debug, log, error- and other
 * messages to stdout. The Logging namespaces provides std::ostream-
 * and printf-like macros and convenience functions.
 */

#ifndef swc_core_LOGGER_H
#define swc_core_LOGGER_H

#include <cstdio>
#include <iostream>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <mutex>
#include <atomic>
#include <sys/stat.h>

#include "Compat.h"
#include "String.h"
#include "FixedStream.h"

/** Logging framework. */
namespace SWC { namespace Logger {

/** Output priorities modelled after syslog */
namespace Priority {
  enum {
    EMERG  = 0,
    FATAL  = 0,
    ALERT  = 1,
    CRIT   = 2,
    ERROR  = 3,
    WARN   = 4,
    NOTICE = 5,
    INFO   = 6,
    DEBUG  = 7,
    NOTSET = 8
  };
  
  static const char *name[] = {
    "FATAL",
    "ALERT",
    "CRIT",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG",
    "NOTSET"
  };
} // namespace Priority


/** Property Extended Enum Cfg calls */
namespace cfg {

inline int from_string(const std::string& loglevel) {
  if (loglevel == "info")
    return Logger::Priority::INFO;
  if (loglevel == "debug")
    return Logger::Priority::DEBUG;
  if (loglevel == "notice")
    return Logger::Priority::NOTICE;
  if (loglevel == "warn")
    return Logger::Priority::WARN;
  if (loglevel == "error")
    return Logger::Priority::ERROR;
  if (loglevel == "crit")
    return Logger::Priority::CRIT;
  if (loglevel == "alert")
    return Logger::Priority::ALERT;
  if (loglevel == "fatal")
    return Logger::Priority::FATAL;
  return -1;
}

inline const std::string repr(int value) {
  switch(value){
    case Logger::Priority::INFO:
      return "info";
    case Logger::Priority::DEBUG:
      return "debug";
    case Logger::Priority::NOTICE:
      return "notice";
    case Logger::Priority::WARN:
      return "warn";
    case Logger::Priority::ERROR:
      return "error";
    case Logger::Priority::CRIT:
      return "crit";
    case Logger::Priority::ALERT:
      return "alert";
    case Logger::Priority::FATAL:
      return "fatal";
    default:
      return format("undefined logging level: %d", value);
  }
}

} // namespace cfg


  /** The LogWriter class writes to stdout. It's not used directly, but
   * rather through the macros below (i.e. HT_ERROR_OUT, HT_ERRORF etc).
   */
class LogWriter {
  public:
  
  /** Constructor
  *
  * @param name The name of the application
  */
  LogWriter(const std::string& name = "", const std::string& logs_path = "") 
            : m_name(name), m_logs_path(logs_path), 
              m_file_out(stdout), m_file_err(stderr), 
              m_priority(Priority::INFO), m_show_line_numbers(true), 
              m_daemon(true) {
    std::cout << " LogWriter()=" << (size_t)this << "\n";
  }
  
  void initialize(const std::string& name) {
    std::cout << " LogWriter::initialize name=" << name << " ptr=" << (size_t)this << "\n";

    std::lock_guard<std::mutex> lock(mutex);
    m_name.clear();
    m_name.append(name);
  }

  void use_file(const std::string& logs_path) {
    std::cout << " LogWriter::use_file logs_path=" << logs_path << " ptr=" << (size_t)this << "\n";
    errno = 0;

    std::lock_guard<std::mutex> lock(mutex);
    m_logs_path = logs_path;
    if(m_logs_path.back() != '/')
      m_logs_path.append("/");
    m_daemon = true;

    renew_files();

    if(errno) 
      throw std::runtime_error(
        "SWC::Logger::initialize err="
        + std::to_string(errno)+"("+strerror(errno)+")"
      );
  }

  /** Sets the message level; all messages with a higher level are discarded
  */
  void set_level(int level) {
    m_priority = level;
  }

  /** Returns the message level */
  const int get_level() const {
    return m_priority;
  }

  /** Returns true if a message with this level is not discarded */
  const bool is_enabled(int level) const {
    return level <= m_priority;
  }

  /** Returns true if line numbers are printed */ 
  bool show_line_numbers() const {
    return m_show_line_numbers;
  }

  /** Prints a debug message with variable arguments (similar to printf) */
  void debug(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    log_varargs(Priority::DEBUG, format, ap);
    va_end(ap);
  }

  /** Prints a debug message */
  void debug(const std::string &message) {
    log(Priority::DEBUG, message);
  }

  /** Prints a message with variable arguments */
  void log(int priority, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    log_varargs(priority, format, ap);
    va_end(ap);
  }

  /** Prints a message */
  void log(int priority, const std::string &message) {
    log_string(priority, message.c_str());
  }

  /** Flushes the log file */
  void flush() {
    std::lock_guard<std::mutex> lock(mutex);
    _flush();
  }

  private:

  void renew_files() {
    errno = 0;
    m_last_time = (::time(0)/86400)*86400;
    auto ltm = localtime(&m_last_time);
    
    ::mkdir(m_logs_path.c_str(), 0755);

    std::string filepath(m_logs_path);
    filepath.append(std::to_string(1900+ltm->tm_year));
    ::mkdir(filepath.c_str(), 0755);
    filepath.append("/");
    filepath.append(std::to_string(1+ltm->tm_mon));
    ::mkdir(filepath.c_str(), 0755);
    filepath.append("/");
    filepath.append(std::to_string(ltm->tm_mday));
    ::mkdir(filepath.c_str(), 0755);
    if(errno == EEXIST)
      errno = 0;

    filepath.append("/");
    filepath.append(m_name);

    std::string filepath_out(filepath+".log");
    std::string filepath_err(filepath+".err");

    if(!errno) {
      m_file_out = std::freopen(filepath_out.c_str(), "w", m_file_out);
      m_file_err = std::freopen(filepath_err.c_str(), "w", m_file_err);
      std::cout << "Changed Standard Output File to=" << filepath_out << "\n";
      std::cerr << "Changed Error Output File to=" << filepath_err << "\n";
    }

    /* else { fallback
      m_file_out = std::freopen('0', "w", m_file_out);
      m_file_err = std::freopen('1', "w", m_file_err);
      ::fdopen(0, "wt");
    }*/

  }

  void _flush() {
    ::fflush(m_file_out);
    ::fflush(m_file_err);
  }

  /** Appends a string message to the log */
  void log_string(int priority, const char *message) {
    std::lock_guard<std::mutex> lock(mutex);

    auto t = ::time(0);
    if(m_daemon && m_last_time < t-86400)
      renew_files();
    
    std::cout << (uint32_t)(t/86400) 
              << ' ' << Priority::name[priority] << ':'
              << ' ' << message 
              << std::endl;

    //fprintf(m_file_out, 
    //         "%u %s: %s\n", 
    //         (uint32_t)(t/86400), Priority::name[priority], message);
    //_flush();
  }

  /** Appends a string message with variable arguments to the log */
  void log_varargs(int priority, const char *format, va_list ap) {
    char buffer[1024 * 16];
    vsnprintf(buffer, sizeof(buffer), format, ap);
    log_string(priority, buffer);
  }

  std::mutex mutex;
  /** The name of the application */
  std::string m_name;

  /** The path for logs of the application */
  std::string m_logs_path;

  /** The output file handle */
  FILE* m_file_out;

  /** The err output file handle */
  FILE* m_file_err;

  /** The current priority (everything above is filtered) */
  std::atomic<int> m_priority;

  /** True if line numbers are shown */
  bool m_show_line_numbers;

  /** True if this log is in test mode */
  bool m_daemon;

  
  time_t m_last_time;
};


extern LogWriter logger;

}} // namespace SWC::Logger





#define HT_LOG_BUFSZ 1024

/* The HT_ABORT macro terminates the application and generates a core dump */
#ifdef HT_USE_ABORT
#  define HT_ABORT abort()
#else
#define HT_ABORT raise(SIGABRT)
#endif

// printf interface macro helper; do not use directly
#define HT_LOG(priority, msg) do { \
  if (Logger::logger.is_enabled(priority)) { \
    if (Logger::logger.show_line_numbers()) \
      Logger::logger.log(priority, SWC::format( \
          "(%s:%d) %s", __FILE__, __LINE__, msg)); \
    else \
      Logger::logger.log(priority, msg); \
  } \
} while (0)

#define HT_LOGF(priority, fmt, ...) do { \
  if (Logger::logger.is_enabled(priority)) { \
    if (Logger::logger.show_line_numbers()) \
      Logger::logger.log(priority, SWC::format( \
          "(%s:%d) " fmt, __FILE__, __LINE__, __VA_ARGS__)); \
    else \
      Logger::logger.log(priority, SWC::format( \
          fmt, __VA_ARGS__));  \
  } \
} while (0)
// ,%s __func__

// stream interface macro helpers
#define HT_LOG_BUF_SIZE 4096

#define HT_OUT(priority) do { if (Logger::logger.is_enabled(priority)) { \
  char logbuf[HT_LOG_BUF_SIZE]; \
  int _priority_ = Logger::logger.get_level(); \
  FixedOstream _out_(logbuf, sizeof(logbuf)); \
  if (Logger::logger.show_line_numbers()) \
    _out_ <<"("<< __FILE__ <<':'<< __LINE__ <<") "; \
  _out_

#define HT_OUT2(priority) do { if (Logger::logger.is_enabled(priority)) { \
  char logbuf[HT_LOG_BUF_SIZE]; \
  int _priority_ = priority; \
  FixedOstream _out_(logbuf, sizeof(logbuf)); \
  _out_ << __func__; \
  if (Logger::logger.show_line_numbers()) \
    _out_ << " ("<< __FILE__ <<':'<< __LINE__ <<")"; \
  _out_ <<": "

#define HT_END ""; Logger::logger.log(_priority_, _out_.str()); \
  if (_priority_ == Logger::Priority::FATAL) HT_ABORT; \
} /* if enabled */ } while (0)

#define HT_OUT_DISABLED do { if (0) {

// helpers for printing a char pointer field
#define HT_DUMP_CSTR(_os_, _label_, _str_) do { \
  if (!_str_) _os_ <<" " #_label_ "=[NULL]"; \
  else _os_ <<" " #_label_ "='"<< (_str_) << "'"; \
} while (0)

#define HT_DUMP_CSTR_FIELD(_os_, _obj_, _field_) \
  HT_DUMP_CSTR(_os_, _field_, _obj_._field_)


// Logging macros interface starts here
#ifndef HT_DISABLE_LOG_ALL

#ifndef HT_DISABLE_LOG_DEBUG

#define HT_LOG_ENTER do { \
  if (Logger::logger.is_enabled(Logger::Priority::DEBUG)) {\
    if (Logger::logger.show_line_numbers()) \
      Logger::logger.debug("(%s:%d) %s() ENTER", __FILE__, __LINE__, HT_FUNC);\
    else \
      Logger::logger.debug("%s() ENTER", HT_FUNC); \
  } \
} while(0)

#define HT_LOG_EXIT do { \
  if (Logger::logger.is_enabled(Logger::Priority::DEBUG)) { \
    if (Logger::logger.show_line_numbers()) \
      Logger::logger.debug("(%s:%d) %s() EXIT", __FILE__, __LINE__, HT_FUNC); \
    else \
      Logger::logger.debug("%s() EXIT", HT_FUNC); \
  } \
} while(0)

#define HT_DEBUG(msg) HT_LOG(Logger::Priority::DEBUG, msg)
#define HT_DEBUGF(msg, ...) HT_LOGF(Logger::Priority::DEBUG, msg, __VA_ARGS__)
#define HT_DEBUG_OUT HT_OUT2(Logger::Priority::DEBUG)
#else
#define HT_LOG_ENTER
#define HT_LOG_EXIT
#define HT_DEBUG(msg)
#define HT_DEBUGF(msg, ...)
#define HT_DEBUG_OUT HT_OUT_DISABLED
#endif

#ifndef HT_DISABLE_LOG_INFO
#define HT_INFO(msg) HT_LOG(Logger::Priority::INFO, msg)
#define HT_INFOF(msg, ...) HT_LOGF(Logger::Priority::INFO, msg, __VA_ARGS__)
#else
#define HT_INFO(msg)
#define HT_INFOF(msg, ...)
#endif

#ifndef HT_DISABLE_LOG_NOTICE
#define HT_NOTICE(msg) HT_LOG(Logger::Priority::NOTICE, msg)
#define HT_NOTICEF(msg, ...) HT_LOGF(Logger::Priority::NOTICE, msg, __VA_ARGS__)
#define HT_NOTICE_OUT HT_OUT(Logger::Priority::NOTICE)
#else
#define HT_NOTICE(msg)
#define HT_NOTICEF(msg, ...)
#define HT_NOTICE_OUT HT_OUT_DISABLED
#endif

#ifndef HT_DISABLE_LOG_WARN
#define HT_WARN(msg) HT_LOG(Logger::Priority::WARN, msg)
#define HT_WARNF(msg, ...) HT_LOGF(Logger::Priority::WARN, msg, __VA_ARGS__)
#define HT_WARN_OUT HT_OUT2(Logger::Priority::WARN)
#else
#define HT_WARN(msg)
#define HT_WARNF(msg, ...)
#define HT_WARN_OUT HT_OUT_DISABLED
#endif

#ifndef HT_DISABLE_LOG_ERROR
#define HT_ERROR(msg) HT_LOG(Logger::Priority::ERROR, msg)
#define HT_ERRORF(msg, ...) HT_LOGF(Logger::Priority::ERROR, msg, __VA_ARGS__)
#define HT_ERROR_OUT HT_OUT2(Logger::Priority::ERROR)
#else
#define HT_ERROR(msg)
#define HT_ERRORF(msg, ...)
#define HT_ERROR_OUT HT_OUT_DISABLED
#endif

#ifndef HT_DISABLE_LOG_CRIT
#define HT_CRIT(msg) HT_LOG(Logger::Priority::CRIT, msg)
#define HT_CRITF(msg, ...) HT_LOGF(Logger::Priority::CRIT, msg, __VA_ARGS__)
#define HT_CRIT_OUT HT_OUT2(Logger::Priority::CRIT)
#else
#define HT_CRIT(msg)
#define HT_CRITF(msg, ...)
#define HT_CRIT_OUT HT_OUT_DISABLED
#endif

#ifndef HT_DISABLE_LOG_ALERT
#define HT_ALERT(msg) HT_LOG(Logger::Priority::ALERT, msg)
#define HT_ALERTF(msg, ...) HT_LOGF(Logger::Priority::ALERT, msg, __VA_ARGS__)
#define HT_ALERT_OUT HT_OUT2(Logger::Priority::ALERT)
#else
#define HT_ALERT(msg)
#define HT_ALERTF(msg, ...)
#define HT_ALERT_OUT HT_OUT_DISABLED
#endif

#ifndef HT_DISABLE_LOG_EMERG
#define HT_EMERG(msg) HT_LOG(Logger::Priority::EMERG, msg)
#define HT_EMERGF(msg, ...) HT_LOGF(Logger::Priority::EMERG, msg, __VA_ARGS__)
#define HT_EMERG_OUT HT_OUT2(Logger::Priority::EMERG)
#else
#define HT_EMERG(msg)
#define HT_EMERGF(msg, ...)
#define HT_EMERG_OUT HT_OUT_DISABLED
#endif

#ifndef HT_DISABLE_LOG_FATAL
#define HT_FATAL(msg) do { \
  HT_LOG(Logger::Priority::FATAL, msg); \
  HT_ABORT; \
} while (0)
#define HT_FATALF(msg, ...) do { \
  HT_LOGF(Logger::Priority::FATAL, msg, __VA_ARGS__); \
  HT_ABORT; \
} while (0)
#define HT_FATAL_OUT HT_OUT2(Logger::Priority::FATAL)
#else
#define HT_FATAL(msg)
#define HT_FATALF(msg, ...)
#define HT_FATAL_OUT HT_OUT_DISABLED
#endif

#else // HT_DISABLE_LOGGING

#define HT_DEBUG(msg)
#define HT_DEBUGF(msg, ...)
#define HT_INFO(msg)
#define HT_INFOF(msg, ...)
#define HT_NOTICE(msg)
#define HT_NOTICEF(msg, ...)
#define HT_WARN(msg)
#define HT_WARNF(msg, ...)
#define HT_ERROR(msg)
#define HT_ERRORF(msg, ...)
#define HT_CRIT(msg)
#define HT_CRITF(msg, ...)
#define HT_ALERT(msg)
#define HT_ALERTF(msg, ...)
#define HT_EMERG(msg)
#define HT_EMERGF(msg, ...)
#define HT_FATAL(msg)
#define HT_FATALF(msg, ...)
#define HT_LOG_ENTER
#define HT_LOG_EXIT
#define HT_DEBUG_OUT HT_OUT_DISABLED
#define HT_NOTICE_OUT HT_OUT_DISABLED
#define HT_WARN_OUT HT_OUT_DISABLED
#define HT_ERROR_OUT HT_OUT_DISABLED
#define HT_CRIT_OUT HT_OUT_DISABLED
#define HT_ALERT_OUT HT_OUT_DISABLED
#define HT_EMERG_OUT HT_OUT_DISABLED
#define HT_FATAL_OUT HT_OUT_DISABLED

#endif // HT_DISABLE_LOGGING

// Probably should be in its own file, but...
#define HT_EXPECT(_e_, _code_) do { if (_e_); else { \
    if (_code_ == Error::FAILED_EXPECTATION) \
      HT_FATAL("failed expectation: " #_e_); \
    HT_THROW(_code_, "failed expectation: " #_e_); } \
} while (0)

// A short cut for HT_EXPECT(expr, Error::FAILED_EXPECTATION)
// unlike assert, it cannot be turned off
#define HT_ASSERT(_e_) HT_EXPECT(_e_, Error::FAILED_EXPECTATION)

#endif // 
