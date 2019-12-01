/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_LOGGER_H
#define swc_core_LOGGER_H

#include "swcdb/core/Compat.h"

#include <mutex>
#include <atomic>
#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <stdio.h>



namespace SWC { 
  
enum LogPriority {
  LOG_EMERG  = 0,
  LOG_FATAL  = 0,
  LOG_ALERT  = 1,
  LOG_CRIT   = 2,
  LOG_ERROR  = 3,
  LOG_WARN   = 4,
  LOG_NOTICE = 5,
  LOG_INFO   = 6,
  LOG_DEBUG  = 7,
  LOG_NOTSET = 8
};

namespace Logger {


class LogWriter {
  public:
  
  static const constexpr char *name[] = {
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

  static const std::string repr(uint8_t priority) {
    return priority < LOG_NOTSET ? 
            name[priority] 
          : "undefined logging level: " +std::to_string(priority);
  }

  static uint8_t from_string(const std::string& loglevel) {
    if(loglevel == "info")
      return LOG_INFO;
    if(loglevel == "debug")
      return LOG_DEBUG;
    if(loglevel == "notice")
      return LOG_NOTICE;
    if(loglevel == "warn")
      return LOG_WARN;
    if(loglevel == "error")
      return LOG_ERROR;
    if(loglevel == "crit")
      return LOG_CRIT;
    if(loglevel == "alert")
      return LOG_ALERT;
    if(loglevel == "fatal")
      return LOG_FATAL;
    return -1;
  }

  std::mutex    mutex;

  LogWriter(const std::string& name = "", const std::string& logs_path = "") 
            : m_name(name), m_logs_path(logs_path), 
              m_file_out(stdout), m_file_err(stderr), 
              m_priority(LOG_INFO), m_show_line_numbers(true), 
              m_daemon(false), m_last_time(0) {
    //std::cout << " LogWriter()=" << (size_t)this << "\n";
  }
  
  void initialize(const std::string& name) {
    //std::cout << " LogWriter::initialize name=" << name 
    //          << " ptr=" << (size_t)this << "\n";

    std::lock_guard<std::mutex> lock(mutex);
    m_name.clear();
    m_name.append(name);
  }

  void daemon(const std::string& logs_path) {
    //std::cout << " LogWriter::daemon logs_path=" << logs_path 
    //          << " ptr=" << (size_t)this << "\n";
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

  void set_level(uint8_t level) {
    m_priority = level;
  }

  const uint8_t get_level() const {
    return m_priority;
  }

  const bool is_enabled(uint8_t level) const {
    return level <= m_priority;
  }

  const bool show_line_numbers() const {
    return m_show_line_numbers;
  }

  const uint32_t seconds() {
    auto t = ::time(0);
    if(m_daemon && m_last_time < t-86400)
      renew_files();
    return (uint32_t)(t-86400*(t/86400)); 
    // seconds since start of a day
  }

  template<typename T>
  void log(uint8_t priority, const T& msg) {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << seconds() << ' ' << name[priority] 
              << ": " << msg << std::endl;
  }

  template<typename T>
  void log(uint8_t priority, const char* filen, int fline, const T& msg) {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << seconds() << ' ' << name[priority] 
              << ": (" << filen << ':' << fline << ") "
              << msg << std::endl;
  }

  template<typename... Args> 
  void log(uint8_t priority, const char *format, 
           Args... args) {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << seconds() << ' ' << name[priority] << ": ";
    std::printf(format, args...);
    std::cout << std::endl;
  }

  template<typename... Args> 
  void log(uint8_t priority, const char* filen, int fline, const char *format,
           Args... args) {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << seconds() << ' ' << name[priority]
              << ": (" << filen << ':' << fline << ") ";
    std::printf(format, args...);
    std::cout << std::endl;
  }

  template<typename T>
  void debug(const T& msg) {
    log(LOG_DEBUG, msg);
  }

  template<typename... Args> 
  void debug(const char *format, Args... args) {
    log(LOG_DEBUG, format, args...);
  }

  void flush() {
    std::lock_guard<std::mutex> lock(mutex);
    _flush();
  }

  private:

  void renew_files() {
    errno = 0;
    m_last_time = (::time(0)/86400)*86400;
    auto ltm = localtime(&m_last_time);
    
    std::string filepath(m_logs_path);
    ::mkdir(filepath.c_str(), 0755);
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
      std::cout << "Changing Standard Output File to=" << filepath_out << "\n";
      m_file_out = std::freopen(filepath_out.c_str(), "w", m_file_out);

      std::cerr << "Changing Error Output File to=" << filepath_err << "\n";
      m_file_err = std::freopen(filepath_err.c_str(), "w", m_file_err);
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

  std::string           m_name;
  std::string           m_logs_path;
  FILE*                 m_file_out;
  FILE*                 m_file_err;
  std::atomic<uint8_t>  m_priority;
  bool                  m_show_line_numbers;
  bool                  m_daemon;
  time_t                m_last_time;
};


extern LogWriter logger;

} // namespace Logger




#ifndef SWC_DISABLE_LOG_ALL

#define SWC_LOG(priority, msg) \
  if(Logger::logger.is_enabled(priority)) { \
    if(Logger::logger.show_line_numbers()) \
      Logger::logger.log(priority, __FILE__, __LINE__, msg); \
    else \
      Logger::logger.log(priority, msg); \
  }

#define SWC_LOGF(priority, fmt, ...) \
  if(Logger::logger.is_enabled(priority)) { \
    if(Logger::logger.show_line_numbers()) \
      Logger::logger.log(priority, __FILE__, __LINE__, fmt, __VA_ARGS__); \
    else \
      Logger::logger.log(priority, fmt, __VA_ARGS__); \
  }


#ifndef SWC_DISABLE_LOG_FATAL ////
#define SWC_LOG_FATAL(msg) do { \
  SWC_LOG(LOG_FATAL, msg); \
  HT_ABORT; \
} while (0)
#define SWC_LOG_FATALF(msg, ...) do { \
  SWC_LOGF(LOG_FATAL, msg, __VA_ARGS__); \
  HT_ABORT; \
} while (0)
#else
#define SWC_LOG_FATAL(msg)
#define SWC_LOG_FATALF(msg, ...)
#endif



// stream interface, SWC_LOG_OUT(LOG_ERROR) << "msg" << SWC_LOG_OUT_END;

#define SWC_LOG_OUT(priority) \
  if(Logger::logger.is_enabled(priority)) { \
    uint8_t _priority_ = priority; \
    std::lock_guard<std::mutex> lock(Logger::logger.mutex); \
    if(Logger::logger.show_line_numbers()) \
      std::cout << Logger::logger.seconds() \
                << ' ' << Logger::logger.name[priority]  \
                << ": (" << __FILE__ << ':' << __LINE__ << ") "; \
    else \
      std::cout << Logger::logger.seconds() \
                << ' ' << Logger::logger.name[priority] << ": "; \
  std::cout 
#define SWC_LOG_OUT_END ""; if(_priority_ == LOG_FATAL) HT_ABORT; }

//

/*
#define HT_LOG_ENTER \
  if(Logger::logger.is_enabled(LOG_DEBUG)) {\
    if(Logger::logger.show_line_numbers()) \
      Logger::logger.debug("(%s:%d) %s() ENTER", __FILE__, __LINE__, HT_FUNC);\
    else \
      Logger::logger.debug("%s() ENTER", HT_FUNC); \
  }

#define HT_LOG_EXIT \
  if(Logger::logger.is_enabled(LOG_DEBUG)) { \
    if(Logger::logger.show_line_numbers()) \
      Logger::logger.debug("(%s:%d) %s() EXIT", __FILE__, __LINE__, HT_FUNC); \
    else \
      Logger::logger.debug("%s() EXIT", HT_FUNC); \
  }
*/

#else // SWC_DISABLE_LOGGING

#define SWC_LOG(priority, msg)
#define SWC_LOGF(priority, fmt, ...)

#define SWC_LOG_OUT(priority)

#define SWC_LOG_FATAL(msg)
#define SWC_LOG_FATAL(msg, ...)

//#define HT_LOG_ENTER
//#define HT_LOG_EXIT

#endif // SWC_DISABLE_LOGGING





/*
// helpers for printing a char pointer field
#define HT_DUMP_CSTR(_os_, _label_, _str_) \
  if (!_str_) _os_ <<" " #_label_ "=[NULL]"; \
  else _os_ <<" " #_label_ "='"<< (_str_) << "'";

#define HT_DUMP_CSTR_FIELD(_os_, _obj_, _field_) \
  HT_DUMP_CSTR(_os_, _field_, _obj_._field_)
*/


} // namespace SWC


#ifdef SWC_IMPL_SOURCE
#include "../../lib/swcdb/Logger.cc"
#endif 

#endif