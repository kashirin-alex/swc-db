/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/db/client/sql/ColumnList.h"


namespace SWC { namespace client { namespace SQL {

ColumnList::ColumnList(const Clients::Ptr& clients, const std::string& sql,
                       DB::SchemasVec& schemas,
                       std::string& message)
                      : Reader(sql, message),
                        clients(clients), schemas(schemas) {
}

int ColumnList::parse_list_columns(const char* expect_cmd,
                                   uint8_t& output_flags) {
  _parse_list_columns(expect_cmd);
  if(err)
    return err;
  bool any = true;
  while(any && remain && !err) {
    if(found_space())
      continue;
    if((any = found_token("OUTPUT_ONLY_CID", 15))) {
      output_flags |= ColumnOutputFlag::ONLY_CID;
      continue;
    }
  }
  if(!err)
    parse_list_columns();
  return err;
}

int ColumnList::parse_list_columns(const char* expect_cmd) {
  _parse_list_columns(expect_cmd);
  if(!err)
    parse_list_columns();
  return err;
}

void ColumnList::_parse_list_columns(const char* expect_cmd) {
  bool token_cmd = false;
  bool token_typ = false;

  while(remain && !err) {
    if(found_space())
      continue;

    if(!token_cmd && (found_token("get", 3) || found_token("list", 4) ||
                      found_token("compact", 7))) {
      token_cmd = true;
      continue;
    }
    if(!token_cmd) {
      expect_token(expect_cmd, strlen(expect_cmd), token_cmd);
      break;
    }
    if(!token_typ && (found_token("columns", 7) || found_token("column", 6) ||
                      found_token("schemas", 7) || found_token("schema", 6))) {
      token_typ = true;
      continue;
    }
    if(token_typ)
      break;
    expect_token("columns", 7, token_typ);
    break;
  }
}

void ColumnList::parse_list_columns() {
  bool bracket = false;
  char stop[3];
  stop[0] = ',';
  stop[1] = ' ';
  stop[2] = 0;

  while(remain && !err) {
    if(found_space())
      continue;

    if(found_char('[')) {
      stop[1] = ']';
      bracket = true;
      continue;
    } else if(found_char('(')) {
      stop[1] = ')';
      bracket = true;
      continue;
    }

    read_columns(schemas, stop);

    if(bracket)
      expect_token(&stop[1], 1, bracket);
    break;
  }
}

void ColumnList::read_columns(DB::SchemasVec& cols, const char* stop) {
  std::string col_name;
  while(remain && !err) {
    if(found_char(',') || found_space())
      continue;

    if(found_token("tags", 4)) {
      read_column_tags(patterns.tags);
      continue;
    }

    read_column(stop, col_name, patterns.names);
    if(!col_name.empty()) {
      cols.push_back(get_schema(clients, col_name));
      col_name.clear();
    } else if(remain && !err && stop[1] != ' ' && is_char(&stop[1])) {
      break;
    }
  }
}


}}} // SWC::client:SQL namespace
