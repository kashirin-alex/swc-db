/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_SpecsBuilderSql_h
#define swcdb_db_Cells_SpecsBuilderSql_h

#include "swcdb/db/Cells/SpecsScan.h"

namespace SWC { namespace DB { namespace Specs {

namespace Builder {

class SqlToSpecs final {
  
  static constexpr const char*    TOKEN_SELECT = "select";
  static const uint8_t  LEN_SELECT = 6;
  static constexpr const char*    TOKEN_WHERE = "where";
  static const uint8_t  LEN_WHERE = 5;
  static constexpr const char*    TOKEN_COL = "col";
  static const uint8_t  LEN_COL = 3;
  static constexpr const char*    TOKEN_CELLS = "cells";
  static const uint8_t  LEN_CELLS = 5;
  static constexpr const char*    TOKEN_AND = "and";
  static const uint8_t  LEN_AND = 3;
  
  public:
  SqlToSpecs(const std::string& sql, 
              Specs::Scan& specs, std::string& message)
            : sql(sql), specs(specs), message(message),
              ptr(sql.data()), remain(sql.length()+1), 
              err(Error::OK) {
  }

  const int parse() {

    bool token_cmd = false;
    bool token_where = false;

    while(remain && !err && (!token_cmd || !token_where)) {
      if(!token_cmd) {
        expect_token(TOKEN_SELECT, LEN_SELECT, token_cmd);
        continue;
      }
      if(!token_where) {
        expect_token(TOKEN_WHERE, LEN_WHERE, token_where);
        continue;
      }
      remain--;
      ptr++;
    }

    if(remain && !err)
      read_columns_intervals();

    if(remain && !err)
      read_flags();
    
    std::cout << "\n SQL='" << sql << "'"
              << "\n specs: " << specs.to_string() << "\n";    
    return err;
  }

  ~SqlToSpecs() {}

  private:
  
  void expect_token(const char* token, uint8_t token_len, bool& found) {
    if(remain >= token_len) {
      if(is_space())
        return;

      if(strncasecmp(ptr, token, token_len) == 0) {
        ptr += token_len;
        remain -= token_len;
        found = true;
        return;
      }
    }
    error_msg(Error::SQL_PARSE_ERROR, "missing '"+std::string(token)+"'");
  }

  const bool token_found(const char* token, uint8_t token_len) {
    if(remain >= token_len && strncasecmp(ptr, token, token_len) == 0) {
      ptr += token_len;
      remain -= token_len;
      return true;
    }
    return false;
  }
  
  const bool is_space() {
    if(*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
      ptr++;
      remain--;
      return true;
    }
    return false;
  }
  

  void read_columns_intervals() {
    bool token_col = false;
    bool bracket_round = false;
    bool eq = false;
    bool col_names_set = false;
    bool processed = false;
    bool possible_and = false;
    
    std::string col_name;
    std::vector<int64_t> cols;

    while(remain && !err) {

      if(possible_and) {        
        if(is_space())
          continue;
        if(token_found(TOKEN_AND, LEN_AND))
          possible_and = false;
        else {
          read_flags(); // to specs.flags 
          // apply global-scope flags if no columns.cells.flag set
          break;
        }
      }

      if(!token_col) {
        expect_token(TOKEN_COL, LEN_COL, token_col);
        continue;
      }

      if(token_col && !col_names_set) {
        //"col(, 1 2, 3 4 ,)"  = >> ["12","34"]
        if(is_space())
          continue;
        
        if(!bracket_round) {
          expect_token("(", 1, bracket_round);
          continue;
        }
        
        if(token_found(",", 1)) { 
          if(!col_name.empty()) {
            add_column(col_name);
            cols.push_back(specs.columns.back()->cid);
            col_name.clear();
          }
          continue;
        } 

        if(token_found(")", 1)) {
          if(col_name.empty() && cols.empty()) {
            error_msg(Error::SQL_PARSE_ERROR, "missing col 'name'");
            break;
          }
          add_column(col_name);
          cols.push_back(specs.columns.back()->cid);
          col_name.clear();
          bracket_round = false;
          col_names_set = true;
          continue;

        } else if(remain <= 1) {
          error_msg(Error::SQL_PARSE_ERROR, "missing ')'");
          break;
        }
        
        col_name += *ptr;
        ptr++;
        remain--;
        continue;
      }
      
      if(col_names_set) {

        if(!processed) {
          if(!eq) {
            expect_token("=", 1, eq);
            continue;
          }
          if(!bracket_round) {
            expect_token("(", 1, bracket_round);
            continue;
          }

          read_cells_intervals(cols);
          cols.clear();

          bracket_round = false;
          eq = false;
          processed = true;
          continue;
        }
        
        if(!bracket_round) {
          expect_token(")", 1, bracket_round);
          continue;
        }

        col_names_set = false;
        bracket_round = false;
        processed = false;
        token_col = false;
        possible_and = true;
      }
      
      remain--;
      ptr++;
    }

  }

