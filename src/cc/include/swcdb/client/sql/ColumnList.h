/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_client_sql_ColumnList_h
#define swcdb_client_sql_ColumnList_h



namespace SWC { namespace client { namespace SQL {

class ColumnList : public Reader {

  public:
  ColumnList(const std::string& sql, std::vector<DB::Schema::Ptr>& schemas,
              std::string& message)
              : Reader(sql, message), schemas(schemas) {
  }

  const int parse_list_columns() {
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

      if(!token_cmd && (found_token("get", 3) || found_token("list", 4))) {   
        token_cmd = true;
        continue;
      } 
      if(!token_cmd) {
        expect_token("list", 4, token_cmd);
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

  ~ColumnList() {}

  private:
    std::vector<DB::Schema::Ptr>& schemas;
};


/*
use options:
get|list column|columns|schema|schemas name|ID;
get|list column|columns|schema|schemas [name|ID,name|ID];
get|list column|columns|schema|schemas "name|ID" "name|ID]";
*/

}}} // SWC::client:SQL namespace

#endif //swcdb_client_sql_ColumnList_h