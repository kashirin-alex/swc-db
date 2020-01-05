/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/utils/cli/Shell.h"
#include "swcdb/utils/cli/Shell_DbClient.h"
#include "swcdb/utils/cli/Shell_Manager.h"
#include "swcdb/utils/cli/Shell_Ranger.h"
#include "swcdb/utils/cli/Shell_FsBroker.h"


namespace SWC { namespace Utils { namespace shell {


int run() {
  auto settings = SWC::Env::Config::settings();

  if(settings->has("ranger"))
    return Rgr().run();
    
  if(settings->has("manager"))
    return Mngr().run();
  
  if(settings->has("fsbroker"))
    return FsBroker().run();

  try {
    return DbClient().run();
  } catch (std::exception& e) {
    std::cout << e.what() << '\n';
  }

  return 1;
}




}}} // namespace SWC::Utils::shell

extern "C" {
int swc_utils_run() {
  return SWC::Utils::shell::run();
};
void swc_utils_apply_cfg(SWC::Env::Config::Ptr env){
  SWC::Env::Config::set(env);
};
}
