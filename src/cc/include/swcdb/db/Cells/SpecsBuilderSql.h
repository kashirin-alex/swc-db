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
  
  static constexpr const char*    TOKEN_BOOL_FALSE = "false";
  static const uint8_t  LEN_BOOL_FALSE = 5;
  static constexpr const char*    TOKEN_BOOL_TRUE = "true";
  static const uint8_t  LEN_BOOL_TRUE = 4;
  
  static constexpr const char*    TOKEN_LIMIT = "limit";
  static const uint8_t  LEN_LIMIT = 5;
  static constexpr const char*    TOKEN_OFFSET = "offset";
  static const uint8_t  LEN_OFFSET = 6;
  static constexpr const char*    TOKEN_MAX_VERS = "max_versions";
  static const uint8_t  LEN_MAX_VERS = 12;
  static constexpr const char*    TOKEN_LIMIT_BY = "limit_by";
  static const uint8_t  LEN_LIMIT_BY = 8;
  static constexpr const char*    TOKEN_OFFSET_BY = "offset_by";
  static const uint8_t  LEN_OFFSET_BY = 9;
  static constexpr const char*    TOKEN_RETURN_DELETES = "return_deletes";
  static const uint8_t  LEN_RETURN_DELETES = 14;
  static constexpr const char*    TOKEN_KEYS_ONLY = "keys_only";
  static const uint8_t  LEN_KEYS_ONLY = 9;

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

    if(remain && !err) {
      read_flags(specs.flags);
      if(specs.flags.was_set) {
        // apply global-scope flags to cells_intervals
        for(auto& col : specs.columns) {
          for(auto& intval : col->intervals) {
            if(!intval->flags.was_set)
              intval->flags.copy(specs.flags);
          }
        }
      }
    }

    std::cout << "\n SQL='" << sql << "'"
              << "\n specs: " << specs.to_string() << "\n";    
    return err;
  }

  ~SqlToSpecs() {}

  private:
  
  void expect_token(const char* token, uint8_t token_len, bool& found) {
    if(remain >= token_len) {
      if(found_space())
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

  void expected_boolean(bool& value) {
    if(found_char('1') || found_token(TOKEN_BOOL_TRUE, LEN_BOOL_TRUE))
      value = true;
    else if(found_char('0') || found_token(TOKEN_BOOL_FALSE, LEN_BOOL_FALSE))
      value = false;
    else {
      ptr++;
      remain--;
      error_msg(Error::SQL_PARSE_ERROR, "missing 'bool'");
    }
  }

  void expect_digit() {
    if(remain >= 1) {
      if(std::isdigit((unsigned char)*ptr)) {
        ptr++;
        remain--;
        return;
      }
    }
    error_msg(Error::SQL_PARSE_ERROR, "missing 'digit'");
  }

  const bool found_token(const char* token, uint8_t token_len) {
    if(remain >= token_len && strncasecmp(ptr, token, token_len) == 0) {
      ptr += token_len;
      remain -= token_len;
      return true;
    }
    return false;
  }
  
  const bool found_char(const char c) {
    if(*ptr == c) {
      ptr++;
      remain--;
      return true;
    }
    return false;
  }
  
  const bool found_space() {
    return found_char(' ') || found_char('\t') 
        || found_char('\n') || found_char('\r');
  }

  const bool found_quote_single(bool& quote) {
    if(found_char('\'')) {
      quote = !quote;
      return true;
    }
    return false;
  }

  const bool found_quote_double(bool& quote) {
    if(found_char('"')) {
      quote = !quote;
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
        if(found_space())
          continue;
        if(found_token(TOKEN_AND, LEN_AND))
          possible_and = false;
        else
          break;
      }

      if(!token_col) {
        expect_token(TOKEN_COL, LEN_COL, token_col);
        continue;
      }

      if(token_col && !col_names_set) {
        //"col(, 1 2, 3 4 ,)"  = >> ["12","34"]
        if(found_space())
          continue;
        
        if(!bracket_round) {
          expect_token("(", 1, bracket_round);
          continue;
        }
        
        if(found_char(',')) { 
          if(!col_name.empty()) {
            add_column(col_name);
            cols.push_back(specs.columns.back()->cid);
            col_name.clear();
          }
          continue;
        } 

        if(found_char(')')) {
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
        if(found_space())
          continue;
        if(found_token(TOKEN_AND, LEN_AND))
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

        auto spec = Interval::make_ptr();
        // + cells_interval();
        read_flags(spec->flags);
        for(auto& col : specs.columns) {
          for(auto cid : cols) {
            if(col->cid == cid)
              col->intervals.push_back(Interval::make_ptr(spec));
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
  
  void read_flags(Flags& flags) {

    bool any = true;
    while(any && remain && !err) {
      if(found_space())
        continue;    

      if(any = found_token(TOKEN_LIMIT, LEN_LIMIT)) {
        read_uint32_t(flags.limit, flags.was_set);
        continue;
      }
      if(any = found_token(TOKEN_OFFSET, LEN_OFFSET)) {
        read_uint32_t(flags.offset, flags.was_set);
        continue;
      }
      if(any = found_token(TOKEN_MAX_VERS, LEN_MAX_VERS)) {
        read_uint32_t(flags.max_versions, flags.was_set);
        continue;
      }

      if(any = found_token(TOKEN_RETURN_DELETES, LEN_RETURN_DELETES)) {
        read_bool(flags.return_deletes, flags.was_set);
        continue;
      }
      if(any = found_token(TOKEN_KEYS_ONLY, LEN_KEYS_ONLY)) {
        read_bool(flags.keys_only, flags.was_set);
        continue;
      }
      // + LimitType limit_by, offset_by;
    }
  }

  void read_uint32_t(uint32_t& value, bool& was_set) {
    bool quote_1 = false;
    bool quote_2 = false;
    bool is_quoted = false;
    bool found = false;   
    //bool eq = false;
    std::string v;

    while(remain && !err) {

      /*
      if(!eq) {
        expect_token("=", 1, eq);
        continue;
      }
      */

      if(v.empty() && (found_space() || found_char('=')))
        continue;
      
      if(found_quote_double(quote_1) || found_quote_single(quote_2)) {
        is_quoted = true;
        continue;
      }

      if(!found && !v.empty() && !std::isdigit((unsigned char)*ptr)) {
        value = std::stoul(v);
        was_set = true;
        found = true;
      }

      if(found) {
        if(is_quoted && (quote_1 || quote_2)) {
          is_quoted = false;
          expect_token(quote_1 ? "\"" : "'", 1, is_quoted);
          if(is_quoted)
            break;
          continue;
        }
        break;
      }
      
      v += *ptr;
      expect_digit();
    }
  }

  void read_bool(bool& value, bool& was_set) {
    bool quote_1 = false;
    bool quote_2 = false;
    bool is_quoted = false;
    bool found = false;   

    while(remain && !err) {

      if(found_space() || found_char('='))
        continue;

      if(found_quote_double(quote_1) || found_quote_single(quote_2)) {
        is_quoted = true;
        continue;
      }

      if(!found) {
        expected_boolean(value);
        was_set = true;
        found = true;
      }

      if(is_quoted && (quote_1 || quote_2)) {
        is_quoted = false;
        expect_token(quote_1 ? "\"" : "'", 1, is_quoted);
        if(is_quoted)
          break;
        continue;
      }
      break;
    }
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