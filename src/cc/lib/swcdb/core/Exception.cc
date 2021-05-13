/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"

#include <future>
//#include <format>
#include <regex>
#include <variant>
#include <netdb.h>
#include <optional>



namespace SWC { namespace Error {


SWC_SHOULD_NOT_INLINE
const Exception Exception::make(const std::exception_ptr& eptr,
                                const std::string& msg,
                                const Exception* prev) {
  try {
    std::rethrow_exception(eptr);

  } catch (const Exception& e) {
    return Exception(e);


  } catch (const std::ios_base::failure& e) {
    return Exception(Error::IO_ERROR, msg, prev, e.what());

  /* stdc++=20+
  } catch (const std::format_error& e) {
    return Exception(Error::BAD_FORMAT, msg, prev, e.what());
  */

  } catch (const std::regex_error& e) {
    return Exception(Error::BAD_REGEXP, msg, prev, e.what());

  } catch (const std::overflow_error& e) {
    return Exception(EOVERFLOW, msg, prev, e.what());

  } catch (const std::range_error& e) {
    return Exception(ERANGE, msg, prev, e.what());

  } catch (const std::system_error& e) {
    return Exception(e.code().value(), msg, prev, e.what());


  } catch (const std::invalid_argument& e) {
    return Exception(EINVAL, msg, prev, e.what());

  } catch (const std::domain_error& e) {
    return Exception(EDOM, msg, prev, e.what());

  } catch (const std::length_error& e) {
    return Exception(EOVERFLOW, msg, prev, e.what());

  } catch (const std::out_of_range& e) {
    return Exception(ERANGE, msg, prev, e.what());

  } catch (const std::future_error& e) {
    return Exception(
      SWC_ERRNO_FUTURE_BEGIN + e.code().value(), msg, prev, e.what());

  } catch (const std::logic_error& e) {
    return Exception(Error::BAD_LOGIC, msg, prev, e.what());


  } catch (const std::bad_optional_access& e) {
    return Exception(EINVAL, msg, prev, e.what());

  } catch (const std::bad_variant_access& e) {
    return Exception(ERANGE, msg, prev, e.what());

  } catch (const std::bad_exception& e) {
    return Exception(Error::EXCEPTION_BAD, msg, prev, e.what());

  } catch (const std::bad_alloc& e) {
    return Exception(Error::BAD_MEMORY_ALLOCATION, msg, prev, e.what());

  } catch (const std::bad_function_call& e) {
    return Exception(Error::BAD_FUNCTION, msg, prev, e.what());

  } catch (const std::bad_weak_ptr& e) {
    return Exception(Error::BAD_POINTER, msg, prev, e.what());

  } catch (const std::bad_cast& e) {
    return Exception(Error::BAD_CAST, msg, prev, e.what());

  } catch (const std::bad_typeid& e) {
    return Exception(Error::BAD_POINTER, msg, prev, e.what());

  } catch (const std::exception& e) {
    return Exception(Error::EXCEPTION_UNKNOWN, msg, prev, e.what());

  } catch(...) { }

  /* instead std::rethrow_exception(eptr) :
    std::exception_ptr missing exception ptr access let to cast
    typeid(std::system_error) == *e.__cxa_exception_type()
    or if e=std::dynamic_pointer_cast<std::system_error>(eptr.get());
  */
  return Exception(Error::EXCEPTION_UNKNOWN, msg, prev);
}

Exception::Exception(int code, const std::string& msg,
                     int line, const char* func, const char* file,
                     const std::string& inner_msg)
                    : _code(code), _msg(msg),
                    _line(line), _func(func), _file(file),
                    _inner_msg(inner_msg),
                    _prev(nullptr) {
}

Exception::Exception(int code, const std::string& msg, const Exception* prev,
                     const std::string& inner_msg)
                    : _code(code), _msg(msg),
                      _line(0), _func(nullptr), _file(nullptr),
                      _inner_msg(inner_msg),
                      _prev(prev ? new Exception(*prev) : prev) {
}

Exception::Exception(int code, const std::string& msg, const Exception* prev,
                     int line , const char* func, const char* file,
                     const std::string& inner_msg)
                    : _code(code), _msg(msg),
                      _line(line), _func(func), _file(file),
                      _inner_msg(inner_msg),
                      _prev(prev ? new Exception(*prev) : prev) {
}

Exception::Exception(int code, const std::string& msg, const Exception& prev,
                     int line, const char* func, const char* file,
                     const std::string& inner_msg)
                    : _code(code), _msg(msg),
                      _line(line), _func(func), _file(file),
                      _inner_msg(inner_msg),
                      _prev(new Exception(prev)) {
}

Exception::Exception(const Exception& other)
                    : _code(other._code), _msg(other._msg),
                      _line(other._line), _func(other._func), _file(other._file),
                      _inner_msg(other._inner_msg),
                      _prev(other._prev) {
  other._prev = nullptr;
}

Exception::~Exception() {
  if(_prev)
    delete _prev;
}

std::string Exception::message() const {
  std::stringstream ss;
  print(ss);
  return ss.str().c_str();
}

SWC_SHOULD_NOT_INLINE
void Exception::print(std::ostream& out) const {
  size_t n = 1;
  print_base(out << '#' << n << " ");
  for(const Exception* p = _prev; p; p = p->_prev)
    p->print_base(out << '#' << ++n << " ");
}

void Exception::print_base(std::ostream& out) const {
  out << "Exception: ";
  if(!_msg.empty())
    out << _msg << " ";

  out << _code << "(" << Error::get_text(_code);
  if(!_inner_msg.empty())
    out << ", " << _inner_msg;
  out << ")";

  if(_line) {
    out << "\n\tat " << (_func ? _func : "-")
        << " (" << (_file ? _file : "-");
    if (Core::logger.show_line_numbers())
      out << ':'<< _line;
    out << ')';
  }
}


}} // namespace SWC::Error
