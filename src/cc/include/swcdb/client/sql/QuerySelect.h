/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_client_sql_QuerySelect_h
#define swcdb_client_sql_QuerySelect_h



namespace SWC { namespace client { namespace SQL {

namespace {
    
  static const char*    TOKEN_SELECT = "select";
  static const uint8_t  LEN_SELECT = 6;
  static const char*    TOKEN_WHERE = "where";
  static const uint8_t  LEN_WHERE = 5;
  static const char*    TOKEN_COL = "col";
  static const uint8_t  LEN_COL = 3;
  static const char*    TOKEN_CELLS = "cells";
  static const uint8_t  LEN_CELLS = 5;

  
  static const char*    TOKEN_RANGE = "range";
  static const uint8_t  LEN_RANGE = 5;
  static const char*    TOKEN_KEY = "key";
  static const uint8_t  LEN_KEY = 3;
  static const char*    TOKEN_VALUE = "value";
  static const uint8_t  LEN_VALUE = 5;
  static const char*    TOKEN_TIMESTAMP = "timestamp";
  static const uint8_t  LEN_TIMESTAMP = 9;
  static const char*    TOKEN_OFFSET_KEY = "offset_key";
  static const uint8_t  LEN_OFFSET_KEY = 10;
  static const char*    TOKEN_OFFSET_REV = "offset_rev";
  static const uint8_t  LEN_OFFSET_REV = 10;

  static const char*    TOKEN_LIMIT = "limit";
  static const uint8_t  LEN_LIMIT = 5;
  static const char*    TOKEN_OFFSET = "offset";
  static const uint8_t  LEN_OFFSET = 6;
  static const char*    TOKEN_MAX_VERS = "max_versions";
  static const uint8_t  LEN_MAX_VERS = 12;
  static const char*    TOKEN_LIMIT_BY = "limit_by";
  static const uint8_t  LEN_LIMIT_BY = 8;
  static const char*    TOKEN_OFFSET_BY = "offset_by";
  static const uint8_t  LEN_OFFSET_BY = 9;
  static const char*    TOKEN_RETURN_DELETES = "return_deletes";
  static const uint8_t  LEN_RETURN_DELETES = 14;
  static const char*    TOKEN_KEYS_ONLY = "keys_only";
  static const uint8_t  LEN_KEYS_ONLY = 9;

}

class QuerySelect : public Reader {

  public:
  QuerySelect(const std::string& sql, DB::Specs::Scan& specs, 
              std::string& message)
              : Reader(sql, message), specs(specs) {
  }

  const int parse_select() {

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
    return err;
  }

  void parse_display_flags(uint8_t& display_flags) {

    bool any = true;
    while(any && remain && !err) {
      if(found_space())
        continue;    

      if(any = found_token("DISPLAY_TIMESTAMP", 17)) {
        display_flags |= DB::DisplayFlag::TIMESTAMP;
        continue;
      }
      if(any = found_token("DISPLAY_DATETIME", 16)) {
        display_flags |= DB::DisplayFlag::DATETIME;
        continue;
      }
      if(any = found_token("DISPLAY_BINARY", 14)) {
        display_flags |= DB::DisplayFlag::BINARY;
        continue;
      }
      if(any = found_token("DISPLAY_SPECS", 13)) {
        display_flags |= DB::DisplayFlag::SPECS;
        continue;
      }
      if(any = found_token("DISPLAY_STATS", 13)) {
        display_flags |= DB::DisplayFlag::STATS;
        continue;
      }
    }
  }

  ~QuerySelect() {}

  private:

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
        
        if(found_char(','))
          continue;

        if(found_char(')')) {
          if(cols.empty()) {
            error_msg(Error::SQL_PARSE_ERROR, "missing col 'name'");
            break;
          }
          bracket_round = false;
          col_names_set = true;
          continue;

        } else if(remain <= 1) {
          error_msg(Error::SQL_PARSE_ERROR, "missing ')'");
          break;
        }

        read(col_name, ",)");
        cols.push_back(add_column(col_name));
        col_name.clear();
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
    int64_t cid = get_cid(col);
    for(auto& col : specs.columns) {
      if(cid == col->cid) 
        return cid;
    }
    specs.columns.push_back(DB::Specs::Column::make_ptr(cid, {}));
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

        auto spec = DB::Specs::Interval::make_ptr();
        read_cells_interval(*spec.get());
        
