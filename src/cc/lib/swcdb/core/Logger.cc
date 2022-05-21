/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Error.h"
#include "swcdb/core/Logger.h"
#include <stdarg.h>


namespace SWC { namespace Core {

/** INITIATE SINGLETONE LOGGER INSTANCE. **/
LogWriter logger;


namespace { //local namespace

static SWC_CAN_INLINE
const char* get_name(uint8_t priority) noexcept {
  switch(priority) {
    case LOG_FATAL:  return "FATAL";
    case LOG_ALERT:  return "ALERT";
    case LOG_CRIT:   return "CRIT";
    case LOG_ERROR:  return "ERROR";
    case LOG_WARN:   return "WARN";
    case LOG_NOTICE: return "NOTICE";
    case LOG_INFO:   return "INFO";
    case LOG_DEBUG:  return "DEBUG";
    default:         return "NOTSET";
  }
}

}


std::string LogWriter::repr(uint8_t priority) {
  return priority <= LOG_DEBUG ?
          get_name(priority)
        : "undefined logging level: " +std::to_string(priority);
}

uint8_t LogWriter::from_string(const std::string& loglevel) noexcept {
  if(Condition::str_case_eq(loglevel.c_str(), "info", loglevel.length()))
    return LOG_INFO;
  if(Condition::str_case_eq(loglevel.c_str(), "debug", loglevel.length()))
    return LOG_DEBUG;
  if(Condition::str_case_eq(loglevel.c_str(), "notice", loglevel.length()))
    return LOG_NOTICE;
  if(Condition::str_case_eq(loglevel.c_str(), "warn", loglevel.length()))
    return LOG_WARN;
  if(Condition::str_case_eq(loglevel.c_str(), "error", loglevel.length()))
    return LOG_ERROR;
  if(Condition::str_case_eq(loglevel.c_str(), "crit", loglevel.length()))
    return LOG_CRIT;
  if(Condition::str_case_eq(loglevel.c_str(), "alert", loglevel.length()))
    return LOG_ALERT;
  if(Condition::str_case_eq(loglevel.c_str(), "fatal", loglevel.length()))
    return LOG_FATAL;
  return -1;
}

SWC_SHOULD_NOT_INLINE
LogWriter::LogWriter(const std::string& name, const std::string& logs_path)
                    : m_name(name), m_logs_path(logs_path),
                      m_file_out(stdout), //m_file_err(stderr),
                      m_priority(LOG_INFO), m_show_line_numbers(true),
                      m_daemon(false), m_next_time(0) {
}
LogWriter::~LogWriter() noexcept { }

SWC_SHOULD_NOT_INLINE
void LogWriter::initialize(const std::string& name) {
  Core::MutexSptd::scope lock(mutex);
  m_name.clear();
  m_name.append(name);
}

SWC_SHOULD_NOT_INLINE
void LogWriter::daemon(const std::string& logs_path) {
  errno = 0;

  Core::MutexSptd::scope lock(mutex);
  m_logs_path = logs_path;
  if(m_logs_path.back() != '/')
    m_logs_path.append("/");
  m_daemon = true;

  _renew_files(::time(nullptr));

  if(errno)
    throw std::runtime_error(
      "SWC::Core::LogWriter::initialize err="
      + std::to_string(errno)+"("+Error::get_text(errno)+")"
    );
  std::fclose(stderr);
}


SWC_SHOULD_NOT_INLINE
void LogWriter::_time_and_level(uint8_t priority) {
  auto t = ::time(nullptr);
  if(m_daemon && m_next_time < t)
    _renew_files(t);
  t -= (t/86400) * 86400; // time since start of a day
  SWC_LOG_OSTREAM << t << ' ' << get_name(priority) << ':' << ' ';
}


#if defined(__MINGW64__) || defined(_WIN32)
  #define SWC_MKDIR(_path, _perms) ::mkdir(_path)
#else
  #define SWC_MKDIR(_path, _perms) ::mkdir(_path, _perms)
#endif


void LogWriter::_renew_files(time_t secs) {
  errno = 0;
  m_next_time = (secs/86400)*86400;;
  auto ltm = localtime(&m_next_time);
  m_next_time += 86400;

  std::string filepath;
  filepath.reserve(m_logs_path.size() + m_name.size() + 16);
  filepath.append(m_logs_path);

  SWC_MKDIR(filepath.c_str(), 0755);
  filepath.append(std::to_string(1900+ltm->tm_year));
  SWC_MKDIR(filepath.c_str(), 0755);
  filepath.append("/");
  filepath.append(std::to_string(1+ltm->tm_mon));
  SWC_MKDIR(filepath.c_str(), 0755);
  filepath.append("/");
  filepath.append(std::to_string(ltm->tm_mday));
  SWC_MKDIR(filepath.c_str(), 0755);
  if(errno == EEXIST)
    errno = 0;
  filepath.append("/");
  filepath.append(m_name);
  filepath.append(".log");

  if(!errno) {
    SWC_LOG_OSTREAM << "Changing Standard Output File to="
                    << filepath << std::endl;
    m_file_out = std::freopen(filepath.c_str(), "w", m_file_out);

    std::cerr.rdbuf(std::cout.rdbuf());
    //std::string filepath_err(filepath+".err");
    //std::cerr << "Changing Error Output File to=" << filepath_err << "\n";
    //m_file_err = std::freopen(filepath_err.c_str(), "w", m_file_err);
  }
  /* else { fallback
    rdbuf
    m_file_out = std::freopen('0', "w", m_file_out);
    m_file_err = std::freopen('1', "w", m_file_err);
    ::fdopen(0, "wt");
  }*/
}
#undef SWC_MKDIR

SWC_SHOULD_NOT_INLINE
void LogWriter::log(uint8_t priority, const char* fmt, ...) noexcept {
  try {
    va_list ap;
    va_start(ap, fmt);
    try {
      Core::MutexSptd::scope lock(mutex);
      _time_and_level(priority);
      vprintf(fmt, ap);
      SWC_LOG_OSTREAM << std::endl;
    } catch(...) { }
    va_end(ap);
  } catch(...) { }
}

SWC_SHOULD_NOT_INLINE
void LogWriter::log(uint8_t priority, const char* filen, int fline,
                    const char* fmt, ...) noexcept {
  try {
    va_list ap;
    va_start(ap, fmt);
    try {
      Core::MutexSptd::scope lock(mutex);
      _time_and_level(priority);
      if(show_line_numbers())
        SWC_LOG_OSTREAM << '(' << filen << ':' << fline << ')' << '"';
      vprintf(fmt, ap);
      SWC_LOG_OSTREAM << std::endl;
    } catch(...) { }
    va_end(ap);
  } catch(...) { }
}

SWC_SHOULD_NOT_INLINE
void LogWriter::_print_prefix(uint8_t priority, const char* filen, int fline) {
  _time_and_level(priority);
  if(show_line_numbers())
    SWC_LOG_OSTREAM << "(" << filen << ':' << fline << ") ";
}

}}
