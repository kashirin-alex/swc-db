/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_utils_ShellRanger_h
#define swcdb_utils_ShellRanger_h

#include "swcdb/utils/cli/Shell.h"
#include "swcdb/core/comm/Resolver.h"
#include "swcdb/db/client/Clients.h"


namespace SWC { namespace Utils { namespace shell {


class Rgr final : public Interface {
  public:

  client::Clients::Ptr  clients;

  Rgr();

  virtual ~Rgr();

  private:

  bool read_endpoint(std::string& host_or_ips, Comm::EndPoints& endpoints);

  bool report_resources(std::string& cmd);

  bool report(std::string& cmd);

};


}}} // namespace Utils::shell

#endif // swcdb_utils_ShellRanger_h
