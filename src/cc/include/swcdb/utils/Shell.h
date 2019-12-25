/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_utils_Shell_h
#define swc_lib_utils_Shell_h

#include "swcdb/core/config/Settings.h"

#include <re2/re2.h>
#include <editline.h> // github.com/troglobit/editline

int el_hist_size = 4000;

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
            : prompt(prompt), history(history) {
  }
  
  virtual ~Interface() {
    for(auto o : options)
      delete o;
  }

  int run() {
  
    read_history(history);
    char* line;
    char* ptr;
    const char* prompt_state = prompt;
    char c;
    
    bool stop = false;
    bool cmd_end = false;;
    bool comment = false;;
    bool quoted_1 = false;
    bool quoted_2 = false;
    bool next_line = false;
    std::string cmd;

    while(!stop && (ptr = line = readline(prompt_state))) {
      //std::cout << "line=" << line << "\n";
      prompt_state = ""; // "-> ";
      do {
        c = *ptr++;
        if(next_line = c == 0)
          c = '\n';

        //std::cout << " c=" << c << "(" << (int)c << ") \n";
        if(c == '\n' && cmd_end) {
          add_history(cmd.c_str());
          write_history(history);
          stop = !cmd_option(cmd);
          cmd.clear();
          cmd_end = false;
          prompt_state = prompt;
          break;

        } else if(c == '\n' && cmd.empty()) {
          prompt_state = prompt;
          break;

        } else if(c == ' ' && cmd.empty()) {
          continue;

        } else if(!quoted_1 && !quoted_2) {
          if(c == '#') {
            comment = true;
            continue;
          } else if(c == '\n' && comment) {
            comment = false;
            break;
          } else if(comment)
            continue;
        }

        cmd.append(std::string(&c, 1));

        if(c == '\'')
          quoted_1 = !quoted_1;
        else if(c == '"')
          quoted_2 = !quoted_2;
        else if(c == ';' && !quoted_1 && !quoted_2)
          cmd_end = true;

      } while(!next_line);
      
	    free(line);
    }

    return 0;
  }
  

  protected:

  struct Option final {
    typedef std::function<bool(const std::string&)> Call_t;

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
                [ptr=this](const std::string& line){return ptr->quit(line);}, 
                new re2::RE2("(?i)(^|\\s+)(quit|exit)(\\s+|);(\\s+|$)")),
    new Option("help", "Commands help information", 
                [ptr=this](const std::string& line){return ptr->help(line);}, 
                new re2::RE2("(?i)(^|\\s+)help(\\s+|);(\\s+|$)"))
  };
  
  virtual bool quit(const std::string& line) {
    return false;
  }

  virtual bool help(const std::string& line) {
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


  private:

  bool cmd_option(const std::string& line) {
    auto opt = std::find_if(options.begin(), options.end(), 
                [line](const Option* opt){ 
                  return RE2::PartialMatch(line.c_str(), *opt->re); 
                });
    if(opt != options.end())
      return (*opt)->call(line);
    std::cout << "Unknown command='\033[31m" << line << "\033[00m'\n";
    return true;
  }

  const char*  prompt;
  const char*  history;
}; 




class Rgr : public Interface {
  public:
  Rgr() 
    : Interface("\033[32mSWC-DB(\033[36mrgr\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-ranger-history") {
  }
};

class Mngr : public Interface {
  public:
  Mngr() 
    : Interface("\033[32mSWC-DB(\033[36mmngr\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-manager-history") {
  }
};

class FsBroker : public Interface {
  public:
  FsBroker() 
    : Interface("\033[32mSWC-DB(\033[36mfsbroker\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-fsbroker-history") {
  }
};

class DbClient : public Interface {
  public:
  DbClient() 
    : Interface("\033[32mSWC-DB(\033[36mclient\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-dbclient-history") {
    options.push_back(
      new Option("select", "select [where_clause [Columns-Intervals or Cells-Intervals]] [Flags];", 
                  [ptr=this](const std::string& line){return ptr->select(line);}, 
                  new re2::RE2("(?i)(^|\\s+)select.*"))
    );
  }

  bool select(const std::string& line) {
    
    std::cout << "select(cmd) '" << line << "'\n";
    return true;
  }
};



}}} // namespace SWC::Utils::shell

#endif // swc_lib_utils_Shell_h