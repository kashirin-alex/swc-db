/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_client_SQL_h
#define swcdb_client_SQL_h


#include "swcdb/client/Clients.h"
#include "swcdb/client/AppContext.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnList.h"
#include "swcdb/db/Protocol/Common/req/Query.h"

#include "swcdb/client/sql/Reader.h"
#include "swcdb/client/sql/ColumnSchema.h"
#include "swcdb/client/sql/ColumnList.h"
#include "swcdb/client/sql/QuerySelect.h"
#include "swcdb/client/sql/QueryUpdate.h"


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
                  DB::Cells::MapMutable& columns, 
                  DB::Cells::MapMutable& columns_onfraction, 
                  uint8_t& display_flags, std::string& message) {
  QueryUpdate parser(sql, columns, columns_onfraction, message);
  err = parser.parse_update();
  if(!err)
    parser.parse_display_flags(display_flags);
}

void parse_list_columns(int& err, const std::string& sql, 
                        std::vector<DB::Schema::Ptr>& schemas, 
                        std::string& message) {
  ColumnList parser(sql, schemas, message);
  err = parser.parse_list_columns();
}

void parse_column_schema(int& err, const std::string& sql, 
                        Protocol::Mngr::Req::ColumnMng::Func func,
                        DB::Schema::Ptr& schema, std::string& message) {
  ColumnSchema parser(sql, schema, message);
  err = parser.parse(func);
}

void parse_dump(int& err, const std::string& sql, 
                std::string& filepath, DB::Specs::Scan& specs, 
                uint8_t& display_flags, std::string& message) {
  QuerySelect parser(sql, specs, message);
  err = parser.parse_dump(filepath);
  if(!err)
    parser.parse_display_flags(display_flags);
}

void parse_load(int& err, const std::string& sql, 
                std::string& filepath, int64_t& cid,  
                uint8_t& display_flags, std::string& message) {
  DB::Cells::MapMutable columns;
  DB::Cells::MapMutable columns_onfraction;
  QueryUpdate parser(sql, columns, columns_onfraction, message);
  err = parser.parse_load(filepath, cid);
  if(!err)
    parser.parse_display_flags(display_flags);
}




}}} // SWC::client:SQL namespace

#endif //swcdb_client_SQL_h