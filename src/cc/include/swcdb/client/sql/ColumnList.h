/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_client_sql_ColumnList_h
#define swcdb_client_sql_ColumnList_h

#include "swcdb/client/sql/Reader.h"


namespace SWC { namespace client { namespace SQL {

class ColumnList : public Reader {

  public:
  ColumnList(const std::string& sql, std::vector<DB::Schema::Ptr>& schemas,
              std::string& message);

  ~ColumnList();

  const int parse_list_columns(const char* expect_cmd);

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
#include "swcdb/client/sql/ColumnList.cc"
#endif 


#endif //swcdb_client_sql_ColumnList_h