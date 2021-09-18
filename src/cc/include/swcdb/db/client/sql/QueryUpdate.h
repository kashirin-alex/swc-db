/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_sql_QueryUpdate_h
#define swcdb_db_client_sql_QueryUpdate_h


#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/Cells/SpecsScan.h"
#include "swcdb/db/client/Query/Update/Handlers/BaseUnorderedMap.h"

namespace SWC { namespace client { namespace SQL {

class QueryUpdate final : public Reader {

  public:
  QueryUpdate(const std::string& sql,
              const Query::Update::Handlers::BaseUnorderedMap::Ptr& hdlr,
              std::string& message);

  ~QueryUpdate() noexcept { }

  int parse_update();

  int parse_load(std::string& fs, std::string& filepath, cid_t& cid);

  void parse_display_flags(uint8_t& display_flags);

  private:

  void read_cells();

  void read_cell(cid_t& cid, DB::Cells::Cell& cell, bool& on_fraction);

  void op_from(const uint8_t** ptr, size_t* remainp,
               uint8_t& op, int64_t& value);

  void read_flag(uint8_t& flag, bool& on_fraction);

  Query::Update::Handlers::BaseUnorderedMap::Ptr hdlr;
};




}}} // SWC::client:SQL namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/sql/QueryUpdate.cc"
#endif

#endif //swcdb_db_client_sql_QueryUpdate_h
