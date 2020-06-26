/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/sql/ColumnList.h"


namespace SWC { namespace client { namespace SQL {

ColumnList::ColumnList(const std::string& sql, std::vector<DB::Schema::Ptr>& schemas,
                       std::string& message)
                      : Reader(sql, message), schemas(schemas) {
}

int ColumnList::parse_list_columns(const char* expect_cmd) {
  bool token_cmd = false;
  bool token_typ = false;
  bool bracket = false;
  char stop[3];
  stop[0] = ',';
  stop[1] = ' ';
  stop[2] = 0;

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
    if(!token_typ) {
      expect_token("columns", 7, token_typ);
      break;
    }

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
  return err;
}

ColumnList::~ColumnList() {}
  
void ColumnList::read_columns(std::vector<DB::Schema::Ptr>& cols, const char* stop) {
  std::string col_name;
  while(remain && !err) {
    if(found_char(',') || found_char(' '))
      continue;
    read(col_name, stop);
    if(col_name.empty())
      break;
    cols.push_back(get_schema(col_name));
    col_name.clear();
  }
}


}}} // SWC::client:SQL namespace