  void add_column(const std::string& col) {
    int64_t cid = 0;
    if(std::find_if(col.begin(), col.end(), 
        [](unsigned char c){ return !std::isdigit(c); } ) != col.end()){
      cid = 2;  // cid = col_name = 
    } else {
      cid = std::stoll(col);
    }
    
    for(auto& col : specs.columns) {
      if(cid == col->cid) 
        return;
    }
    specs.columns.push_back(Column::make_ptr(cid, {}));
  }

  void read_cells_intervals(const std::vector<int64_t>& cols) {

    bool token_cells = false;
    bool bracket_round = false;
    bool eq = false;
    bool processed = false;
    bool possible_and = false;

    while(remain && !err) {
      if(possible_and) {        
        if(is_space())
          continue;
        if(token_found(TOKEN_AND, LEN_AND))
          possible_and = false;
        else
          break;
      }
      if(!token_cells) {
        expect_token(TOKEN_CELLS, LEN_CELLS, token_cells);
        continue;
      }
      
      if(!processed) {

        if(!eq) {
          expect_token("=", 1, eq);
          continue;
        }
        if(!bracket_round) {
          expect_token("(", 1, bracket_round);
          continue;
        }
        // cells_interval();
        auto spec = Interval::make_ptr();
        read_flags();
        for(auto& col : specs.columns) {
          for(auto cid : cols) {
            if(col->cid == cid)
              col->intervals.push_back(spec); // ?cpy
          }
        }

        bracket_round = false;
        eq = false;
        processed = true;
        continue;
      }
        
      if(!bracket_round) {
        expect_token(")", 1, bracket_round);
        continue;
      }
      bracket_round = false;
      processed = false;
      token_cells = false;
      possible_and = true;
    }

  }

  void read_flags() {

  }

  void error_msg(int error, const std::string& msg) {
    err = error;
    auto at = sql.length() - remain + 5;
    message.append("error=");
    message.append(std::to_string(err));
    message.append("(");
    message.append(Error::get_text(err));
    message.append(")\n");
  
    message.append(" SQL=");
    message.append(sql);
    message.append("\n");
    message.insert(message.length(), at, ' ');
    message.append("^");
    message.append("\n");
    message.insert(message.end(), at, ' ');
    message.append(msg);
    message.append("\n");
  }

  const std::string&  sql;
  Specs::Scan&        specs;
  std::string&        message;
  const char*         ptr;
  uint32_t            remain;
  int                 err;
};


void parse_sql_select(int& err, const std::string& sql, 
                      Specs::Scan& specs, std::string& message) {
  SqlToSpecs parser(sql, specs, message);
  err = parser.parse();
}

/*
/*

OK, select;
BAD, select fddsf;
BAD, select where;
OK, select where col(name1,name2,);
 
  col(ColNameA1) = ( 
    range >= ('1-') 
    and 
    (>='1-') <= key = (<='1-1-',="1") 
    and 
    value = "Value-Data-1" 
    and 
    timestamp > "2010/05/29" 
    limit=10 limit_by="KEYS"
  )
 and
  col(ColNameB1, ColNameB2) = ( 
    cells = (
      (>='2-') <= key = (<='2-2-',"1") 
      and 
      value = "Value-Data-2" 
      and 
      timestamp > "2010/05/29" 
    )
    and
    cells = (
      keys = (<='21-',"1") 
      and 
      timestamp > "2010/05/29"
    )
  )
 max_versions=1;

  SQL_PARSE_ERROR                              = 0x00060001,
  SQL_BAD_LOAD_FILE_FORMAT                     = 0x00060002,
  SQL_BAD_COMMAND                              = 0x00060003
*/


}

}}} // SWC::DB:Specs namespace

#endif //swcdb_db_Cells_SpecsBuilderSql_h