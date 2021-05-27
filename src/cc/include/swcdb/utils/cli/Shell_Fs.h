/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_utils_ShellFs_h
#define swcdb_utils_ShellFs_h

#include "swcdb/utils/cli/Shell.h"


namespace SWC { namespace Utils { namespace shell {


class Fs final : public Interface {
  public:

  Fs();

  virtual ~Fs();

  private:

  bool ls(const std::string& cmd);

};



}}} // namespace Utils::shell

#endif // swcdb_utils_ShellFs_h
