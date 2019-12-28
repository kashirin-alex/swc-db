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
  
  static constexpr const char*    TOKEN_RANGE = "range";
  static const uint8_t  LEN_RANGE = 5;
  static constexpr const char*    TOKEN_KEY = "key";
  static const uint8_t  LEN_KEY = 3;
  static constexpr const char*    TOKEN_VALUE = "value";
  static const uint8_t  LEN_VALUE = 5;
  static constexpr const char*    TOKEN_TIMESTAMP = "timestamp";
  static const uint8_t  LEN_TIMESTAMP = 9;
  static constexpr const char*    TOKEN_OFFSET_KEY = "offset_key";
  static const uint8_t  LEN_OFFSET_KEY = 10;
  static constexpr const char*    TOKEN_OFFSET_REV = "offset_rev";
  static const uint8_t  LEN_OFFSET_REV = 10;

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
            cols.push_back(add_column(col_name));
            col_name.clear();
          }
          continue;
        } 

        if(found_char(')')) {
          if(col_name.empty() && cols.empty()) {
            error_msg(Error::SQL_PARSE_ERROR, "missing col 'name'");
            break;
          }
          cols.push_back(add_column(col_name));
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

  int64_t add_column(const std::string& col) {
    int64_t cid = 0;
    if(std::find_if(col.begin(), col.end(), 
        [](unsigned char c){ return !std::isdigit(c); } ) != col.end()){
      cid = 2;  // cid = col_name = 
    } else {
      cid = std::stoll(col);
    }
    
    for(auto& col : specs.columns) {
      if(cid == col->cid) 
        return cid;
    }
    specs.columns.push_back(Column::make_ptr(cid, {}));
    return cid;
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
        read_cells_interval(*spec.get());
        
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
  
  void read_cells_interval(Interval& spec) {

    uint32_t escape = 0;
    bool quote_1 = false;
    bool quote_2 = false;
    bool is_quoted = false;
    bool possible_and = false;
    bool found_any = false;
    bool flw_and = false;
    uint32_t base_remain = remain;
    const char* base_rptr = ptr;
    
    while(remain && !err) {

      if(possible_and) {        
        found_any = true;
        if(found_space())
          continue;
        if(found_token(TOKEN_AND, LEN_AND)) {
          possible_and = false;
          flw_and = true;
        } else
          break;
      }
      
      if(!escape && found_char('\\')) {
        escape = remain;
        continue;
      } else if(escape && escape != remain)
        escape = 0;

      if(!escape) {
        if((found_quote_double(quote_1) || found_quote_single(quote_2))) {
          is_quoted = true;
          continue;
        } // if not in square brackets
        if(found_space())
          continue;
      }

      if(found_token(TOKEN_RANGE, LEN_RANGE)) {        
        read_range(spec.range_begin, spec.range_end, flw_and);
        possible_and = true;
        continue;
      }

      if(found_token(TOKEN_KEY, LEN_KEY)) {
        read_key(spec.key_start, spec.key_finish, flw_and);
        possible_and = true;
        continue;
      }

      if(found_token(TOKEN_TIMESTAMP, LEN_TIMESTAMP)) {
        read_timestamp(spec.ts_start, spec.ts_finish, flw_and);
        possible_and = true;
        continue;
      }

      if(found_token(TOKEN_VALUE, LEN_VALUE)) {
        read_value(spec.value);
        possible_and = true;
        continue;
      }

      if(found_token(TOKEN_OFFSET_KEY, LEN_OFFSET_KEY)) {
        expect_eq();
        read_key(spec.offset_key);
        possible_and = true;
        continue;
      }

      if(found_token(TOKEN_OFFSET_REV, LEN_OFFSET_REV)) {
        expect_eq();
        std::string buf;
        read(buf);
        if(err) 
          return;
        spec.offset_rev = Time::parse_ns(err, buf);
        if(err) {
          error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
          return;
        }
        possible_and = true; 
        continue;
      }
      ptr++;
      remain--;
    }

    if(!found_any) {        
      remain = base_remain;
      ptr = base_rptr;
    }
    read_flags(spec.flags);
  }

  
  void expect_comparator(Condition::Comp& comp) {
    while(remain) {
      if(found_space())
        continue;
      if((comp = Condition::from(&ptr, &remain)) != Condition::NONE)
        return;
      break;
    }
    error_msg(Error::SQL_PARSE_ERROR, "missing 'comparator'");
  }

  void expect_eq() {
    bool eq = false;
    while(remain && !err) {
      if(found_space())
        continue;
      if(!eq) { // break; (not space is eq)
        expect_token("=", 1, eq);
        return;
      }
    }
  }
  
  void read_range(Cell::Key& begin, Cell::Key& end, bool flw) {
    uint32_t base_remain = remain;
    const char* base_ptr = ptr;

    Condition::Comp comp_right;
    expect_comparator(comp_right);
    if(comp_right != Condition::GE && comp_right != Condition::LE) {
      error_msg(
        Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed GE|LE");
      return;
    }

    if(comp_right == Condition::GE)
      read_key(begin);
    else 
      read_key(end);
    if(err) 
      return;

    uint32_t mark_remain = remain;
    const char* mark_ptr = ptr;

    uint32_t remain_start = sql.length()+1;

    ptr = base_ptr - LEN_RANGE;
    remain = 0;
    while(remain++ < remain_start) {
      ptr--;
      if(flw) {
        if(found_token(TOKEN_AND, LEN_AND))
          break;
      } else if(found_char('('))
        break;
    }
    
    while(remain && !err && found_space());
    if(*ptr != '[') {
      remain = mark_remain;
      ptr = mark_ptr;
      return;
    }

    if(comp_right == Condition::GE)
      read_key(end);
    else 
      read_key(begin);
    if(err) {
      remain = mark_remain;
      ptr = mark_ptr;
      return;
    }
   
    Condition::Comp comp_left = Condition::NONE;
    expect_comparator(comp_left);
    remain += base_remain;
    if(comp_left != Condition::GE && comp_left != Condition::LE) {
      error_msg(
        Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed GE|LE");
      return;
    }
    comp_left = comp_left == Condition::GE ? Condition::LE : Condition::GE;
    if(comp_left == comp_right) {
      error_msg(Error::SQL_PARSE_ERROR, "bad comparators pair");
      return;
    }

    remain = mark_remain;
    ptr = mark_ptr;
  }

  void read_key(Key& start, Key& finish, bool flw) {
    uint32_t base_remain = remain;
    const char* base_ptr = ptr;

    Condition::Comp comp_right;
    expect_comparator(comp_right);
    if(comp_right != Condition::EQ && 
       comp_right != Condition::GE && comp_right != Condition::LE) {
      error_msg(
        Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed EQ|GE|LE");
      return;
    }

    if(comp_right == Condition::GE || comp_right == Condition::EQ)
      read_key(start);
    else 
      read_key(finish);

    if(err || comp_right == Condition::EQ) 
      return;

    uint32_t mark_remain = remain;
    const char* mark_ptr = ptr;

    uint32_t remain_start = sql.length()+1;

    ptr = base_ptr - LEN_KEY;
    remain = 0;
    while(remain++ < remain_start) {
      ptr--;
      if(flw) {
        if(found_token(TOKEN_AND, LEN_AND))
          break;
      } else if(found_char('('))
        break;
    }
    
    while(remain && !err && found_space());
    if(*ptr != '[') {
      remain = mark_remain;
      ptr = mark_ptr;
      return;
    }

    if(comp_right == Condition::GE)
      read_key(finish);
    else 
      read_key(start);
    if(err) {
      remain = mark_remain;
      ptr = mark_ptr;
      return;
    }
   
    Condition::Comp comp_left = Condition::NONE;
    expect_comparator(comp_left);
    remain += base_remain;
    if(comp_left != Condition::GE && comp_left != Condition::LE) {
      error_msg(Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed GE|LE");
      return;
    }
    comp_left = comp_left == Condition::GE ? Condition::LE : Condition::GE;
    if(comp_right == comp_left) {
      error_msg(Error::SQL_PARSE_ERROR, "bad comparators pair");
      return;
    }

    remain = mark_remain;
    ptr = mark_ptr;
  }

  void read_key(Cell::Key& key) {

    bool bracket_square = false;
    std::string fraction;
    
    while(remain && !err && found_space());
    expect_token("[", 1, bracket_square);

    while(remain && !err) {
      if(found_space())
        continue;

      read(fraction, ",]");
      key.add(fraction);
      fraction.clear();

      while(remain && !err && found_space());
      if(found_char(',')) 
        continue;
      break;
    }
    
    expect_token("]", 1, bracket_square);
  }

  void read_key(Key& key) {

    bool bracket_square = false;
    
    while(remain && !err && found_space());
    expect_token("[", 1, bracket_square);

    Condition::Comp comp = Condition::NONE;
    std::string fraction;
    while(remain && !err) {
      if(found_space())
        continue;

      expect_comparator(comp);
      read(fraction, ",]");
      key.add(fraction, comp);
      fraction.clear();

      while(remain && !err && found_space());
      if(found_char(',')) 
        continue;
      break;
    }
    
    expect_token("]", 1, bracket_square);
  }

  void read_value(Value& value) {
    Condition::Comp comp;
    expect_comparator(comp);
    std::string buf;
    read(buf, 0, comp == Condition::RE);
    if(!err)
      value.set((uint8_t*)buf.data(), buf.length(), comp, true);
  }
  
  void read_timestamp(Timestamp& start, Timestamp& finish, bool flw) {
    uint32_t base_remain = remain;
    const char* base_ptr = ptr;

    Condition::Comp comp_right;
    expect_comparator(comp_right);
    if(comp_right != Condition::EQ && 
       comp_right != Condition::GE && 
       comp_right != Condition::GT &&  
       comp_right != Condition::LE && 
       comp_right != Condition::LT) {
      error_msg(Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed EQ|GE|GT|LE|LT");
      return;
    }

    std::string buf;
    read(buf);
    if(err) 
      return;
    int64_t ts = Time::parse_ns(err, buf);
    if(err) {
      error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
      return;
    }

    if(comp_right == Condition::EQ || comp_right == Condition::GE || 
       comp_right == Condition::GT)
      start.set(ts, comp_right);
    else 
      finish.set(ts, comp_right);

    if(comp_right == Condition::EQ)
      return;

    uint32_t mark_remain = remain;
    const char* mark_ptr = ptr;

    uint32_t end = sql.length()+1;

    ptr = base_ptr - LEN_TIMESTAMP;
    remain = 0;
    while(remain++ < end) {
      ptr--;
      if(flw) {
        if(found_token(TOKEN_AND, LEN_AND))
          break;
      } else if(found_char('('))
        break;
    }
    
    buf.clear();
    read(buf);
    if(buf.empty()) {
      remain = mark_remain;
      ptr = mark_ptr;
      return;
    }
    if(err) 
      return;
    ts = Time::parse_ns(err, buf);
    if(err) {
      error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
      return;
    }

    Condition::Comp comp_left = Condition::NONE;
    expect_comparator(comp_left);
    remain += base_remain;
    if(comp_left != Condition::GE && 
       comp_left != Condition::GT && 
       comp_left != Condition::LE && 
       comp_left != Condition::LT) {
      error_msg(Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed GE|GT|LE|LT");
      return;
    }

    comp_left = comp_left == Condition::GE ? Condition::LE 
        : (comp_left == Condition::GT ? Condition::LT 
        : (comp_left == Condition::LE ? Condition::GE 
        : (comp_left == Condition::LT ? Condition::GT : comp_left)));

    if(comp_left == comp_right) {
      error_msg(Error::SQL_PARSE_ERROR, "bad comparators pair");
      return;
    }

    if(finish.was_set)
      start.set(ts, comp_left);
    else  
      finish.set(ts, comp_left);

    remain = mark_remain;
    ptr = mark_ptr;
  }

  void read(std::string& buf, const char* stop = 0,bool keep_escape=false) {
    uint32_t escape = 0;
    bool quote_1 = false;
    bool quote_2 = false;
    bool is_quoted = false;
    bool was_quoted = false;

    while(remain && !err) {
      if(!escape && *ptr == '\\') {
        escape = remain-1;
        if(!keep_escape) {
          ptr++;
          remain--;
          continue;
        }
      } else if(escape && escape != remain)
        escape = 0;

      if(!escape) {
        if(!is_quoted && buf.empty() && found_space())
            continue;
        if(((!is_quoted || quote_1) && found_quote_single(quote_1)) || 
           ((!is_quoted || quote_2) && found_quote_double(quote_2))) {
          is_quoted = quote_1 || quote_2;
          was_quoted = true;
          continue;
        }
        if((was_quoted && !is_quoted) || 
           (!was_quoted && (found_space() || is_char(stop))))
          break;
      }
      buf += *ptr;
      ptr++;
      remain--;
    }
  }

  const bool is_char(const char* stop) const {
    if(stop) do {
      if(*stop++ == *ptr)
        return true;
    } while(*stop);
    return false;
  }

  void read_flags(Flags& flags) {

    bool any = true;
    while(any && remain && !err) {
      if(found_space())
        continue;    

      if(any = found_token(TOKEN_LIMIT, LEN_LIMIT)) {
        expect_eq();
        read_uint32_t(flags.limit, flags.was_set);
        continue;
      }
      if(any = found_token(TOKEN_OFFSET, LEN_OFFSET)) {
        expect_eq();
        read_uint32_t(flags.offset, flags.was_set);
        continue;
      }
      if(any = found_token(TOKEN_MAX_VERS, LEN_MAX_VERS)) {
        expect_eq();
        read_uint32_t(flags.max_versions, flags.was_set);
        continue;
      }

      if(any = found_token(TOKEN_RETURN_DELETES, LEN_RETURN_DELETES)) {
        flags.return_deletes = flags.was_set = true;
        continue;
      }
      if(any = found_token(TOKEN_KEYS_ONLY, LEN_KEYS_ONLY)) {
        flags.keys_only = flags.was_set = true;
        continue;
      }
      // + LimitType limit_by, offset_by;
    }
  }


  void read_uint32_t(uint32_t& value, bool& was_set) {
    int64_t v;
    read_int64_t(v, was_set);
    if (v > INT32_MAX || v < INT32_MIN)
      error_msg(Error::SQL_PARSE_ERROR, " unsigned 32-bit integer out of range");
    else
      value = (uint32_t)v;
  }

  void read_int64_t(int64_t& value, bool& was_set) {
    std::string buf;
    read(buf);
    if(err) 
      return;
    try {
      value = std::stoll(buf);
      was_set = true;
    } catch(const std::exception& ex) {
      error_msg(Error::SQL_PARSE_ERROR, " signed 64-bit integer out of range");
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