/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/sql/QueryUpdate.h"
#include "swcdb/core/Time.h"


namespace SWC { namespace client { namespace SQL {

namespace {

  static const char     TOKEN_CELL[] = "cell";
  static const uint8_t  LEN_CELL = 4;
  static const char     TOKEN_UPDATE[] = "update";
  static const uint8_t  LEN_UPDATE = 6;
  static const char     TOKEN_LOAD[] = "load";
  static const uint8_t  LEN_LOAD = 4;
}

QueryUpdate::QueryUpdate(
        const std::string& a_sql,
        const Query::Update::Handlers::BaseUnorderedMap::Ptr& a_hdlr,
        std::string& a_message)
        : Reader(a_sql, a_message), hdlr(a_hdlr) {
}

int QueryUpdate::parse_update() {

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

int QueryUpdate::parse_load(std::string& fs, std::string& filepath,
                            cid_t& cid) {
  bool token = false;

  seek_space();
  expect_token(TOKEN_LOAD, LEN_LOAD, token);
  if(err)
    return err;

  seek_space();
  expect_token("from", 4, token);
  if(err)
    return err;

  bool any = true;
  while(any && remain && !err) {
    if(found_space())
      continue;

    if((any = found_token("fs", 2))) {
      expect_eq();
      if(err)
        break;
      read(fs);
      if(!err && fs.empty())
        error_msg(Error::SQL_PARSE_ERROR, "missing 'fs' value");
      continue;
    }

    if((any = found_token("path", 4))) {
      expect_eq();
      if(err)
        break;
      read(filepath);
      continue;
    }
  }
  if(!err && filepath.empty())
    error_msg(Error::SQL_PARSE_ERROR, "missing 'path'");
  if(err)
    return err;

  seek_space();
  expect_token("into", 4, token);
  if(err)
    return err;

  seek_space();
  expect_token("col", 3, token);
  if(err)
    return err;
  expect_eq();
  if(err)
    return err;

  std::string col;
  read(col);
  if(!err && col.empty())
    error_msg(Error::SQL_PARSE_ERROR, "missing col 'id|name'");
  if(err)
    return err;

  auto schema = get_schema(hdlr->clients, col);
  if(!err)
    cid = schema->cid;
  return err;
}

void QueryUpdate::parse_display_flags(uint8_t& display_flags) {

  bool any = true;
  while(any && remain && !err) {
    if(found_space())
      continue;

    if((any = found_token("DISPLAY_SPECS", 13))) {
      display_flags |= DB::DisplayFlag::SPECS;
      continue;
    }
    if((any = found_token("DISPLAY_STATS", 13))) {
      display_flags |= DB::DisplayFlag::STATS;
      continue;
    }
  }
}

void QueryUpdate::read_cells() {
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

      seek_space();
      expect_token("(", 1, bracket);
      if(err)
        return;

      cid_t cid = DB::Schema::NO_CID;
      DB::Cells::Cell cell;
      read_cell(cid, cell);
      if(err)
        return;

      seek_space();
      expect_token(")", 1, bracket);
      if(err)
        return;
      hdlr->add(cid, cell);
    }
  }
}

void QueryUpdate::read_cell(cid_t& cid, DB::Cells::Cell& cell) {
  bool comma = false;

  read_flag(cell.flag);
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
  auto schema = get_schema(hdlr->clients, col);
  if(err)
    return;
  cid = schema->cid;
  hdlr->create(schema);


  read_key(cell.key);

  comma = false;
  bool require_ts = cell.flag == DB::Cells::DELETE_EQ;
  if(!err && require_ts)
    expect_comma(comma);
  if(err)
    return;

  seek_space();
  if(err || (!comma && !found_char(','))) {
    cell.set_time_order_desc(true);
    return;
  }

  if(cell.flag == DB::Cells::INSERT) {
    bool time_order;
    seek_space();
    if((time_order = found_token("DESC", 4)) ||
       !(time_order = found_token("ASC", 3)))
      cell.set_time_order_desc(true);
    if(time_order) {
      seek_space();
      if(err || !found_char(','))
        return;
    }
  }
  read_ts_and_value(schema->col_type, require_ts, cell, nullptr);
}

void QueryUpdate::read_flag(uint8_t& flag) {
  std::string buf;
  read(buf, ",");
  if(err)
  return;

  switch(buf.length()) {
  case 1: {
    switch(*buf.data()) {
      case '1':
        flag = DB::Cells::INSERT;
        return;
      case '2':
        flag = DB::Cells::DELETE_LE;
        return;
      case '3':
        flag = DB::Cells::DELETE_EQ;
        return;
      default:
        break;
    }
    break;
  }
  case 6: {
    if(Condition::str_case_eq(buf.data(), "INSERT", 6)) {
      flag = DB::Cells::INSERT;
      return;
    }
    break;
  }
  case 9: {
    if(Condition::str_case_eq(buf.data(), "DELETE_LE", 9)) {
      flag = DB::Cells::DELETE_LE;
      return;
    }
    if(Condition::str_case_eq(buf.data(), "DELETE_EQ", 9)) {
      flag = DB::Cells::DELETE_EQ;
      return;
    }
    break;
  }
  default:
    break;
  }
  std::string s;
  s.reserve(24 + buf.size());
  s.append("unsupported cell flag='");
  s.append(buf);
  s += '\'';
  error_msg(Error::SQL_PARSE_ERROR, s);

}



}}} // SWC::client:SQL namespace
