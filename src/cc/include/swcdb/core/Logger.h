/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Logger_h
#define swcdb_core_Logger_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/MutexSptd.h"

#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <stdio.h>



namespace SWC { 


/*!
 *  \addtogroup Core
 *  @{
 */
  
enum LogPriority : uint8_t {
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


/*! @} End of Core Group*/



namespace Core {


class LogWriter final {
  public:
  
  static std::string repr(uint8_t priority);

  static const char* get_name(uint8_t priority);

  static uint8_t from_string(const std::string& loglevel);


  MutexSptd    mutex;


  LogWriter(const std::string& name="", const std::string& logs_path="");
  
  ~LogWriter();
  
  void initialize(const std::string& name);

  void daemon(const std::string& logs_path);

  void set_level(uint8_t level) {
    m_priority = level;
  }

  uint8_t get_level() const {
    return m_priority;
  }

  bool is_enabled(uint8_t level) const {
    return level <= m_priority;
  }

  bool show_line_numbers() const {
    return m_show_line_numbers;
  }

  bool print_prefix(uint8_t priority, const char* filen, int fline);

  void print_suffix(bool support);

  uint32_t _seconds();

  void log(uint8_t priority, const char* fmt, ...)
      __attribute__((format(printf, 3, 4)));

  void log(uint8_t priority, const char* filen, int fline, 
           const char* fmt, ...)
      __attribute__((format(printf, 5, 6)));

  template<typename T>
  SWC_SHOULD_NOT_INLINE
  void msg(uint8_t priority, const T& message) {
    MutexSptd::scope lock(mutex);
    std::cout << _seconds() << ' ' << get_name(priority) << ": " 
              << message << std::endl;
  }

  template<typename T>
  SWC_SHOULD_NOT_INLINE
  void msg(uint8_t priority, const char* filen, int fline, const T& message) {
    bool support(print_prefix(priority, filen, fline));
    std::cout << message;
    print_suffix(support);
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

}} // namespace SWC::Core




/*!
 *  \addtogroup Core
 *  @{
 */


#define SWC_LOG_OSTREAM std::cout

#define SWC_LOG_PRINTF(fmt, ...) printf(fmt, __VA_ARGS__)

#define SWC_PRINT { \
  ::SWC::Core::MutexSptd::scope lock(::SWC::Core::logger.mutex); \
  SWC_LOG_OSTREAM
#define SWC_PRINT_CLOSE std::endl; }


#ifndef SWC_DISABLE_LOG_ALL


// stream interface, SWC_LOG_OUT(LOG_ERROR, code{SWC_LOG_OSTREAM << "msg";});
#define SWC_LOG_OUT(pr, _code_) { \
  uint8_t _log_pr = pr; \
  if(::SWC::Core::logger.is_enabled(_log_pr)) { \
    bool _support( \
      ::SWC::Core::logger.print_prefix(_log_pr, __FILE__, __LINE__)); \
    {_code_}; \
    ::SWC::Core::logger.print_suffix(_support); \
  } }

#define SWC_LOGF(priority, fmt, ...) \
  SWC_LOG_OUT(priority, SWC_LOG_PRINTF(fmt, __VA_ARGS__); )

#define SWC_LOG(priority, message) \
  if(::SWC::Core::logger.is_enabled(priority)) { \
    if(::SWC::Core::logger.show_line_numbers()) \
      ::SWC::Core::logger.msg(priority, __FILE__, __LINE__, message); \
    else \
      ::SWC::Core::logger.msg(priority, message); \
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

#else // SWC_DISABLE_LOGGING

#define SWC_LOG(priority, msg)
#define SWC_LOGF(priority, fmt, ...)

#define SWC_LOG_OUT(pr, code)

#define SWC_LOG_FATAL(msg)
#define SWC_LOG_FATAL(msg, ...)

#endif // SWC_DISABLE_LOGGING



/*! @} End of Core Group*/




#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Logger.cc"
#endif 

#endif // swcdb_core_Logger_h
