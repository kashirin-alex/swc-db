/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/sql/SQL.h"


namespace SWC { namespace client { namespace SQL {



Cmd recognize_cmd(int& err, const std::string& sql, std::string& message) {
  std::string cmd;
  int words = 1;
  bool word = true;
  for(auto chr : sql) {
    if(isspace(chr)) {
      if(word) {
        if(words == 2)
          break;
        if(!cmd.empty()) {
          ++words;
          cmd.append(" ");
        }
        word = false;
      }
    } else {
      word = true;
      cmd += std::tolower(chr);
    }
  }

  if(cmd.empty())
    return Cmd::UNKNOWN;

  if(Condition::str_eq(cmd.data(), "add", 3) ||
     Condition::str_eq(cmd.data(), "create", 6)) {
    return Cmd::CREATE_COLUMN;
  }
  if(Condition::str_eq(cmd.data(), "modify", 6) ||
     Condition::str_eq(cmd.data(), "change", 6) ||
     Condition::str_eq(cmd.data(), "update column", 13) ||
     Condition::str_eq(cmd.data(), "update schema", 13)) {
    return Cmd::MODIFY_COLUMN;
  }
  if(Condition::str_eq(cmd.data(), "delete", 6) ||
     Condition::str_eq(cmd.data(), "remove", 6)) {
    return Cmd::REMOVE_COLUMN;
  }
  if(Condition::str_eq(cmd.data(), "get", 3) ||
     Condition::str_eq(cmd.data(), "list", 4)) {
    return Cmd::GET_COLUMNS;
  }
  if(Condition::str_eq(cmd.data(), "compact", 7)) {
    return Cmd::COMPACT_COLUMNS;
  }
  if(Condition::str_eq(cmd.data(), "select", 6)) {
    return Cmd::SELECT;
  }
  if(Condition::str_eq(cmd.data(), "update", 6)) {
    return Cmd::UPDATE;
  }

  err = Error::SQL_PARSE_ERROR;
  message.append("Unrecognized Command begin with '");
  message.append(cmd);
  message += "'";
  return Cmd::UNKNOWN;
}

void parse_select(int& err, const Clients::Ptr& clients,
                  const std::string& sql,
                  DB::Specs::Scan& specs,
                  uint8_t& display_flags, std::string& message) {
  QuerySelect parser(clients, sql, specs, message);
  err = parser.parse_select();
  if(!err)
    parser.parse_display_flags(display_flags);
}

void parse_update(int& err, const std::string& sql,
                  const Query::Update::Handlers::BaseUnorderedMap::Ptr& hdlr,
                  uint8_t& display_flags, std::string& message) {
  QueryUpdate parser(sql, hdlr, message);
  err = parser.parse_update();
  if(!err)
    parser.parse_display_flags(display_flags);
}

void parse_list_columns(int& err, const Clients::Ptr& clients,
                        const std::string& sql,
                        DB::SchemasVec& schemas,
                        std::string& message, const char* expect_cmd) {
  ColumnList parser(clients, sql, schemas, message);
  err = parser.parse_list_columns(expect_cmd);
}

void parse_list_columns(int& err, const Clients::Ptr& clients,
                        const std::string& sql,
                        DB::SchemasVec& schemas,
                        Comm::Protocol::Mngr::Params::ColumnListReq& params,
                        std::string& message, const char* expect_cmd) {
  ColumnList parser(clients, sql, schemas, message);
  err = parser.parse_list_columns(expect_cmd);
  if(!parser.patterns.names.empty() ||
     parser.patterns.tags.comp != Condition::NONE)
    params.patterns = std::move(parser.patterns);
}

void parse_list_columns(int& err, const Clients::Ptr& clients,
                        const std::string& sql,
                        DB::SchemasVec& schemas,
                        Comm::Protocol::Mngr::Params::ColumnListReq& params,
                        uint8_t& output_flags,
                        std::string& message, const char* expect_cmd) {
  ColumnList parser(clients, sql, schemas, message);
  err = parser.parse_list_columns(expect_cmd, output_flags);
  if(!parser.patterns.names.empty() ||
     parser.patterns.tags.comp != Condition::NONE)
    params.patterns = std::move(parser.patterns);
}


void parse_column_schema(
        int& err, const std::string& sql,
        Comm::Protocol::Mngr::Params::ColumnMng::Function func,
        DB::Schema::Ptr& schema, std::string& message) {
  ColumnSchema parser(sql, schema, message);
  err = parser.parse(func);
}

void parse_column_schema(
        int& err, const std::string& sql,
        Comm::Protocol::Mngr::Params::ColumnMng::Function* func,
        DB::Schema::Ptr& schema, std::string& message) {
  ColumnSchema parser(sql, schema, message);
  err = parser.parse(func);
}

void parse_dump(int& err, const Clients::Ptr& clients,
                const std::string& sql,
                std::string& fs, std::string& filepath,
                uint64_t& split_size, std::string& ext, int& level,
                DB::Specs::Scan& specs,
                uint8_t& output_flags, uint8_t& display_flags,
                std::string& message) {
  QuerySelect parser(clients, sql, specs, message);
  err = parser.parse_dump(fs, filepath, split_size, ext, level);
  if(!err)
    parser.parse_output_flags(output_flags);
  if(!err)
    parser.parse_display_flags(display_flags);
}

void parse_load(int& err,
                const std::string& sql,
                const Query::Update::Handlers::BaseUnorderedMap::Ptr& hdlr,
                std::string& fs, std::string& filepath, cid_t& cid,
                uint8_t& display_flags, std::string& message) {
  QueryUpdate parser(sql, hdlr, message);
  err = parser.parse_load(fs, filepath, cid);
  if(!err)
    parser.parse_display_flags(display_flags);
}




}}} // SWC::client:SQL namespace
