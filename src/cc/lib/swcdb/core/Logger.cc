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

const char* priority_name[] = {
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

std::string LogWriter::repr(uint8_t priority) {
  return priority < LOG_NOTSET ?
          get_name(priority)
        : "undefined logging level: " +std::to_string(priority);
}

const char* LogWriter::get_name(uint8_t priority) {
  return priority <= LOG_NOTSET ? priority_name[priority] : "UNDEFINED";
}

uint8_t LogWriter::from_string(const std::string& loglevel) {
  if(!strncasecmp(loglevel.c_str(), "info", loglevel.length()))
    return LOG_INFO;
  if(!strncasecmp(loglevel.c_str(), "debug", loglevel.length()))
    return LOG_DEBUG;
  if(!strncasecmp(loglevel.c_str(), "notice", loglevel.length()))
    return LOG_NOTICE;
  if(!strncasecmp(loglevel.c_str(), "warn", loglevel.length()))
    return LOG_WARN;
  if(!strncasecmp(loglevel.c_str(), "error", loglevel.length()))
    return LOG_ERROR;
  if(!strncasecmp(loglevel.c_str(), "crit", loglevel.length()))
    return LOG_CRIT;
  if(!strncasecmp(loglevel.c_str(), "alert", loglevel.length()))
    return LOG_ALERT;
  if(!strncasecmp(loglevel.c_str(), "fatal", loglevel.length()))
    return LOG_FATAL;
  return -1;
}

LogWriter::LogWriter(const std::string& name, const std::string& logs_path)
                    : m_name(name), m_logs_path(logs_path),
                      m_file_out(stdout), //m_file_err(stderr),
                      m_priority(LOG_INFO), m_show_line_numbers(true),
                      m_daemon(false), m_last_time(0) {
  //std::cout << " LogWriter()=" << size_t(this) << "\n";
}
LogWriter::~LogWriter() {
  std::cout << std::flush;
}

void LogWriter::initialize(const std::string& name) {
  //std::cout << " LogWriter::initialize name=" << name
  //          << " ptr=" << size_t(this) << "\n";
  Core::MutexSptd::scope lock(mutex);
  m_name.clear();
  m_name.append(name);
}

void LogWriter::daemon(const std::string& logs_path) {
  //std::cout << " LogWriter::daemon logs_path=" << logs_path
  //          << " ptr=" << size_t(this) << "\n";
  errno = 0;

  Core::MutexSptd::scope lock(mutex);
  m_logs_path = logs_path;
  if(m_logs_path.back() != '/')
    m_logs_path.append("/");
  m_daemon = true;

  renew_files();

  if(errno)
    throw std::runtime_error(
      "SWC::Core::LogWriter::initialize err="
      + std::to_string(errno)+"("+Error::get_text(errno)+")"
    );
  std::fclose(stderr);
  std::cerr << " AFTER(std::fclose(stderr);) \n";
}


SWC_SHOULD_NOT_INLINE
uint32_t LogWriter::_seconds() {
  auto t = ::time(nullptr);
  if(m_daemon && m_last_time < t-86400)
    renew_files();
  return t-86400*(t/86400);
  // seconds since start of a day
}

void LogWriter::renew_files() {
  errno = 0;
  m_last_time = (::time(nullptr)/86400)*86400;
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
  //std::string filepath_err(filepath+".err");

  if(!errno) {
    std::cout << "Changing Standard Output File to=" << filepath_out << "\n";
    m_file_out = std::freopen(filepath_out.c_str(), "w", m_file_out);

    std::cerr.rdbuf(std::cout.rdbuf());
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

SWC_SHOULD_NOT_INLINE
void LogWriter::log(uint8_t priority, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  {
    Core::MutexSptd::scope lock(mutex);
    std::cout << _seconds() << ' ' << get_name(priority) << ": ";
    vprintf(fmt, ap);
    std::cout << std::endl;
  }
  va_end(ap);
}

SWC_SHOULD_NOT_INLINE
void LogWriter::log(uint8_t priority, const char* filen, int fline,
                    const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  bool support(print_prefix(priority, filen, fline));
  vprintf(fmt, ap);
  print_suffix(support);

  va_end(ap);
}

SWC_SHOULD_NOT_INLINE
bool LogWriter::print_prefix(uint8_t priority, const char* filen, int fline) {
  bool support = mutex.lock();
  std::cout << _seconds() << ' ' << get_name(priority) << ": ";
  if(show_line_numbers())
    std::cout << "(" << filen << ':' << fline << ") ";
  return support;
}

SWC_SHOULD_NOT_INLINE
void LogWriter::print_suffix(bool support) {
  std::cout << std::endl;
  mutex.unlock(support);
}

}}
