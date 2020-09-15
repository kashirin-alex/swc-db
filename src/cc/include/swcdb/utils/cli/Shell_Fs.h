/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_utils_ShellFs_h
#define swc_utils_ShellFs_h

#include "swcdb/utils/cli/Shell.h"


namespace SWC { namespace Utils { namespace shell {


class Fs : public Interface {
  public:
  Fs();

  virtual ~Fs();

  bool ls(const std::string& cmd);

};



}}} // namespace Utils::shell

#endif // swc_utils_ShellFs_h
