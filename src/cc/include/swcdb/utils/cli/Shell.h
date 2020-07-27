/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_utils_Shell_h
#define swc_utils_Shell_h

#include "swcdb/core/config/Settings.h"

#include <re2/re2.h>

extern "C" {
int   swc_utils_run();
void  swc_utils_apply_cfg(SWC::Env::Config::Ptr env);
}

namespace SWC { namespace Utils { namespace shell {

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

#endif // swc_utils_Shell_h
