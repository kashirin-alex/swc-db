/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/** @file
 * Error codes, Exception handling, error logging.
 *
 * This file contains all error codes used in Hypertable, the Exception
 * base class and macros for logging and error handling.
 */

#include "Compat.h"
#include "Error.h"

#include <iomanip>
#include <unordered_map>

using namespace SWC;

namespace {
  struct ErrorInfo {
    int          code;
    const char  *text;
  };

  ErrorInfo error_info[] = {
    { Error::UNPOSSIBLE,                  "But that's unpossible!" },
    { Error::EXTERNAL,                    "External error" },
    { Error::FAILED_EXPECTATION,          "failed expectation" },
    { Error::OK,                          "ok" },

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
    { Error::COLUMN_SCHEMA_NOT_DIFFERENT,   "Schemas expected to be different"},
    { Error::COLUMN_SCHEMA_MISSING,         "cid's Schema is missing"},
    { Error::COLUMN_MARKED_REMOVED,         "Column is being removed"},
    { Error::COLUMN_NOT_EXISTS,             "Column does not exist"},
    { Error::COLUMN_NOT_READY,              "Column is not ready"},

    { Error::RANGE_NOT_FOUND,               "No corresponding range"},

    { Error::SERIALIZATION_BAD_VINT,      "SERIALIZATION bad vint encoding" },
    { Error::SERIALIZATION_BAD_VSTR,      "SERIALIZATION bad vstr encoding" },
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

    { Error::IO_ERROR,              " i/o error" },
    { Error::BAD_SCHEMA,           "bad schema" },
    { Error::BAD_KEY,              "bad key" },

    { Error::SQL_PARSE_ERROR,           "HQL parse error" },
    { Error::SQL_BAD_LOAD_FILE_FORMAT,  "HQL bad load file format" },
    { Error::SQL_BAD_COMMAND,           "HQL bad command" },
    


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
    { Error::BAD_MEMORY_ALLOCATION,       "bad memory allocation"},
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
    { Error::SERVER_NOT_READY,              "server not ready" },

    { 0, 0 }
  };

  typedef std::unordered_map<int, const char *>  TextMap;

  TextMap &build_text_map() {
    TextMap *map = new TextMap();
    for (int i=0; error_info[i].text != 0; i++)
      (*map)[error_info[i].code] = error_info[i].text;
    return *map;
  }

  TextMap &text_map = build_text_map();

} // local namespace

const char *Error::get_text(int error) {

  const char *text = error < 2048? strerror(error) :text_map[error];
  if (text == 0)
    return "ERROR NOT REGISTERED";
  return text;
}

void Error::generate_html_error_code_documentation(std::ostream &out) {
  out << "<table border=\"1\" cellpadding=\"4\" cellspacing=\"1\" style=\"width: 720px; \">\n";
  out << "<thead><tr><th scope=\"col\">Code<br />(hexidecimal)</th>\n";
  out << "<th scope=\"col\">Code<br />(decimal)</th>\n";
  out << "<th scope=\"col\">Description</th></tr></thead><tbody>\n";

  for (size_t i=0; error_info[i].text; i++) {
    if (error_info[i].code >= 0)
      out << "<tr><td style=\"text-align: right; \"><code>0x" << std::hex << error_info[i].code << "</code></td>\n";
    else
      out << "<tr><td style=\"text-align: right; \"><code></code></td>\n";
    out << "<td style=\"text-align: right; \"><code>" << std::dec << error_info[i].code << "</code></td>\n";
    out << "<td>" << error_info[i].text << "</td></tr>\n";
  }
  out << "</tbody></table>\n" << std::flush;
}

namespace SWC {

  const char *relative_fname(const Exception &e) {
    if (e.file()) {
      const char *ptr = strstr(e.file(), "src/cc/");
      return ptr ? ptr : e.file();
    }
    return "";
  }

std::ostream &operator<<(std::ostream &out, const Exception &e) {
  out <<"SWC::Exception: "<< e.message() <<" - "
      << Error::get_text(e.code());

  if (e.line()) {
    out <<"\n\tat "<< e.func() <<" (";
    if (Logger::get()->show_line_numbers())
      out << e.file() <<':'<< e.line();
    else
      out << relative_fname(e);
    out <<')';
  }

  int prev_code = e.code();

  for (Exception *prev = e.prev; prev; prev = prev->prev) {
    out <<"\n\tat "<< (prev->func() ? prev->func() : "-") <<" (";
    if (Logger::get()->show_line_numbers())
      out << (prev->file() ? prev->file() : "-") <<':'<< prev->line();
    else
      out << relative_fname(*prev);
    out <<"): " << prev->message();

    if (prev->code() != prev_code) {
      out <<" - "<< Error::get_text(prev->code());
      prev_code = prev->code();
    }
  }
  return out;
}

std::ostream &
Exception::render_messages(std::ostream &out, const char *sep) const {
  out << message() <<" - "<< Error::get_text(m_error);

  for (Exception *p = prev; p; p = p->prev)
    out << sep << p->message();

  return out;
}

} 