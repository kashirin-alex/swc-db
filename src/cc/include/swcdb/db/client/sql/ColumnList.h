/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_sql_ColumnList_h
#define swcdb_db_client_sql_ColumnList_h

#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace client { namespace SQL {


enum ColumnOutputFlag : uint8_t {
  ONLY_CID  = 0x01
};


class ColumnList final : public Reader {

  public:

  DB::Schemas::SelectorPatterns patterns;

  ColumnList(const Clients::Ptr& clients, const std::string& sql,
             DB::SchemasVec& schemas, std::string& message);

  ~ColumnList() noexcept { }

  int parse_list_columns(const char* expect_cmd, uint8_t& output_flags);

  int parse_list_columns(const char* expect_cmd);

  private:

  void _parse_list_columns(const char* expect_cmd);

  void parse_list_columns();

  void read_columns(DB::SchemasVec& cols, const char* stop);

  Clients::Ptr     clients;
  DB::SchemasVec&  schemas;
};


/*
use options:
get|list column|columns|schema|schemas name|ID;
get|list column|columns|schema|schemas [name|ID,name|ID];
get|list column|columns|schema|schemas "name|ID" "name|ID]";
*/

}}} // SWC::client:SQL namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/sql/ColumnList.cc"
#endif


#endif //swcdb_db_client_sql_ColumnList_h
