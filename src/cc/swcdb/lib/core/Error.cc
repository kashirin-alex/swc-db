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
#include "Logger.h"

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
    { Error::REQUEST_TRUNCATED,           "request truncated" },
    { Error::RESPONSE_TRUNCATED,          "response truncated" },
    { Error::REQUEST_TIMEOUT,             "request timeout" },

    { Error::CONFIG_BAD_ARGUMENT,         "CONFIG bad argument(s)"},
    { Error::CONFIG_BAD_CFG_FILE,         "CONFIG bad cfg file"},
    { Error::CONFIG_GET_ERROR,            "CONFIG failed to get config value"},
    { Error::CONFIG_BAD_VALUE,            "CONFIG bad config value"},

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

    { Error::LOCAL_IO_ERROR,       "local i/o error" },
    { Error::BAD_SCHEMA,           "bad schema" },
    { Error::BAD_KEY,              "bad key" },

    { Error::HQL_PARSE_ERROR,           "HQL parse error" },
    { Error::HQL_BAD_LOAD_FILE_FORMAT,  "HQL bad load file format" },
    { Error::HQL_BAD_COMMAND,           "HQL bad command" },
    


    { Error::BLOCK_COMPRESSOR_UNSUPPORTED_TYPE,
        "HYPERTABLE block compressor unsupported type" },
    { Error::BLOCK_COMPRESSOR_INVALID_ARG,
        "HYPERTABLE block compressor invalid arg" },
    { Error::BLOCK_COMPRESSOR_TRUNCATED,
        "HYPERTABLE block compressor block truncated" },
    { Error::BLOCK_COMPRESSOR_BAD_HEADER,
        "HYPERTABLE block compressor bad block header" },
    { Error::BLOCK_COMPRESSOR_BAD_MAGIC,
        "HYPERTABLE block compressor bad magic string" },
    { Error::BLOCK_COMPRESSOR_CHECKSUM_MISMATCH,
        "HYPERTABLE block compressor block checksum mismatch" },
    { Error::BLOCK_COMPRESSOR_DEFLATE_ERROR,
        "HYPERTABLE block compressor deflate error" },
    { Error::BLOCK_COMPRESSOR_INFLATE_ERROR,
        "HYPERTABLE block compressor inflate error" },
    { Error::BLOCK_COMPRESSOR_INIT_ERROR,
        "HYPERTABLE block compressor initialization error" },
        
    { Error::COMMAND_PARSE_ERROR,         "HYPERTABLE command parse error" },
    { Error::CONNECT_ERROR_MASTER,        "HYPERTABLE Master connect error" },
    { Error::CONNECT_ERROR_HYPERSPACE,
        "HYPERTABLE Hyperspace client connect error" },
    { Error::TOO_MANY_COLUMNS,            "HYPERTABLE too many columns" },
    { Error::BAD_DOMAIN_NAME,             "HYPERTABLE bad domain name" },
    { Error::MALFORMED_REQUEST,           "HYPERTABLE malformed request" },
    { Error::BAD_MEMORY_ALLOCATION,       "HYPERTABLE bad memory allocation"},
    { Error::BAD_SCAN_SPEC,               "HYPERTABLE bad scan specification"},
    { Error::NOT_IMPLEMENTED,             "HYPERTABLE not implemented"},
    { Error::VERSION_MISMATCH,            "HYPERTABLE version mismatch"},
    { Error::CANCELLED,                   "HYPERTABLE cancelled"},
    { Error::SCHEMA_PARSE_ERROR,          "HYPERTABLE schema parse error" },
    { Error::SYNTAX_ERROR,                "HYPERTABLE syntax error" },
    { Error::DOUBLE_UNGET,                  "HYPERTABLE double unget" },
    { Error::EMPTY_BLOOMFILTER,             "HYPERTABLE empty bloom filter" },
    { Error::BLOOMFILTER_CHECKSUM_MISMATCH, "HYPERTABLE bloom filter checksum mismatch" },
    { Error::NAME_ALREADY_IN_USE,           "HYPERTABLE name already in use" },
    { Error::NAMESPACE_DOES_NOT_EXIST,      "HYPERTABLE namespace does not exist" },
    { Error::BAD_NAMESPACE,                 "HYPERTABLE bad namespace" },
    { Error::NAMESPACE_EXISTS,              "HYPERTABLE namespace exists" },
    { Error::NO_RESPONSE,                   "HYPERTABLE no response" },
    { Error::NOT_ALLOWED,                   "HYPERTABLE not allowed" },
    { Error::INDUCED_FAILURE,               "HYPERTABLE induced failure" },
    { Error::SERVER_SHUTTING_DOWN,          "HYPERTABLE server shutting down" },
    { Error::LOCATION_UNASSIGNED,           "HYPERTABLE location unassigned" },
    { Error::ALREADY_EXISTS,                "HYPERTABLE cell already exists" },
    { Error::CHECKSUM_MISMATCH,             "HYPERTABLE checksum mismatch" },
    { Error::CLOSED,                        "HYPERTABLE closed" },
    { Error::RANGESERVER_NOT_FOUND,         "HYPERTABLE RangeServer not found" },
    { Error::CONNECTION_NOT_INITIALIZED,    "HYPERTABLE connection not initialized" },
    { Error::DUPLICATE_RANGE,               "HYPERTABLE duplicate range" },
    { Error::INVALID_PSEUDO_TABLE_NAME,     "HYPERTABLE invalid pseudo-table name" },
    { Error::BAD_FORMAT,                    "HYPERTABLE bad format" },
    { Error::INVALID_ARGUMENT,              "HYPERTABLE invalid argument" },
    { Error::INVALID_OPERATION,             "HYPERTABLE invalid operation" },
    { Error::UNSUPPORTED_OPERATION,         "HYPERTABLE unsupported operation" },
    { Error::COLUMN_FAMILY_NOT_FOUND,       "HYPERTABLE column family not found" },
    { Error::NOTHING_TO_DO,                 "HYPERTABLE nothing to do" },
    { Error::INCOMPATIBLE_OPTIONS,          "HYPERTABLE incompatible options" },
    { Error::BAD_VALUE,                     "HYPERTABLE bad value" },
    { Error::SCHEMA_GENERATION_MISMATCH,    "HYPERTABLE schema generation mismatch" },
    { Error::INVALID_METHOD_IDENTIFIER,     "HYPERTABLE invalid method identifier" },
    { Error::SERVER_NOT_READY,              "HYPERTABLE server not ready" },

    { Error::MASTER_TABLE_EXISTS,         "MASTER table exists" },
    { Error::MASTER_BAD_SCHEMA,           "MASTER bad schema" },
    { Error::MASTER_NOT_RUNNING,          "MASTER not running" },
    { Error::MASTER_NO_RANGESERVERS,      "MASTER no range servers" },
    { Error::MASTER_FILE_NOT_LOCKED,      "MASTER file not locked" },
    { Error::MASTER_RANGESERVER_ALREADY_REGISTERED,
        "MASTER range server with same location already registered" },
    { Error::MASTER_BAD_COLUMN_FAMILY,    "MASTER bad column family" },
    { Error::MASTER_SCHEMA_GENERATION_MISMATCH,
        "Master schema generation mismatch" },
    { Error::MASTER_LOCATION_ALREADY_ASSIGNED,
      "MASTER location already assigned" },
    { Error::MASTER_LOCATION_INVALID, "MASTER location invalid" },
    { Error::MASTER_OPERATION_IN_PROGRESS, "MASTER operation in progress" },
    { Error::MASTER_RANGESERVER_IN_RECOVERY, "MASTER RangeServer in recovery" },
    { Error::MASTER_BALANCE_PREVENTED, "MASTER balance operation prevented" },

    { Error::RANGESERVER_GENERATION_MISMATCH,
        "RANGE SERVER generation mismatch" },
    { Error::RANGESERVER_RANGE_ALREADY_LOADED,
        "RANGE SERVER range already loaded" },
    { Error::RANGESERVER_RANGE_MISMATCH,       "RANGE SERVER range mismatch" },
    { Error::RANGESERVER_NONEXISTENT_RANGE,
        "RANGE SERVER non-existent range" },
    { Error::RANGESERVER_OUT_OF_RANGE,         "RANGE SERVER out of range" },
    { Error::RANGESERVER_RANGE_NOT_FOUND,      "RANGE SERVER range not found" },
    { Error::RANGESERVER_INVALID_SCANNER_ID,
        "RANGE SERVER invalid scanner id" },
    { Error::RANGESERVER_SCHEMA_PARSE_ERROR,
        "RANGE SERVER schema parse error" },
    { Error::RANGESERVER_SCHEMA_INVALID_CFID,
        "RANGE SERVER invalid column family id" },
    { Error::RANGESERVER_INVALID_COLUMNFAMILY,
        "RANGE SERVER invalid column family" },
    { Error::RANGESERVER_TRUNCATED_COMMIT_LOG,
        "RANGE SERVER truncated commit log" },
    { Error::RANGESERVER_NO_METADATA_FOR_RANGE,
        "RANGE SERVER no metadata for range" },
    { Error::RANGESERVER_SHUTTING_DOWN,        "RANGE SERVER shutting down" },
    { Error::RANGESERVER_CORRUPT_COMMIT_LOG,
        "RANGE SERVER corrupt commit log" },
    { Error::RANGESERVER_UNAVAILABLE,          "RANGE SERVER unavailable" },
    { Error::RANGESERVER_REVISION_ORDER_ERROR,
        "RANGE SERVER supplied revision is not strictly increasing" },
    { Error::RANGESERVER_ROW_OVERFLOW,         "RANGE SERVER row overflow" },
    { Error::RANGESERVER_BAD_SCAN_SPEC,
        "RANGE SERVER bad scan specification" },
    { Error::RANGESERVER_CLOCK_SKEW,
        "RANGE SERVER clock skew detected" },
    { Error::RANGESERVER_BAD_CELLSTORE_FILENAME,
        "RANGE SERVER bad CellStore filename" },
    { Error::RANGESERVER_CORRUPT_CELLSTORE,
        "RANGE SERVER corrupt CellStore" },
    { Error::RANGESERVER_TABLE_DROPPED, "RANGE SERVER table dropped" },
    { Error::RANGESERVER_UNEXPECTED_TABLE_ID, "RANGE SERVER unexpected table ID" },
    { Error::RANGESERVER_RANGE_BUSY, "RANGE SERVER range busy" },
    { Error::RANGESERVER_BAD_CELL_INTERVAL, "RANGE SERVER bad cell interval" },
    { Error::RANGESERVER_SHORT_CELLSTORE_READ, "RANGE SERVER short cellstore read" },
    { Error::RANGESERVER_RANGE_NOT_ACTIVE, "RANGE SERVER range no longer active" },
    { Error::RANGESERVER_FRAGMENT_ALREADY_PROCESSED,
        "RANGE SERVER fragment already processed"},
    { Error::RANGESERVER_RECOVERY_PLAN_GENERATION_MISMATCH,
        "RANGE SERVER recovery plan generation mismatch"},
    { Error::RANGESERVER_PHANTOM_RANGE_MAP_NOT_FOUND,
      "RANGE SERVER phantom range map not found"},
    { Error::RANGESERVER_RANGES_ALREADY_LIVE,
      "RANGE SERVER ranges already live"},
    { Error::RANGESERVER_RANGE_NOT_YET_ACKNOWLEDGED,
      "RANGE SERVER range not yet acknowledged"},
    { Error::RANGESERVER_SERVER_IN_READONLY_MODE,
      "RANGE SERVER server in readonly mode"},
    { Error::RANGESERVER_RANGE_NOT_YET_RELINQUISHED,
      "RANGE SERVER range not yet relinquished"},

    { Error::THRIFTBROKER_BAD_SCANNER_ID, "THRIFT BROKER bad scanner id" },
    { Error::THRIFTBROKER_BAD_MUTATOR_ID, "THRIFT BROKER bad mutator id" },
    { Error::THRIFTBROKER_BAD_NAMESPACE_ID, "THRIFT BROKER bad namespace id" },
    { Error::THRIFTBROKER_BAD_FUTURE_ID,    "THRIFT BROKER bad future id" },
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
  const char *text = text_map[error];
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