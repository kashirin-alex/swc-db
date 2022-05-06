/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_utils_Shell_h
#define swcdb_utils_Shell_h

#include "swcdb/core/config/Settings.h"


extern "C" {
int   swcdb_utils_run();
void  swcdb_utils_apply_cfg(SWC::Env::Config::Ptr env);
}

namespace SWC { namespace Utils {


/**
 * @brief The SWC-DB Shell C++ namespace 'SWC::Utils::shell'
 *
 * \ingroup Applications
 */
namespace shell {


enum CLI : uint8_t {
  QUIT_CLI    = 0x00,
  DBCLIENT    = 0x01,
  MANAGER     = 0x02,
  RANGER      = 0x03,
  FILESYSTEM  = 0x04,
  STATISTICS  = 0x05
};


int run();


class Interface {
  public:

  Interface(std::string&& prompt="CLI>",
            std::string&& history="/tmp/.swc-cli-history");

  virtual ~Interface();

  CLI run();

  protected:

  mutable int           err;
  Core::Atomic<CLI>     _state;

  typedef std::function<bool(std::string&)> OptCall_t;
  void add_option(const char* a_name,
                  Core::Vector<std::string>&& a_desc,
                  OptCall_t&& a_call,
                  const char* a_re);

  bool error(const std::string& message);

  virtual bool quit(std::string&) const {
    return false;
  }

  virtual bool switch_to(std::string& cmd);

  virtual bool help(std::string& cmd) const;

  private:

  void init();

  bool cmd_option(std::string& cmd) const;

  const std::string   prompt;
  const std::string   history;

  struct Option;
  Core::Vector<Option*> options;
};


}}} // namespace Utils::shell

#endif // swcdb_utils_Shell_h
