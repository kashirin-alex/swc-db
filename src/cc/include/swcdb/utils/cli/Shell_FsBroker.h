/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_utils_ShellFsBroker_h
#define swc_utils_ShellFsBroker_h



namespace SWC { namespace Utils { namespace shell {


class FsBroker : public Interface {
  public:
  FsBroker() 
    : Interface("\033[32mSWC-DB(\033[36mfsbroker\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-fsbroker-history") {
  }
};



}}} // namespace Utils::shell

#endif // swc_utils_ShellFsBroker_h