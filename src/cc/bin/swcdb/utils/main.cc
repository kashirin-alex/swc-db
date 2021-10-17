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
    lib_path.reserve(lib_path.length() + 18 + cmd.length());
    lib_path.append("libswcdb_utils_");
    lib_path.append(cmd);
    lib_path.append(SWC_DSO_EXT);// {lib-path}/libswcdb_utils_shell.so
  }

  const char* err = dlerror();
  void* handle = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_LAZY | RTLD_LOCAL);
  if (handle == nullptr || err != nullptr)
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
  int res = reinterpret_cast<swcdb_utils_run_t*>(f_new_ptr)();

  SWC_CAN_QUICK_EXIT(res);
  reinterpret_cast<swcdb_utils_apply_cfg_t*>(f_cfg_ptr)(nullptr);
  dlclose(handle);
  return res;
}



int not_implemented(const std::string& cmd) {
  SWC_LOGF(LOG_ERROR, "Not implemented, Utils command=%s", cmd.c_str());
  return 1;
}

}} // namespace SWC::Utils



int main(int argc, char** argv) {
  SWC::Env::Config::init(
    argc,
    argv,
    &SWC::Config::init_app_options,
    &SWC::Config::init_post_cmd_args
  );

  int res;
  {
    const auto& command = SWC::Env::Config::settings()->get_str("command");

    if(SWC::Condition::str_case_eq(command.data(), "shell", command.size())) {
      res = SWC::Utils::run(command);
    } else if(SWC::Condition::str_case_eq(
                command.data(), "custom", command.size())) {
      res = SWC::Utils::run("command", true);
    } else {
      res = SWC::Utils::not_implemented(command);
    }
  }

  SWC::Env::Config::reset();
  return res;
}
