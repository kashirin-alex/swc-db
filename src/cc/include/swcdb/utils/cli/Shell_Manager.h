/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_lib_utils_ShellManager_h
#define swc_lib_utils_ShellManager_h



namespace SWC { namespace Utils { namespace shell {


class Mngr : public Interface {
  public:
  Mngr() 
    : Interface("\033[32mSWC-DB(\033[36mmngr\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-manager-history") {
  }
};



}}} // namespace Utils::shell

#endif // swc_lib_utils_ShellManager_h