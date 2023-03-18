/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"

#include <future>
//#include <format>
#include <regex>
#include <variant>
#include <optional>

#if defined(__MINGW64__) || defined(_WIN32)
  #include <Winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <netdb.h>
#endif


namespace SWC { namespace Error {


SWC_SHOULD_NOT_INLINE
Exception Exception::make(std::exception_ptr&& eptr,
                          std::string&& msg,
                          Exception* prev) noexcept {
  try {
    std::rethrow_exception(std::move(eptr));

  } catch (Exception& e) {
    return e;


  } catch (const std::ios_base::failure& e) {
    return Exception(Error::IO_ERROR, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  /* stdc++=20+
  } catch (const std::format_error& e) {
    return Exception(Error::BAD_FORMAT, std::move(msg), prev, 0, nullptr, nullptr, e.what());
  */

  } catch (const std::regex_error& e) {
    return Exception(Error::BAD_REGEXP, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::overflow_error& e) {
    return Exception(EOVERFLOW, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::range_error& e) {
    return Exception(ERANGE, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::system_error& e) {
    return Exception(e.code().value(), std::move(msg), prev, 0, nullptr, nullptr, e.what());


  } catch (const std::invalid_argument& e) {
    return Exception(EINVAL, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::domain_error& e) {
    return Exception(EDOM, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::length_error& e) {
    return Exception(EOVERFLOW, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::out_of_range& e) {
    return Exception(ERANGE, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::future_error& e) {
    return Exception(
      SWC_ERRNO_FUTURE_BEGIN + e.code().value(), std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::logic_error& e) {
    return Exception(Error::BAD_LOGIC, std::move(msg), prev, 0, nullptr, nullptr, e.what());


  } catch (const std::bad_optional_access& e) {
    return Exception(EINVAL, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::bad_variant_access& e) {
    return Exception(ERANGE, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::bad_exception& e) {
    return Exception(Error::EXCEPTION_BAD, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::bad_alloc& e) {
    return Exception(Error::BAD_MEMORY_ALLOCATION, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::bad_function_call& e) {
    return Exception(Error::BAD_FUNCTION, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::bad_weak_ptr& e) {
    return Exception(Error::BAD_POINTER, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::bad_cast& e) {
    return Exception(Error::BAD_CAST, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::bad_typeid& e) {
    return Exception(Error::BAD_POINTER, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch (const std::exception& e) {
    return Exception(Error::EXCEPTION_UNKNOWN, std::move(msg), prev, 0, nullptr, nullptr, e.what());

  } catch(...) { }

  /* instead std::rethrow_exception(eptr) :
    std::exception_ptr missing exception ptr access let to cast
    typeid(std::system_error) == *e.__cxa_exception_type()
    or if e=std::dynamic_pointer_cast<std::system_error>(eptr.get());
  */
  return Exception(Error::EXCEPTION_UNKNOWN, std::move(msg), prev);
}

Exception::Exception(
  int code, 
  std::string&& msg,
  Exception* prev,
  int line,
  const char* func,
  const char* file,
  const char* inner_msg
) noexcept
  : _code(code),
    _msg(std::move(msg)),
    _line(line),
    _func(func),
    _file(file),
    _inner_msg(),
    _prev(nullptr) {

    try {
      if (inner_msg)
        _inner_msg.assign(inner_msg);
      if(prev)
        _prev = new Exception(std::move(*prev));
    } catch(...) {
      _code = Error::BAD_MEMORY_ALLOCATION;
    }

}

Exception::Exception(Exception& other) noexcept
                    : _code(other._code), _msg(std::move(other._msg)),
                      _line(other._line), _func(other._func), _file(other._file),
                      _inner_msg(std::move(other._inner_msg)),
                      _prev(other._prev) {
  other._prev = nullptr;
}

Exception::Exception(Exception&& other) noexcept
                    : _code(other._code), _msg(std::move(other._msg)),
                      _line(other._line), _func(other._func), _file(other._file),
                      _inner_msg(std::move(other._inner_msg)),
                      _prev(other._prev) {
  other._prev = nullptr;
}

Exception::~Exception() noexcept {
  delete _prev;
}

std::string Exception::message() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
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
