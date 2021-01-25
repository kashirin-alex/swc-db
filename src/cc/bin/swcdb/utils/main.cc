/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/utils/Settings.h"
#include <dlfcn.h>

namespace SWC { namespace Utils {


int run(const std::string& cmd, bool custom=false) {

  std::string lib_path(Env::Config::settings()->get_str("lib-path"));
  if(custom) {
    lib_path.append(Env::Config::settings()->get_str("lib"));
  } else {
    lib_path.append("libswcdb_utils_");
    lib_path.append(cmd);
    lib_path.append(".so");// {lib-path}/libswcdb_utils_shell.so
  }

  const char* err = dlerror();
  void* handle = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_LAZY | RTLD_LOCAL);
  if (handle == NULL || err != NULL)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Shared Lib %s, open fail: %s\n", lib_path.c_str(), err);

  err = dlerror();
  std::string handler_name =  "swcdb_utils_apply_cfg";
  void* f_cfg_ptr = dlsym(handle, handler_name.c_str());
  if(err || !f_cfg_ptr)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Shared Lib %s, link(%s) fail: %s handle=%p\n",
              lib_path.c_str(), handler_name.c_str(), err, handle);
  reinterpret_cast<swcdb_utils_apply_cfg_t*>(f_cfg_ptr)(Env::Config::get());

  err = dlerror();
  handler_name =  "swcdb_utils_run";
  void* f_new_ptr = dlsym(handle, handler_name.c_str());
  if(err || !f_new_ptr)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Shared Lib %s, link(%s) fail: %s handle=%p\n",
              lib_path.c_str(), handler_name.c_str(), err, handle);
  return reinterpret_cast<swcdb_utils_run_t*>(f_new_ptr)();
}



int not_implemented(const std::string& cmd) {
  SWC_LOGF(LOG_ERROR, "Not implemented, Utils command=%s", cmd.c_str());
  return 1;
}

}} // namespace SWC::Utils



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);

  const auto& command = SWC::Env::Config::settings()->get_str("command");

  if(!strncasecmp(command.data(), "shell", command.size()))
    return SWC::Utils::run(command);

  if(!strncasecmp(command.data(), "custom", command.size()))
    return SWC::Utils::run("command", true);

  return SWC::Utils::not_implemented(command);
}
