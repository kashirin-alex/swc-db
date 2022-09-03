/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_SQL_h
#define swcdb_db_client_SQL_h


#include "swcdb/db/client/Clients.h"

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"

#include "swcdb/db/client/Query/Select/Scanner.h"
#include "swcdb/db/client/Query/Update/Committer.h"

#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/client/sql/ColumnSchema.h"
#include "swcdb/db/client/sql/ColumnList.h"
#include "swcdb/db/client/sql/QuerySelect.h"
#include "swcdb/db/client/sql/QueryUpdate.h"


namespace SWC { namespace client {


//! The SWC-DB SQL C++ namespace 'SWC::client::SQL'
namespace SQL {


enum Cmd : uint8_t {
  UNKNOWN         = 0,
  CREATE_COLUMN   = 1,
  MODIFY_COLUMN   = 2,
  REMOVE_COLUMN   = 3,
  GET_COLUMNS     = 4,
  COMPACT_COLUMNS = 5,
  SELECT          = 6,
  UPDATE          = 7
};

Cmd recognize_cmd(int& err, const std::string& sql, std::string& message);

void parse_select(int& err, const Clients::Ptr& clients,
                  const std::string& sql,
                  DB::Specs::Scan& specs,
                  uint8_t& display_flags, std::string& message);

void parse_update(int& err, const std::string& sql,
                  const Query::Update::Handlers::BaseUnorderedMap::Ptr& hdlr,
                  uint8_t& display_flags, std::string& message);

void parse_list_columns(int& err, const Clients::Ptr& clients,
                        const std::string& sql,
                        DB::SchemasVec& schemas,
                        std::string& message, const char* expect_cmd);

void parse_list_columns(int& err, const Clients::Ptr& clients,
                        const std::string& sql,
                        DB::SchemasVec& schemas,
                        Comm::Protocol::Mngr::Params::ColumnListReq& params,
                        std::string& message, const char* expect_cmd);

void parse_list_columns(int& err, const Clients::Ptr& clients,
                        const std::string& sql,
                        DB::SchemasVec& schemas,
                        Comm::Protocol::Mngr::Params::ColumnListReq& params,
                        uint8_t& output_flags,
                        std::string& message, const char* expect_cmd);

void parse_column_schema(
        int& err, const std::string& sql,
        Comm::Protocol::Mngr::Params::ColumnMng::Function func,
        DB::Schema::Ptr& schema, std::string& message);

void parse_column_schema(
        int& err, const std::string& sql,
        Comm::Protocol::Mngr::Params::ColumnMng::Function* func,
        DB::Schema::Ptr& schema, std::string& message);

void parse_dump(int& err, const Clients::Ptr& clients,
                const std::string& sql,
                std::string& fs, std::string& filepath,
                uint64_t& split_size, std::string& ext, int& level,
                DB::Specs::Scan& specs,
                uint8_t& output_flags, uint8_t& display_flags,
                std::string& message);

void parse_load(int& err, const std::string& sql,
                const Query::Update::Handlers::BaseUnorderedMap::Ptr& hdlr,
                std::string& fs, std::string& filepath, cid_t& cid,
                uint8_t& display_flags, std::string& message);





}}} // SWC::client:SQL namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/sql/SQL.cc"
#endif


#endif //swcdb_db_client_SQL_h
