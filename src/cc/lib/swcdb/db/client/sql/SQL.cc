/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/sql/SQL.h"


namespace SWC { namespace client { namespace SQL {


void parse_select(int& err, const std::string& sql, 
                  DB::Specs::Scan& specs, 
                  uint8_t& display_flags, std::string& message) {
  QuerySelect parser(sql, specs, message);
  err = parser.parse_select();
  if(!err)
    parser.parse_display_flags(display_flags);
}

void parse_update(int& err, const std::string& sql, 
                  DB::Cells::MutableMap& columns, 
                  DB::Cells::MutableMap& columns_onfractions, 
                  uint8_t& display_flags, std::string& message) {
  QueryUpdate parser(sql, columns, columns_onfractions, message);
  err = parser.parse_update();
  if(!err)
    parser.parse_display_flags(display_flags);
}

void parse_list_columns(int& err, const std::string& sql, 
                        std::vector<DB::Schema::Ptr>& schemas, 
                        std::string& message, const char* expect_cmd) {
  ColumnList parser(sql, schemas, message);
  err = parser.parse_list_columns(expect_cmd);
}

void parse_list_columns(int& err, const std::string& sql, 
                        std::vector<DB::Schema::Ptr>& schemas, 
                        Protocol::Mngr::Params::ColumnListReq& params,
                        std::string& message, const char* expect_cmd) {
  ColumnList parser(sql, schemas, message);
  err = parser.parse_list_columns(expect_cmd);
  if(!parser.patterns.empty())
    params.patterns = parser.patterns;
}


void parse_column_schema(int& err, const std::string& sql, 
                        Protocol::Mngr::Req::ColumnMng::Func func,
                        DB::Schema::Ptr& schema, std::string& message) {
  ColumnSchema parser(sql, schema, message);
  err = parser.parse(func);
}

void parse_column_schema(int& err, const std::string& sql,
                        Protocol::Mngr::Req::ColumnMng::Func* func,
                        DB::Schema::Ptr& schema, std::string& message) {
  ColumnSchema parser(sql, schema, message);
  err = parser.parse(func);
}

void parse_dump(int& err, const std::string& sql, 
                std::string& filepath, DB::Specs::Scan& specs, 
                uint8_t& output_flags, uint8_t& display_flags, 
                std::string& message) {
  QuerySelect parser(sql, specs, message);
  err = parser.parse_dump(filepath);
  if(!err)
    parser.parse_output_flags(output_flags);
  if(!err)
    parser.parse_display_flags(display_flags);
}

void parse_load(int& err, const std::string& sql, 
                std::string& filepath, cid_t& cid,  
                uint8_t& display_flags, std::string& message) {
  DB::Cells::MutableMap columns;
  DB::Cells::MutableMap columns_onfractions;
  QueryUpdate parser(sql, columns, columns_onfractions, message);
  err = parser.parse_load(filepath, cid);
  if(!err)
    parser.parse_display_flags(display_flags);
}




}}} // SWC::client:SQL namespace
