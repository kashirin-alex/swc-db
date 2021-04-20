/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/db/client/sql/ColumnList.h"


namespace SWC { namespace client { namespace SQL {

ColumnList::ColumnList(const std::string& sql,
                       std::vector<DB::Schema::Ptr>& schemas,
                       std::string& message)
                      : Reader(sql, message), schemas(schemas) {
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

void ColumnList::read_columns(std::vector<DB::Schema::Ptr>& cols,
                              const char* stop) {
  std::string col_name;
  Condition::Comp comp;
  while(remain && !err) {
    if(found_char(',') || found_space())
      continue;

    found_comparator(comp = Condition::NONE, true);
    if(comp != Condition::NONE) {
      read(col_name, stop, comp == Condition::RE);
      if(col_name.empty()) {
        error_msg(
          Error::SQL_PARSE_ERROR,
          "expected column name(expression) after comparator"
        );
        break;
      }
      patterns.emplace_back(comp, col_name);

    } else {
      read(col_name, stop);
      if(col_name.empty())
        break;
      cols.push_back(get_schema(col_name));
    }
    col_name.clear();
  }
}


}}} // SWC::client:SQL namespace
