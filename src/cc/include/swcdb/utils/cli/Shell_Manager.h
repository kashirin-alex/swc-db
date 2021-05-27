/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_utils_ShellManager_h
#define swcdb_utils_ShellManager_h

#include "swcdb/utils/cli/Shell.h"


namespace SWC { namespace Utils { namespace shell {


class Mngr final : public Interface {
  public:

  Mngr();

  virtual ~Mngr();

  private:

  bool cluster_status(std::string&);

  bool managers_status(std::string& cmd);

  bool column_status(std::string& cmd);

  bool rangers_status(std::string& cmd);

};



}}} // namespace Utils::shell

#endif // swcdb_utils_ShellManager_h
