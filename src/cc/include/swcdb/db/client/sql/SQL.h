/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_client_SQL_h
#define swcdb_client_SQL_h


#include "swcdb/db/client/Clients.h"

#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnList.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact.h"

#include "swcdb/db/client/Query/Select.h"

#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/client/sql/ColumnSchema.h"
#include "swcdb/db/client/sql/ColumnList.h"
#include "swcdb/db/client/sql/QuerySelect.h"
#include "swcdb/db/client/sql/QueryUpdate.h"


namespace SWC { namespace client { namespace SQL {


void parse_select(int& err, const std::string& sql, 
                  DB::Specs::Scan& specs, 
                  uint8_t& display_flags, std::string& message);

void parse_update(int& err, const std::string& sql, 
                  DB::Cells::MutableMap& columns, 
                  DB::Cells::MutableMap& columns_onfractions, 
                  uint8_t& display_flags, std::string& message);

void parse_list_columns(int& err, const std::string& sql, 
                        std::vector<DB::Schema::Ptr>& schemas, 
                        std::string& message, const char* expect_cmd);

void parse_list_columns(int& err, const std::string& sql, 
                        std::vector<DB::Schema::Ptr>& schemas, 
                        Protocol::Mngr::Params::ColumnListReq& params,
                        std::string& message, const char* expect_cmd);

void parse_column_schema(int& err, const std::string& sql, 
                        Protocol::Mngr::Req::ColumnMng::Func func,
                        DB::Schema::Ptr& schema, std::string& message);

void parse_column_schema(int& err, const std::string& sql,
                        Protocol::Mngr::Req::ColumnMng::Func* func,
                        DB::Schema::Ptr& schema, std::string& message);

void parse_dump(int& err, const std::string& sql, 
                std::string& filepath, DB::Specs::Scan& specs, 
                uint8_t& output_flags, uint8_t& display_flags, 
                std::string& message);

void parse_load(int& err, const std::string& sql, 
                std::string& filepath, cid_t& cid,  
                uint8_t& display_flags, std::string& message);




}}} // SWC::client:SQL namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/sql/SQL.cc"
#endif 


#endif //swcdb_client_SQL_h