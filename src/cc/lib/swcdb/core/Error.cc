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



namespace SWC {


namespace {

std::map<const int, const char *> text_map {

  { Error::EXCEPTION_BAD,               "Bad Exception" },
  { Error::EXCEPTION_UNKNOWN,           "Unknown Exception" },
  { Error::UNPOSSIBLE,                  "But that's unpossible!" },
  { Error::FAILED_EXPECTATION,          "Failed Expectation" },

  { Error::OK,                          "ok" },

  { Error::IO_ERROR,                    "i/o error" },
  { Error::BAD_MEMORY_ALLOCATION,       "bad memory allocation"},
  { Error::BAD_FUNCTION,                "bad function"},
  { Error::BAD_POINTER,                 "bad pointer"},
  { Error::BAD_CAST,                    "bad cast"},
  { Error::BAD_FORMAT,                  "bad format"},
  { Error::BAD_REGEXP,                  "bad regexp"},
  { Error::BAD_LOGIC,                   "bad logic"},
  

  { Error::COMM_NOT_CONNECTED,          "COMM not connected" },
  { Error::COMM_BROKEN_CONNECTION,      "COMM broken connection" },
  { Error::COMM_CONNECT_ERROR,          "COMM connect error" },
  { Error::COMM_ALREADY_CONNECTED,      "COMM already connected" },
  { Error::COMM_SEND_ERROR,             "COMM send error" },
  { Error::COMM_RECEIVE_ERROR,          "COMM receive error" },
  { Error::COMM_POLL_ERROR,             "COMM poll error" },
  { Error::COMM_CONFLICTING_ADDRESS,    "COMM conflicting address" },
  { Error::COMM_SOCKET_ERROR,           "COMM socket error" },
  { Error::COMM_BIND_ERROR,             "COMM bind error" },
  { Error::COMM_LISTEN_ERROR,           "COMM listen error" },
  { Error::COMM_HEADER_CHECKSUM_MISMATCH,  "COMM header checksum mismatch" },
  { Error::COMM_PAYLOAD_CHECKSUM_MISMATCH, "COMM payload checksum mismatch" },
  { Error::COMM_BAD_HEADER,             "COMM bad header" },
  { Error::COMM_INVALID_PROXY,          "COMM invalid proxy" },

  { Error::PROTOCOL_ERROR,              "protocol error" },
  { Error::REQUEST_TRUNCATED_HEADER,    "request truncated header" },
  { Error::REQUEST_TRUNCATED_PAYLOAD,   "request truncated payload" },
  { Error::REQUEST_TIMEOUT,             "request timeout" },

  { Error::CONFIG_BAD_ARGUMENT,         "CONFIG bad argument(s)"},
  { Error::CONFIG_BAD_CFG_FILE,         "CONFIG bad cfg file"},
  { Error::CONFIG_GET_ERROR,            "CONFIG failed to get config value"},
  { Error::CONFIG_BAD_VALUE,            "CONFIG bad config value"},

  { Error::MNGR_NOT_ACTIVE,             "Manager not active for the duty" },
  { Error::MNGR_NOT_INITIALIZED,        "Manager is initializing" },
    
  { Error::COLUMN_SCHEMA_NAME_EXISTS,     "Schema column name already exists!"},
  { Error::COLUMN_SCHEMA_NAME_NOT_EXISTS, "Schema column name doesn't exist!"},
  { Error::COLUMN_UNKNOWN_GET_FLAG,       "unknown get column flag!"},
  { Error::COLUMN_REACHED_ID_LIMIT,       "Columd ID max-reached"},
  { Error::COLUMN_SCHEMA_BAD_SAVE,        "Schema for save not matches saved"},
  { Error::COLUMN_SCHEMA_NAME_EMPTY,      "Schema's column name cannot be empty"},
  { Error::COLUMN_SCHEMA_ID_EMPTY,        "Schema's column id cannot be empty"},
  { Error::COLUMN_SCHEMA_NAME_NOT_CORRES, "Schema's column name not correspond"},
  
  { Error::COLUMN_SCHEMA_NOT_DIFFERENT,   "Schemas expected to be different"},
  { Error::COLUMN_SCHEMA_MISSING,         "cid's Schema is missing"},
  { Error::COLUMN_MARKED_REMOVED,         "Column is being removed"},
  { Error::COLUMN_NOT_EXISTS,             "Column does not exist"},
  { Error::COLUMN_NOT_READY,              "Column is not ready"},
  { Error::COLUMN_CHANGE_INCOMPATIBLE,    "Schema seq/type/sysname incompatible"},
  { Error::COLUMN_SCHEMA_IS_SYSTEM,       "Action not allowed on system schema"},
  
  { Error::RANGE_NOT_FOUND,               "No corresponding range"},
  { Error::RANGE_CELLSTORES,              "Cellstores had Errors"},
  { Error::RANGE_COMMITLOG,               "Commitlog had Errors"},
  { Error::RANGE_BAD_INTERVAL,            "Partial Write, bad range interval begin /& end"},


  { Error::SERIALIZATION_VERSION_MISMATCH, "SERIALIZATION version mismatch" },
  { Error::SERIALIZATION_INPUT_OVERRUN,  "SERIALIZATION input overrun" },

  { Error::FS_BAD_FILE_HANDLE,   "FS bad file handle" },
  { Error::FS_IO_ERROR,          "FS i/o error" },
  { Error::FS_FILE_NOT_FOUND,    "FS file not found" },
  { Error::FS_BAD_FILENAME,      "FS bad filename" },
  { Error::FS_PERMISSION_DENIED, "FS permission denied" },
  { Error::FS_INVALID_ARGUMENT,  "FS invalid argument" },
  { Error::FS_INVALID_CONFIG,    "FS invalid config value" },
  { Error::FS_EOF,               "FS end of file" },
  { Error::FS_PATH_NOT_FOUND,    "FS destination path" },

  { Error::RS_NOT_LOADED_RANGE,     "Ranger range id not loaded"},
  { Error::RS_DELETED_RANGE,        "Ranger range state deleted"},
  { Error::RS_NOT_READY,            "Ranger is not ready, lacks id"},

  { Error::BAD_SCHEMA,           "bad schema" },
  { Error::BAD_KEY,              "bad key" },

  { Error::SQL_PARSE_ERROR,           "SQL parse error" },
  { Error::SQL_BAD_LOAD_FILE_FORMAT,  "SQL bad load file format" },
  { Error::SQL_BAD_COMMAND,           "SQL bad command" },

  { Error::CLIENT_DATA_REMAINED,      "Data remained on client input" },


  { Error::BLOCK_COMPRESSOR_UNSUPPORTED_TYPE,
        "block compressor unsupported type" },
  { Error::BLOCK_COMPRESSOR_INVALID_ARG,
        "block compressor invalid arg" },
  { Error::BLOCK_COMPRESSOR_TRUNCATED,
        "block compressor block truncated" },
  { Error::BLOCK_COMPRESSOR_BAD_HEADER,
        "block compressor bad block header" },
  { Error::BLOCK_COMPRESSOR_BAD_MAGIC,
        "block compressor bad magic string" },
  { Error::BLOCK_COMPRESSOR_CHECKSUM_MISMATCH,
        "block compressor block checksum mismatch" },
        
  { Error::ENCODER_ENCODE,
        "encoder decode-error" },
  { Error::ENCODER_DECODE,
        "encoder encode-error" },
  { Error::BLOCK_COMPRESSOR_INIT_ERROR,
        "block compressor initialization error" },
        
  { Error::COMMAND_PARSE_ERROR,         "command parse error" },
  { Error::REQUEST_MALFORMED,           "malformed request" },
  { Error::BAD_SCAN_SPEC,               "bad scan specification"},
  { Error::NOT_IMPLEMENTED,             "not implemented"},
  { Error::VERSION_MISMATCH,            "version mismatch"},
  { Error::CANCELLED,                   "cancelled"},
  { Error::SCHEMA_PARSE_ERROR,          "schema parse error" },
  { Error::SYNTAX_ERROR,                "syntax error" },
  { Error::DOUBLE_UNGET,                  "double unget" },
  { Error::NO_RESPONSE,                   "no response" },
  { Error::NOT_ALLOWED,                   "not allowed" },
  { Error::INDUCED_FAILURE,               "induced failure" },
  { Error::SERVER_SHUTTING_DOWN,          "server shutting down" },
  { Error::ALREADY_EXISTS,                "cell already exists" },
  { Error::CHECKSUM_MISMATCH,             "checksum mismatch" },
  { Error::CLOSED,                        "closed" },
  { Error::DUPLICATE_RANGE,               "duplicate range" },
  { Error::BAD_FORMAT,                    "bad format" },
  { Error::INVALID_ARGUMENT,              "invalid argument" },
  { Error::INVALID_OPERATION,             "invalid operation" },
  { Error::UNSUPPORTED_OPERATION,         "unsupported operation" },
  { Error::NOTHING_TO_DO,                 "nothing to do" },
  { Error::INCOMPATIBLE_OPTIONS,          "incompatible options" },
  { Error::BAD_VALUE,                     "bad value" },
  { Error::SCHEMA_GENERATION_MISMATCH,    "schema generation mismatch" },
  { Error::INVALID_METHOD_IDENTIFIER,     "invalid method identifier" },
  { Error::SERVER_NOT_READY,              "server not ready" }

};

} // local namespace

const char* Error::get_text(const int& err) {
  const char * text;
  return (text = err <= SYS_END
            ? strerror(err)
            : (err <= FUTURE_END
                ? std::future_error(
                    std::future_errc(err - FUTURE_BEGIN)).what()
                : text_map[err])
          ) ? text : "ERROR NOT REGISTERED";
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
    return Exception(FUTURE_BEGIN + e.code().value(), msg, prev, e.what());
  
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
