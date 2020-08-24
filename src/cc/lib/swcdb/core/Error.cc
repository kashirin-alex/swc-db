/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Error.h"
#include <map>
#include <future>
//#include <format>
#include <regex>
#include <variant>
#include <netdb.h>



namespace SWC {


namespace {

std::map<const int, const char *> text_map {

  { Error::EXCEPTION_BAD,                  "Bad Exception" },
  { Error::EXCEPTION_UNKNOWN,              "Unknown Exception" },
  { Error::UNPOSSIBLE,                     "But that's unpossible!" },
  { Error::FAILED_EXPECTATION,             "Failed Expectation" },

  { Error::OK,                             "ok" },

  { Error::IO_ERROR,                        "i/o error" },
  { Error::BAD_MEMORY_ALLOCATION,           "bad memory allocation"},
  { Error::BAD_FUNCTION,                    "bad function"},
  { Error::BAD_POINTER,                     "bad pointer"},
  { Error::BAD_CAST,                        "bad cast"},
  { Error::BAD_FORMAT,                      "bad format"},
  { Error::BAD_REGEXP,                      "bad regexp"},
  { Error::BAD_LOGIC,                       "bad logic"},
  
  { Error::CANCELLED,                       "cancelled"},
  { Error::NOT_ALLOWED,                     "not allowed" },
  { Error::INVALID_ARGUMENT,                "invalid argument" },
  { Error::INCOMPATIBLE_OPTIONS,            "incompatible options" },

  { Error::SERIALIZATION_INPUT_OVERRUN,     "SERIALIZATION input overrun" },
  { Error::CHECKSUM_MISMATCH,               "checksum mismatch" },

  { Error::COMM_NOT_CONNECTED,              "COMM not connected" },
  { Error::COMM_CONNECT_ERROR,              "COMM connect error" },
  { Error::COMM_SEND_ERROR,                 "COMM send error" },
  { Error::COMM_HEADER_CHECKSUM_MISMATCH,   "COMM header checksum mismatch" },
  { Error::COMM_BAD_HEADER,                 "COMM bad header" },

  { Error::PROTOCOL_ERROR,                  "protocol error" },
  { Error::REQUEST_TRUNCATED_HEADER,        "request truncated header" },
  { Error::REQUEST_TRUNCATED_PAYLOAD,       "request truncated payload" },
  { Error::REQUEST_TIMEOUT,                 "request timeout" },
  { Error::NOT_IMPLEMENTED,                 "not implemented"},

  { Error::ENCODER_ENCODE,                  "encoder decode-error" },
  { Error::ENCODER_DECODE,                  "encoder encode-error" },

  { Error::CONFIG_BAD_CFG_FILE,             "bad cfg file"},
  { Error::CONFIG_GET_ERROR,                "failed to get config value"},
  { Error::CONFIG_BAD_VALUE,                "bad config value"},

  { Error::SERVER_NOT_READY,                "server not ready" },
  { Error::SERVER_SHUTTING_DOWN,            "server shutting down" },

  { Error::MNGR_NOT_ACTIVE,                 "Manager not active for the duty" },
  { Error::MNGR_NOT_INITIALIZED,            "Manager is initializing" },

  { Error::RGR_NOT_READY,                   "Ranger is not ready, lacks id"},
  { Error::RGR_NOT_LOADED_RANGE,            "Ranger range id not loaded"},
  { Error::RGR_DELETED_RANGE,               "Ranger range state deleted"},

  { Error::FS_BAD_FILE_HANDLE,              "FS bad file handle" },
  { Error::FS_PERMISSION_DENIED,            "FS permission denied" },
  { Error::FS_EOF,                          "FS end of file" },
  { Error::FS_PATH_NOT_FOUND,               "FS destination path" },

  { Error::COLUMN_NOT_READY,                "Column is not ready"},
  { Error::COLUMN_NOT_EXISTS,               "Column does not exist"},
  { Error::COLUMN_REACHED_ID_LIMIT,         "Columd ID max-reached"},
  { Error::COLUMN_MARKED_REMOVED,           "Column is being removed"},
  { Error::COLUMN_UNKNOWN_GET_FLAG,         "unknown get column flag!"},
    
  { Error::COLUMN_SCHEMA_NAME_EXISTS,       "Schema column name already exists!"},
  { Error::COLUMN_SCHEMA_NAME_NOT_EXISTS,   "Schema column name doesn't exist!"},
  { Error::COLUMN_SCHEMA_BAD_SAVE,          "Schema for save not matches saved"},
  { Error::COLUMN_SCHEMA_NAME_EMPTY,        "Schema's column name cannot be empty"},
  { Error::COLUMN_SCHEMA_NAME_NOT_CORRES,   "Schema's column name not correspond"},
  { Error::COLUMN_SCHEMA_ID_EMPTY,          "Schema's column id cannot be empty"},
  { Error::COLUMN_SCHEMA_NOT_DIFFERENT,     "Schemas expected to be different"},
  { Error::COLUMN_SCHEMA_MISSING,           "cid's Schema is missing"},
  { Error::COLUMN_SCHEMA_IS_SYSTEM,         "Action not allowed on system schema"},
  { Error::COLUMN_CHANGE_INCOMPATIBLE,      "Schema seq/type/sysname incompatible"},
  
  { Error::RANGE_NOT_FOUND,                 "No corresponding range"},
  { Error::RANGE_CELLSTORES,                "Cellstores had Errors"},
  { Error::RANGE_COMMITLOG,                 "Commitlog had Errors"},
  { Error::RANGE_BAD_INTERVAL,              "Partial Write, bad range interval begin /& end"},

  { Error::SQL_PARSE_ERROR,                 "SQL parse error" },
  { Error::SQL_BAD_LOAD_FILE_FORMAT,        "SQL bad load file format" },

  { Error::CLIENT_DATA_REMAINED,            "Data remained on client input" },

};

static const char ERROR_NOT_REGISTERED[] = "ERROR NOT REGISTERED";

} // local namespace


const char* Error::get_text(const int err) {
  const char* text = nullptr;
  switch(err) {
    case SWC_ERRNO_SYS_BEGIN ... SWC_ERRNO_SYS_END:
       text = strerror(err);
       break;
    case SWC_ERRNO_FUTURE_BEGIN ... SWC_ERRNO_FUTURE_END:
       text =  std::future_error(
         std::future_errc(err - SWC_ERRNO_FUTURE_BEGIN)).what();
       break;
    case SWC_ERRNO_EAI_BEGIN ... SWC_ERRNO_EAI_END:
       text = gai_strerror(-(err - SWC_ERRNO_EAI_BEGIN));
       break;
    case SWC_ERRNO_APP_BEGIN ... SWC_ERRNO_APP_END:
       text = text_map[err];
       break;
    default:
      break;
  }
  return text ? text : ERROR_NOT_REGISTERED;
}




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
                      _line(0), _func(0), _file(0), 
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
  render(ss);
  return ss.str().c_str();
}

void Exception::render(std::ostream& out) const {
  size_t n = 1;
  render_base(out << '#' << n << " ");
  for(const Exception* p = _prev; p; p = p->_prev)
    p->render_base(out << '#' << ++n << " ");
}

void Exception::render_base(std::ostream& out) const {
  out << "SWC::Exception: ";
  if(!_msg.empty()) 
    out << _msg << " ";

  out << _code << "(" << Error::get_text(_code);
  if(!_inner_msg.empty())
    out << ", " << _inner_msg;
  out << ")";

  if(_line) {
    out << "\n\tat " << (_func ? _func : "-") 
        << " (" << (_file ? _file : "-");
    if (Logger::logger.show_line_numbers())
      out << ':'<< _line;
    out << ')';
  }
}


}
