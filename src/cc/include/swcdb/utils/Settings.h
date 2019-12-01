/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_utils_Settings_h
#define swc_app_utils_Settings_h

#include "swcdb/core/config/Settings.h"

extern "C"{
typedef int swc_utils_run_t();
typedef void swc_utils_apply_cfg_t(SWC::Env::Config::Ptr env);
}

namespace SWC{ namespace Config {


void Settings::init_app_options() {

  cmdline_desc
  .definition(
    usage_str("SWC-DB(utils) Usage: %s 'command' [options]\n\nOptions:")
  )
  .add_options()
   ("command,cmd",  str("shell"),  
   "Command to execute shell|status|report|custom (1st arg token)")
   ("command",  1)

   ("lib-path",  str(install_path+"/lib/"), "Path to utilities libraries")
   ("lib",  str(), "Utility-Library of the custom command")

   ("ranger,rgr",   "Work with Ranger")
   ("manager,mngr", "Work with Manager")
   ("fsbroker",     "Work with FsBroker")
   // default work with DbClient
  ;

}

void Settings::init_post_cmd_args(){ }

}}

#endif // swc_app_utils_Settings_h