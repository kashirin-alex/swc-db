/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_utils_ShellDbClient_h
#define swc_utils_ShellDbClient_h

#include "swcdb/utils/cli/Shell.h"
#include "swcdb/db/client/sql/SQL.h"

namespace SWC { namespace Utils { namespace shell {


class DbClient : public Interface {

  public:

  DbClient();

  bool mng_column(Protocol::Mngr::Req::ColumnMng::Func func, 
                  std::string& cmd);
  
  bool compact_column(std::string& cmd);
  
  bool list_columns(std::string& cmd);

  bool select(std::string& cmd);

  void display(const client::Query::Select::Result::Ptr& result,
               uint8_t display_flags, 
               size_t& cells_count, size_t& cells_bytes) const;

  bool update(std::string& cmd);

  bool load(std::string& cmd);

  bool dump(std::string& cmd);

  void display_stats(const client::Query::Profiling& profile, 
                     size_t took, size_t bytes,
                     size_t cells_count, size_t resend_cells = 0) const;
  
};



}}} // namespace Utils::shell

#endif // swc_utils_ShellDbClient_h