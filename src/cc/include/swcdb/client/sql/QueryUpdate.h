/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_client_sql_QueryUpdate_h
#define swcdb_client_sql_QueryUpdate_h


#include "swcdb/client/sql/Reader.h"
#include "swcdb/db/Cells/SpecsScan.h"
#include "swcdb/db/Cells/MapMutable.h"

namespace SWC { namespace client { namespace SQL {

class QueryUpdate : public Reader {

  public:
  QueryUpdate(const std::string& sql, 
              DB::Cells::MapMutable& columns, 
              DB::Cells::MapMutable& columns_onfractions,
              std::string& message);

  ~QueryUpdate();

  const int parse_update();

  const int parse_load(std::string& filepath, int64_t& cid);
  
  void parse_display_flags(uint8_t& display_flags);

  private:

  void read_cells();

  void read_cell(int64_t& cid, DB::Cells::Cell& cell, bool& on_fraction);
  
  void op_from(const uint8_t** ptr, size_t* remainp, 
               uint8_t& op, int64_t& value);

  void read_flag(uint8_t& flag, bool& on_fraction);
  
  DB::Cells::MapMutable& columns;
  DB::Cells::MapMutable& columns_onfractions;
};




}}} // SWC::client:SQL namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/client/sql/QueryUpdate.cc"
#endif 

#endif //swcdb_client_sql_QueryUpdate_h