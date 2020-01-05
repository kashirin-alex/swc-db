/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_utils_ShellRanger_h
#define swc_lib_utils_ShellRanger_h



namespace SWC { namespace Utils { namespace shell {


class Rgr : public Interface {
  public:
  Rgr() 
    : Interface("\033[32mSWC-DB(\033[36mrgr\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-ranger-history") {
  }
};


}}} // namespace Utils::shell

#endif // swc_lib_utils_ShellRanger_h