/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_utils_Shell_h
#define swcdb_utils_Shell_h

#include "swcdb/core/config/Settings.h"

#include <re2/re2.h>

extern "C" {
int   swcdb_utils_run();
void  swcdb_utils_apply_cfg(SWC::Env::Config::Ptr env);
}

namespace SWC { 

//! The SWC-DB Utilities C++ namespace 'SWC::Utils'
namespace Utils {


namespace shell {


int run();

class Interface {

  public:
  Interface(const std::string& prompt="CLI>", 
            const std::string& history="/tmp/.swc-cli-history");
  
  virtual ~Interface();

  int run();
  
  protected:

  struct Option final {
    typedef std::function<bool(std::string&)> Call_t;

    Option(const std::string& name, const std::vector<std::string>& desc, 
            const Call_t& call, const re2::RE2* re) 
          : name(name), desc(desc), call(call), re(re) { 
    }

    ~Option() {
      if(re)
        delete re;
    }

    const std::string              name;
    const std::vector<std::string> desc;
    const Call_t      call;
    const re2::RE2*   re;
  };

  mutable int          err;
  std::vector<Option*> options;
  
  bool error(const std::string& message);

  virtual bool quit(std::string& cmd) const;

  virtual bool help(std::string& cmd) const;

  private:

  void init();

  bool cmd_option(std::string& cmd) const;

  const std::string   prompt;
  const std::string   history;
}; 


}}} // namespace Utils::shell

#endif // swcdb_utils_Shell_h
