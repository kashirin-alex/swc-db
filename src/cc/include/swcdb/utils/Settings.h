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



/**
 * \defgroup Applications The Applications Group
 * @brief A group of all the SWC-DB Applications / Programs.
 *
 * The group consists all the SWC-DB Applications / Programs with a 'main()' function
 */



//! The SWC-DB Utilities Applications C++ namespace 'SWC::Utils'
namespace Utils { }



namespace Config {


void init_app_options(Settings* settings) {

  init_comm_options(settings);
  init_client_options(settings);
  init_fs_options(settings);

  settings->cmdline_desc
  .definition(
    settings->usage_str(
      "SWC-DB(utils) Usage: %s 'command' [options]\n\nOptions:")
  )
  .add_options()
   ("command,cmd", str("shell"),
   "Command to execute shell|status|report|custom (1st arg token)")
   ("command",  1)

   ("lib-path", str(settings->install_path+"/lib/"),
    "Path to utilities libraries")
   ("lib", str(), "Utility-Library of the custom command")

   ("with-broker", boo(false)->zero_token(),
    "Query applicable requests with Broker")

   ("ranger,rgr",                 "Work with Ranger")
   ("manager,mngr",               "Work with Manager")
   ("filesystem,fs",              "Work with FileSystem type by swc.fs='type'")
   ("statistics,stats,monit",     "Work with Statistics monitor")
   // default work with DbClient
  ;

}

void init_post_cmd_args(Settings* settings) {
  auto loglevel = settings->get<Property::Value_enum_g>("swc.logging.level");
  if(loglevel->is_default())
    loglevel->set(LOG_WARN);
}

}}

#endif // swcdb_app_utils_Settings_h
