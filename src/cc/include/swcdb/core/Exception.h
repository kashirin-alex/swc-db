/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Exception_h
#define swcdb_core_Exception_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/Error.h"
#include "swcdb/core/String.h"
#include "swcdb/core/Logger.h"

#include <stdexcept>


namespace SWC { namespace Error {


class Exception final : public std::exception {
  public:

  static const Exception make(const std::exception_ptr& eptr,
                              const std::string& msg,
                              const Exception* prev = nullptr);

  Exception(int code, const std::string& msg,
            int line = 0, const char* func = nullptr,
            const char* file = nullptr,
            const std::string& inner_msg = "");

  Exception(int code, const std::string& msg, const Exception* prev,
            const std::string& inner_msg);

  Exception(int code, const std::string& msg, const Exception* prev,
            int line = 0, const char* func = nullptr,
            const char* file = nullptr,
            const std::string& inner_msg = "");

  Exception(int code, const std::string& msg, const Exception& prev,
            int line = 0, const char* func = nullptr,
            const char* file = nullptr,
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

  friend std::ostream& operator<<(std::ostream& out, const Exception& e) {
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


}} //  namespace SWC::Error




// EXCEPTION HELPERS
#define SWC_EXCEPTION(_code_, _msg_) \
  ::SWC::Error::Exception(\
    _code_, _msg_, __LINE__, __PRETTY_FUNCTION__, __FILE__)

#define SWC_EXCEPTION2(_code_, _ex_, _msg_) \
  ::SWC::Error::Exception(\
    _code_, _msg_, _ex_, __LINE__, __PRETTY_FUNCTION__, __FILE__)


#define SWC_CURRENT_EXCEPTION(_msg_) \
  ::SWC::Error::Exception::make(std::current_exception(), _msg_)


/* preferred:
try {
  --code--
} catch (...) {
  const Error::Exception& e = SWC_CURRENT_EXCEPTION("INFO..");
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
  try { _code_ } catch(...) { throw SWC_CURRENT_EXCEPTION(_s_); }

#define SWC_TRY_OR_LOG(_s_, _code_) \
  try { _code_ } catch(...) { SWC_LOG_CURRENT_EXCEPTION(_s_); }


// CONDITION HELPERS
#define SWC_EXPECT(_e_, _code_) \
  if (!(_e_)) { \
    if (_code_ == ::SWC::Error::FAILED_EXPECTATION) \
      SWC_LOG_FATAL("failed expectation: " #_e_); \
    SWC_THROW(_code_, "failed expectation: " #_e_); \
  }

#define SWC_ASSERT(_e_) SWC_EXPECT(_e_, ::SWC::Error::FAILED_EXPECTATION)


#define SWC_EXPECTF(_e_, _code_, _fmt_, ...) \
  if (!(_e_)) { \
    if (_code_ == ::SWC::Error::FAILED_EXPECTATION) \
      SWC_LOG_FATALF(_fmt_, __VA_ARGS__); \
    SWC_THROWF(_code_, _fmt_, __VA_ARGS__); \
  }

#define SWC_ASSERTF(_e_, _fmt_, ...) \
  SWC_EXPECTF(_e_, ::SWC::Error::FAILED_EXPECTATION, _fmt_, __VA_ARGS__)




#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Exception.cc"
#endif


#endif // swcdb_core_Exception_h
