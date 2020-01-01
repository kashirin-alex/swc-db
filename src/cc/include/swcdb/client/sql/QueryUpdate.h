/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_client_sql_QueryUpdate_h
#define swcdb_client_sql_QueryUpdate_h



namespace SWC { namespace client { namespace SQL {

namespace {
  static const char*    TOKEN_CELL = "cell";
  static const uint8_t  LEN_CELL = 4;
  static const char*    TOKEN_UPDATE = "update";
  static const uint8_t  LEN_UPDATE = 6;
}

class QueryUpdate : public Reader {

  public:
  QueryUpdate(const std::string& sql, 
              DB::Cells::MapMutable& columns, 
              DB::Cells::MapMutable& columns_onfraction,
              std::string& message)
              : Reader(sql, message), 
                columns(columns), columns_onfraction(columns_onfraction) {
  }

  const int parse_update() {

    bool token_cmd = false;
    while(remain && !err && !token_cmd) {
      if(!token_cmd) {
        expect_token(TOKEN_UPDATE, LEN_UPDATE, token_cmd);
        continue;
      }
    }

    if(remain && !err)
      read_cells();

    return err;
  }

  void parse_display_flags(uint8_t& display_flags) {

    bool any = true;
    while(any && remain && !err) {
      if(found_space())
        continue;    

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

  ~QueryUpdate() {}

  private:
  void read_cells() {
    bool possible_and = false;
    bool bracket = false;
    
    while(remain && !err) {
      if(found_space())
        continue;

      if(possible_and) {   
        if(found_token(",", 1)) {
          possible_and = false;
          continue;
        } else
          break;
      }

      expect_token(TOKEN_CELL, LEN_CELL, possible_and);

      if(remain && !err) {    

        while(remain && !err && found_space());
        expect_token("(", 1, bracket);
        if(err) 
          return;

        bool on_fraction;
        int64_t cid;
        DB::Cells::Cell cell;
        read_cell(cid, cell, on_fraction);

        while(remain && !err && found_space());
        expect_token(")", 1, bracket);
        if(err) 
          return;
        if(on_fraction)
          columns_onfraction.add(cid, cell);
        else 
          columns.add(cid, cell);
      }
    }
  }

  void read_cell(int64_t& cid, DB::Cells::Cell& cell, bool& on_fraction) {
    bool comma = false;
    
    read_flag(cell.flag, on_fraction);
    if(!err) 
      expect_comma(comma);
    if(err) 
      return;


    std::string col;
    read(col, ",");
    if(!err) 
      expect_comma(comma);
    if(err) 
      return;
    auto schema = get_schema(col);
    if(err) 
      return;
    cid = schema->cid;
    columns.create(schema);


    read_key(cell.key);

    comma = false;
    bool require_ts = cell.flag == DB::Cells::DELETE_VERSION;
    if(!err && require_ts) 
      expect_comma(comma);
    if(err) 
      return;

    while(remain && !err && found_space());
    if(err || (!comma && !found_char(','))) {
      cell.set_time_order_desc(true);
      return;
    }

    if(cell.flag == DB::Cells::INSERT) {
      bool time_order;
      while(remain && !err && found_space());
      if((time_order = found_token("DESC", 4)) || 
         !(time_order = found_token("ASC", 3)))
        cell.set_time_order_desc(true);
      if(time_order) {
        while(remain && !err && found_space());
        if(err || !found_char(','))
          return;
      }
    }

    std::string buf;
    read(buf, ",)");
    if(err) 
      return;
    if(!buf.empty()) 
      cell.set_timestamp(Time::parse_ns(err, buf));
    if(err || (require_ts && buf.empty())) {
      error_msg(Error::SQL_PARSE_ERROR, "bad datetime format");
      return;
    }
      
    while(remain && !err && found_space());
    if(err || !found_char(','))
      return;

    std::string value;
    read(value, ")");
    if(err) 
      return;
    cell.set_value(value, true);

  }

  void read_flag(uint8_t& flag, bool& on_fraction) {
    std::string buf;
    read(buf, ",");
    if(err) 
      return;

    if(buf.compare("1") == 0 || 
       strncasecmp(buf.data(), "INSERT", buf.length()) == 0) {
      flag = DB::Cells::INSERT;
      on_fraction = false;
    } else if(buf.compare("2") == 0 || 
       strncasecmp(buf.data(), "DELETE", buf.length()) == 0) {
      flag = DB::Cells::DELETE;
      on_fraction = false;
    } else if(buf.compare("3") == 0 || 
       strncasecmp(buf.data(), "DELETE_VERSION", buf.length()) == 0) {
      flag = DB::Cells::DELETE_VERSION;
      on_fraction = false;
    } else if(buf.compare("4") == 0 || 
       strncasecmp(buf.data(), "INSERT_FRACTION", buf.length()) == 0) {
      flag = DB::Cells::INSERT;
      on_fraction = true;
    } else if(buf.compare("5") == 0 || 
       strncasecmp(buf.data(), "DELETE_FRACTION", buf.length()) == 0) {
      flag = DB::Cells::DELETE;
      on_fraction = true;
    } else if(buf.compare("6") == 0 || 
       strncasecmp(buf.data(), "DELETE_FRACTION_VERSION", buf.length()) == 0) {
      flag = DB::Cells::DELETE_VERSION;
      on_fraction = true;
    } else {
      error_msg(Error::SQL_PARSE_ERROR, "unsupported cell flag='"+buf+"'");
    }
  }
  
  DB::Cells::MapMutable& columns;
  DB::Cells::MapMutable& columns_onfraction;
};









}}} // SWC::client:SQL namespace

#endif //swcdb_client_sql_QueryUpdate_h