/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_app_utils_Settings_h
#define swcdb_app_utils_Settings_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"
#include "swcdb/fs/Settings.h"
#include "swcdb/db/client/Settings.h"

extern "C"{
typedef int swcdb_utils_run_t();
typedef void swcdb_utils_apply_cfg_t(SWC::Env::Config::Ptr env);
}

namespace SWC {



//! The SWC-DB Utilities Applications C++ namespace 'SWC::Utils'
namespace Utils { }



namespace Config {


void Settings::init_app_options() {

  init_comm_options();
  init_client_options();
  init_fs_options();

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

   ("ranger,rgr",    "Work with Ranger")
   ("manager,mngr",  "Work with Manager")
   ("filesystem,fs", "Work with FileSystem type by swc.fs='type'")
   // default work with DbClient
  ;

}

void Settings::init_post_cmd_args() {
  auto loglevel = get<Property::V_GENUM>("swc.logging.level");
  if(loglevel->is_default())
    loglevel->set(LOG_WARN);
}

}}

#endif // swcdb_app_utils_Settings_h