        for(auto& col : specs.columns) {
          for(auto cid : cols) {
            if(col->cid == cid)
              col->intervals.push_back(DB::Specs::Interval::make_ptr(spec));
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
  
  void read_cells_interval(DB::Specs::Interval& spec) {

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
        if(((!is_quoted || quote_1) && found_quote_single(quote_1)) || 
           ((!is_quoted || quote_2) && found_quote_double(quote_2))) {
          is_quoted = quote_1 || quote_2;
          continue;
        }
        if(found_space())
          continue;
      }
      
      if(is_quoted) {
        ptr++;
        remain--;
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
        Reader::read_key(spec.offset_key);
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
      
      if(*ptr == ')') 
        break;
      ptr++;
      remain--;

    }

    if(!found_any) {        
      remain = base_remain;
      ptr = base_rptr;
    }
    read_flags(spec.flags);
  }

  
  void read_range(DB::Cell::Key& begin, DB::Cell::Key& end, bool flw) {
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
      Reader::read_key(begin);
    else 
      Reader::read_key(end);
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
      Reader::read_key(end);
    else 
      Reader::read_key(begin);
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

  void read_key(DB::Specs::Key& start, DB::Specs::Key& finish, bool flw) {
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
      error_msg(
        Error::SQL_PARSE_ERROR, "unsupported 'comparator' allowed GE|LE");
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

  void read_key(DB::Specs::Key& key) {

    bool bracket_square = false;
    
    while(remain && !err && found_space());
    expect_token("[", 1, bracket_square);

    Condition::Comp comp = Condition::NONE;
    std::string fraction;
    while(remain && !err) {
      if(found_space())
        continue;
      
      found_comparator(comp);
      if(comp == Condition::NONE)
        comp = Condition::EQ;
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

  void read_value(DB::Specs::Value& value) {
    Condition::Comp comp;
    expect_comparator(comp);
    std::string buf;
    read(buf, 0, comp == Condition::RE);
    if(!err)
      value.set((uint8_t*)buf.data(), buf.length(), comp, true);
  }
  
  void read_timestamp(DB::Specs::Timestamp& start, 
                      DB::Specs::Timestamp& finish, bool flw) {
    uint32_t base_remain = remain;
    const char* base_ptr = ptr;

    Condition::Comp comp_right;
    expect_comparator(comp_right);
    if(comp_right != Condition::EQ && 
       comp_right != Condition::GE && 
       comp_right != Condition::GT &&  
       comp_right != Condition::LE && 
       comp_right != Condition::LT) {
      error_msg(
        Error::SQL_PARSE_ERROR, 
        "unsupported 'comparator' allowed EQ|GE|GT|LE|LT"
      );
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
      error_msg(
        Error::SQL_PARSE_ERROR, 
        "unsupported 'comparator' allowed GE|GT|LE|LT"
      );
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

  void read_flags(DB::Specs::Flags& flags) {

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

  DB::Specs::Scan&  specs;
};





/*
# select all cells(INSERT) from column ID or NAME
select where 
  col(NAME|ID,"NAME|ID",) = (
    cells=()
  )
;

# select cells on-interval  from columns 1 and 2
select where 
  col(1,2) = (
    cells=(
        ["a", "start", "of", "range", "fractions"]
      <= range <=
        ["an", "end", "of", "range", "fractions"]
      AND
      key = [="where",="fractions", ="equal", ="to"]
      AND
        "2019/12/28 06:51:27.857347289"
      <= timestamp <=
        "1577508687857347289"
      AND 
      OFFSET_KEY=["an", "offset", "key", "fractions", "to", "start", "from", "the", "scan"]
      AND 
      OFFSET_REV="1577508700000000000"
      AND 
      value re "\"aRegExp\""
      
      KEYS_ONLY
      RETURN_DELETES
      limit=1
      offset=0
    )
  )
;

# select cells on-interval-1  from columns 1 and 2 AND cells on-interval-2  from column "col-test-1"
select where 
  col(1,2) = (
    cells=(
        ["a", "start", "of", "range", "fractions"]
      <= range <=
        ["an", "end", "of", "range", "fractions"]
      AND
      key = ["where","fractions", "equal", "to"]
      AND
        "2019/12/28 06:51:27.857347289"
      <= timestamp <=
        "1577508687857347289"
      AND 
      OFFSET_KEY=["an", "offset", "key", "fractions", "to", "start", "from", "the", "scan"]
      AND 
      OFFSET_REV="1577508700000000000"
      AND 
      value re "\"aRegExp\""
      
      KEYS_ONLY
      RETURN_DELETES
      limit=1
      offset=0
    )
  )

  AND
  
  col("col-test-1") = (
    cells=(
        ["a", "start", "of", "range", "fractions"]
      <= range <=
        ["an", "end", "of", "range", "fractions"]
      AND
        [>="where",>="fractions", =^"start_with", >="this", >="", >="fractions", !="NOT_THIS", <="within"]
      <= key <=
        [<="where under and more", <="fractions incl.", =^"start_with", >="this", r"is it a \\re", >="fractions and above", <="UP TO 1000", <="within"]

      AND
        "2019/12/28 06:51:27.857347289"
      <= timestamp <=
        "1577508687857347289"
      AND 
      OFFSET_KEY=["an", "offset", "key", "fractions", "to", "start", "from", "the", "scan"]
      AND 
      OFFSET_REV="1577508700000000000"
      AND 
      value re "\"aRegExp\""
      
      KEYS_ONLY
      RETURN_DELETES
      limit=1
      offset=0
    )
  )
;


*/


}}} // SWC::client:SQL namespace

#endif //swcdb_client_sql_QuerySelect_h