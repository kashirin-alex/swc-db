/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_client_SQL_h
#define swcdb_client_SQL_h


#include "swcdb/client/Clients.h"
#include "swcdb/client/AppContext.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Common/req/Query.h"

#include "swcdb/client/sql/Reader.h"
#include "swcdb/client/sql/ColumnList.h"
#include "swcdb/client/sql/QuerySelect.h"


namespace SWC { namespace client { namespace SQL {


void parse_select(int& err, const std::string& sql, 
                  DB::Specs::Scan& specs, std::string& message) {
  QuerySelect parser(sql, specs, message);
  err = parser.parse_select();
}


void parse_list_columns(int& err, const std::string& sql, 
                        std::vector<DB::Schema::Ptr>& schemas, 
                        std::string& message) {
  ColumnList parser(sql, schemas, message);
  err = parser.parse_list_columns();
}



}}} // SWC::client:SQL namespace

#endif //swcdb_client_SQL_h