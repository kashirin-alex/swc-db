/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_core_LOGGER_H
#define swc_core_LOGGER_H

#include "swcdb/core/Compat.h"
#include "swcdb/core/Mutex.h"

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


class LogWriter final {
  public:
  
  static std::string repr(uint8_t priority);

  static const char* get_name(uint8_t priority);

  static uint8_t from_string(const std::string& loglevel);

  Mutex    mutex;

  LogWriter(const std::string& name="", const std::string& logs_path="");
  
  ~LogWriter();
  
  void initialize(const std::string& name);

  void daemon(const std::string& logs_path);

  void set_level(uint8_t level);

  uint8_t get_level() const;

  bool is_enabled(uint8_t level) const;

  bool show_line_numbers() const;

  uint32_t seconds();

  template<typename T>
  void log(uint8_t priority, const T& msg) {
    Mutex::scope lock(mutex);
    std::cout << seconds() << ' ' << get_name(priority) 
              << ": " << msg << std::endl;
  }

  template<typename T>
  void log(uint8_t priority, const char* filen, int fline, const T& msg) {
    Mutex::scope lock(mutex);
    std::cout << seconds() << ' ' << get_name(priority) 
              << ": (" << filen << ':' << fline << ") "
              << msg << std::endl;
  }

  template<typename... Args> 
  void log(uint8_t priority, const char *format, 
           Args... args) {
    Mutex::scope lock(mutex);
    std::cout << seconds() << ' ' << get_name(priority) << ": ";
    std::printf(format, args...);
    std::cout << std::endl;
  }

  template<typename... Args> 
  void log(uint8_t priority, const char* filen, int fline, const char *format,
           Args... args) {
    Mutex::scope lock(mutex);
    std::cout << seconds() << ' ' << get_name(priority)
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

  private:

  void renew_files();

  std::string           m_name;
  std::string           m_logs_path;
  FILE*                 m_file_out;
  //FILE*               m_file_err;
  std::atomic<uint8_t>  m_priority;
  bool                  m_show_line_numbers;
  bool                  m_daemon;
  time_t                m_last_time;
};


extern LogWriter logger;

} // namespace Logger

} // namespace SWC



#ifndef SWC_DISABLE_LOG_ALL

#define SWC_LOG(priority, msg) \
  if(::SWC::Logger::logger.is_enabled(priority)) { \
    if(::SWC::Logger::logger.show_line_numbers()) \
      ::SWC::Logger::logger.log(priority, __FILE__, __LINE__, msg); \
    else \
      ::SWC::Logger::logger.log(priority, msg); \
  }

#define SWC_LOGF(priority, fmt, ...) \
  if(::SWC::Logger::logger.is_enabled(priority)) { \
    if(::SWC::Logger::logger.show_line_numbers()) \
      ::SWC::Logger::logger.log(priority, __FILE__, __LINE__, fmt, __VA_ARGS__); \
    else \
      ::SWC::Logger::logger.log(priority, fmt, __VA_ARGS__); \
  }


#ifndef SWC_DISABLE_LOG_FATAL ////
#define SWC_LOG_FATAL(msg) do { \
  SWC_LOG(::SWC::LOG_FATAL, msg); \
  SWC_ABORT; \
} while (0)
#define SWC_LOG_FATALF(msg, ...) do { \
  SWC_LOGF(::SWC::LOG_FATAL, msg, __VA_ARGS__); \
  SWC_ABORT; \
} while (0)
#else
#define SWC_LOG_FATAL(msg)
#define SWC_LOG_FATALF(msg, ...)
#endif



// stream interface, SWC_LOG_OUT(LOG_ERROR) << "msg" << SWC_LOG_OUT_END;

#define SWC_LOG_OUT(priority) \
  if(::SWC::Logger::logger.is_enabled(priority)) { \
    uint8_t _priority_ = priority; \
    ::SWC::Mutex::scope lock(::SWC::Logger::logger.mutex); \
    if(::SWC::Logger::logger.show_line_numbers()) \
      std::cout << ::SWC::Logger::logger.seconds() \
                << ' ' << ::SWC::Logger::logger.get_name(priority)  \
                << ": (" << __FILE__ << ':' << __LINE__ << ") "; \
    else \
      std::cout << ::SWC::Logger::logger.seconds() \
                << ' ' << ::SWC::Logger::logger.get_name(priority) << ": "; \
  std::cout 

#define SWC_LOG_OUT_END std::endl; if(_priority_ == ::SWC::LOG_FATAL) SWC_ABORT; }

#define SWC_PRINT \
  { \
    ::SWC::Mutex::scope lock(::SWC::Logger::logger.mutex); \
    std::cout 
#define SWC_PRINT_CLOSE std::endl; }
//

/*
#define HT_LOG_ENTER \
  if(::SWC::Logger::logger.is_enabled(LOG_DEBUG)) {\
    if(::SWC::Logger::logger.show_line_numbers()) \
      ::SWC::Logger::logger.debug("(%s:%d) %s() ENTER", __FILE__, __LINE__, __PRETTY_FUNCTION__);\
    else \
      ::SWC::Logger::logger.debug("%s() ENTER", __PRETTY_FUNCTION__); \
  }

#define HT_LOG_EXIT \
  if(::SWC::Logger::logger.is_enabled(LOG_DEBUG)) { \
    if(::SWC::Logger::logger.show_line_numbers()) \
      ::SWC::Logger::logger.debug("(%s:%d) %s() EXIT", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    else \
      ::SWC::Logger::logger.debug("%s() EXIT", __PRETTY_FUNCTION__); \
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




#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Logger.cc"
#endif 

#endif