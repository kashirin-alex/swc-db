/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_client_sql_ColumnSchema_h
#define swcdb_client_sql_ColumnSchema_h


#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"

namespace SWC { namespace client { namespace SQL {

class ColumnSchema final : public Reader {

  public:
  
  using Func = Protocol::Mngr::Params::ColumnMng::Function;

  ColumnSchema(const std::string& sql, DB::Schema::Ptr& schema,
              std::string& message);

  ~ColumnSchema();

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


#endif //swcdb_client_sql_ColumnSchema_h