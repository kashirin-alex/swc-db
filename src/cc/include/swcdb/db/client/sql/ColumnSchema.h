/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_sql_ColumnSchema_h
#define swcdb_db_client_sql_ColumnSchema_h


#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"

namespace SWC { namespace client { namespace SQL {

class ColumnSchema final : public Reader {

  public:

  using Func = Comm::Protocol::Mngr::Params::ColumnMng::Function;

  ColumnSchema(const std::string& sql, DB::Schema::Ptr& schema,
              std::string& message);

  ~ColumnSchema() noexcept { }

  int parse(Func* func);

  int parse(Func func, bool token_cmd = false);

  void read_schema_options(Func func);

  private:

  DB::Schema::Ptr& schema;
};


/*
use options:
add|create|modify|delete (schema definitions);
*/

}}} // SWC::client:SQL namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/sql/ColumnSchema.cc"
#endif


#endif //swcdb_db_client_sql_ColumnSchema_h
