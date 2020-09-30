/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_sql_ColumnList_h
#define swcdb_db_client_sql_ColumnList_h

#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace client { namespace SQL {

class ColumnList final : public Reader {

  public:
  
  std::vector<DB::Schemas::Pattern> patterns;

  ColumnList(const std::string& sql, std::vector<DB::Schema::Ptr>& schemas,
              std::string& message);

  ~ColumnList();

  int parse_list_columns(const char* expect_cmd);

  private:
  
  void read_columns(std::vector<DB::Schema::Ptr>& cols, const char* stop);
  
  std::vector<DB::Schema::Ptr>& schemas;
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
