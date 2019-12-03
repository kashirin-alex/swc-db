/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_utils_Shell_h
#define swc_lib_utils_Shell_h

#include "swcdb/core/config/Settings.h"

#include <re2/re2.h>
#include <termios.h>
#include <editline.h>

extern "C" {
int   swc_utils_run();
void  swc_utils_apply_cfg(SWC::Env::Config::Ptr env);
}

namespace SWC { namespace Utils { namespace shell {

int run();

class Interface {
  public:
  Interface(const char* prompt="CLI>", 
            const char* history="/tmp/.swc-cli-history")
            : prompt(prompt), history(history) {}

  virtual ~Interface() {
    for(auto o : options)
      delete o;
  }

  int run() {
    std::cout << "Running SWC::Utils::shell " << prompt << "\n";  
  
    char* line;
    read_history(history);

    while((line = readline(prompt)) && process_line(line));
    return 0;
  }
  
  private:
  bool process_line(char* line) {
    bool ok = line;
    if(!ok)
      return ok;

    const char* end = line + strlen(line);
    const char* ptr = line;
    const char* begin = line;
    std::string cmd;
    int len = 0;
    bool quoted_1 = false;
    bool quoted_2 = false;
    bool executed = true;
    while(ptr < end) {
      if(*ptr == '#')
        break;

      if(*ptr == '\'')
        quoted_1 = !quoted_1;
      else if(*ptr == '\"')
        quoted_2 = !quoted_2;

      len++;
      if(*ptr++ == ';') {
        if(quoted_1 || quoted_2)
          continue;
        cmd = std::string(begin, len);
        add_history(cmd.c_str());
        if(!(ok = cmd_option(cmd.c_str())))
          break;
        len = 0;
        begin = ptr;
      } else if(ptr == end) {
        executed = false;
      }
    }
    if(!executed)
      std::cout << "Missing closing '\033[31m;\033[00m' for '\033[31m" 
                <<  std::string(begin, len) << "\033[00m'\n";

    //fdhgfg d; #gdfgsfdgdfgs(aaa="fdgsdg; fdgsfdg; "); gdgd; quit;
	  
    write_history(history);
	  free(line);
    
    return ok;
  }

  bool cmd_option(const char* line) {
    auto opt = std::find_if(options.begin(), options.end(), 
                [line](const Option* opt){ 
                  return RE2::PartialMatch(line, *opt->re); 
                });
    if(opt != options.end()) {
	    printf("\t\t\tran|%s|\n", line);
      return (*opt)->call(line);
    } else {
      std::cout << "Unknown command='\033[31m" << line << "\033[00m'\n";
    }
    return true;
  }
  
  virtual bool quit(const char* line) {
    return false;
  }

  virtual bool help(const char* line) {
    std::cout << "Usage Help:  \033[4m'command' [options];\033[00m\n";
    size_t offset_name = 0;
    size_t offset_desc = 0;
    for(auto opt : options) {
      if(offset_name < opt->name.length())
        offset_name = opt->name.length();
      if(offset_desc < opt->desc.length())
        offset_desc = opt->desc.length();
    }
    offset_name += 4;
    offset_desc += 4;
    for(auto opt : options)
      std::cout << std::left << std::setw(2) << " "
                << std::left << std::setw(offset_name) << opt->name
                << std::left << std::setw(offset_desc) << opt->desc
                << std::endl;
    return true;
  }

  const char* prompt;
  const char* history;

  protected:

  struct Option {
    typedef std::function<bool(const char*)> Call_t;
    Option(const std::string& name, const std::string& desc, 
            const Call_t& call, const re2::RE2* re) 
          : name(name), desc(desc), call(call), re(re) { }
    ~Option() {
      if(re)
        delete re;
    }
    const std::string name;
    const std::string desc;
    const Call_t      call;
    const re2::RE2*   re;
  };

  std::vector<Option*> options {
    new Option("quit", "Quit or Exit the Console", 
                [ptr=this](const char* line){return ptr->quit(line);}, 
                new re2::RE2("(^|\\s+)(quit|exit)(\\s+|);(\\s+|$)")),
    new Option("help", "Commands help information", 
                [ptr=this](const char* line){return ptr->help(line);}, 
                new re2::RE2("(^|\\s+)help(\\s+|);(\\s+|$)"))
  };
}; 

class Rgr : public Interface {
  public:
  Rgr() 
    : Interface("\033[32mSWC-DB(\033[36mrgr\033[32m\033[00m)\033[33m> \033[00m",
                "/tmp/.swc-cli-ranger-history") {
  }
};
class Mngr : public Interface {
  public:
  Mngr() 
    : Interface("\033[32mSWC-DB(\033[36mmngr\033[32m\033[00m)\033[33m> \033[00m",
                "/tmp/.swc-cli-manager-history") {
  }
};
class FsBroker : public Interface {
  public:
  FsBroker() 
    : Interface("\033[32mSWC-DB(\033[36mfsbroker\033[32m\033[00m)\033[33m> \033[00m",
                "/tmp/.swc-cli-fsbroker-history") {
  }
};
class DbClient : public Interface {
  public:
  DbClient() 
    : Interface("\033[32mSWC-DB(\033[36mclient\033[32m\033[00m)\033[33m> \033[00m",
                "/tmp/.swc-cli-dbclient-history") {
  }
};



/*
tcflag_t old_lflag;
cc_t     old_vtime;
struct termios term;

int quit() {
  std::cout << std::endl;
  term.c_lflag     = old_lflag;
  term.c_cc[VTIME] = old_vtime;
  if(tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0 ) {
    perror("tcsetattr");
    return 1;
  }
  return 0;
}

int run_async() {
  el_hist_size = 4096;
  read_history(history);

  if(tcgetattr(STDIN_FILENO, &term) < 0 ) {
    perror("tcgetattr");
    return 1;
  }
  
  old_lflag = term.c_lflag;
  old_vtime = term.c_cc[VTIME];
  
  term.c_lflag &= ~ICANON;
  term.c_cc[VTIME] = 1;
  if(tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0 ) {
    perror("tcsetattr");
    return 1;
  }

  rl_callback_handler_install(get_prompt(), process_line);

  fd_set fds;
  do {
    FD_ZERO(&fds);
    FD_SET(fileno(stdin), &fds);

    if(select(FD_SETSIZE, &fds, NULL, NULL, NULL) < 0) {
      perror("select");
      return quit();
    }

    if(FD_ISSET(fileno(stdin), &fds))
      rl_callback_read_char();
  } while(1);

  return quit();
}
*/


}}} // namespace SWC::Utils::shell

#endif // swc_lib_utils_Shell_h