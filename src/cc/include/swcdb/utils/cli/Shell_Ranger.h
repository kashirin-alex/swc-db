/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_utils_ShellRanger_h
#define swc_utils_ShellRanger_h

#include "swcdb/utils/cli/Shell.h"
#include "swcdb/core/comm/Resolver.h"


namespace SWC { namespace Utils { namespace shell {


class Rgr : public Interface {
  public:

  Rgr();

  bool read_endpoint(std::string& host_or_ips, EndPoints& endpoints);

  bool report_resources(std::string& cmd);

  bool report(std::string& cmd);
  
};


}}} // namespace Utils::shell

#endif // swc_utils_ShellRanger_h
