/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_utils_Shell_h
#define swc_utils_Shell_h

#include "swcdb/core/config/Settings.h"

#include <re2/re2.h>
#include <termios.h>
#include <term.h>


extern "C" {
int   swc_utils_run();
void  swc_utils_apply_cfg(SWC::Env::Config::Ptr env);
}

namespace SWC { namespace Utils { namespace shell {

int run();

class Interface {

  public:
  Interface(const std::string& prompt="CLI>", 
            const char* history="/tmp/.swc-cli-history")
            : prompt(prompt), history(history), 
              fd_o(1), fd_i(0) {
                
    screen_cols = tgetnum("co");
    if(screen_cols <= 0)
      screen_cols = 80;

    screen_buf = screen_ptr = new uint8_t[screen_sz = screen_cols];
    
    tcgetattr(fd_i, &tc_attrs_in_orig);
    tc_attrs_in = tc_attrs_in_orig;

    tc_attrs_in.c_lflag &= ~(ICANON | ECHO);

    tc_attrs_in.c_cflag &= ~CSIZE;
    tc_attrs_in.c_cflag |= CS8;

    tc_attrs_in.c_iflag &= ~INPCK;
    tc_attrs_in.c_iflag |= ISTRIP | IUTF8;

    tc_attrs_in.c_cc[VMIN] = 1;
    tc_attrs_in.c_cc[VTIME] = 0;

    tcsetattr(fd_i, TCSANOW, &tc_attrs_in);
  }
  
  virtual ~Interface() {
    tcsetattr(fd_i, TCSANOW, &tc_attrs_in_orig);

    for(auto o : options)
      delete o;
    delete screen_buf;
  }

  void flush() {
    if(screen_ptr > screen_buf) {
      const uint8_t* ptr = screen_buf;
      do {
        errno = 0;
      } while((ptr+=write(fd_o, ptr, screen_ptr-ptr)) < screen_ptr && !errno);
      screen_ptr = screen_buf;
    }
  }

  void display_prompt() {
    memcpy(screen_ptr, prompt.data(), prompt.length());
    screen_ptr += prompt.length();
    flush();
  }

  void output(const uint8_t c) {
    if(screen_ptr == screen_buf+screen_sz) {
      uint8_t* old = screen_buf;
      uint32_t offset = screen_ptr-old;
      screen_buf = new uint8_t[screen_sz+=screen_cols];
      memcpy(screen_buf, old, offset);
      screen_ptr = screen_buf + offset;
      delete old;
    }
    *screen_ptr++ = c;
  }


  int run() { 
  
    //read_history(history);

    //rl_initialize();

    display_prompt();
    
    bool ctrl;
    bool cmd_end;
    bool comment;
    bool quoted_1 = false;
    bool quoted_2 = false;
    std::string cmd;
    char c;
    while(::read(fd_i, &c, 1) == 1) {
      if(c == '[') {
        ctrl = true;
        continue;
      }
      if(ctrl) {
        ctrl = false;
        // act
        continue;
      }

      output(c);
      flush();

      if(c == '\n' && cmd_end) {
        //add_history(cmd.c_str());
        if(!cmd_option(cmd))
          break;
        cmd.clear();
        cmd_end = false;
        display_prompt();
        continue;

      } else if(c == '\n' && cmd.empty()) {
        display_prompt();
        continue;

      } else if(c == ' ' && cmd.empty()) {
        continue;

      } else if(!quoted_1 && !quoted_2) {
        if(c == '#') {
          comment = true;
          continue;
        } else if(c == '\n' && comment) {
          comment = false;
          continue;
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
    }
    
    //rl_uninitialize();

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


  private:

  bool cmd_option(const std::string& line) {
    auto opt = std::find_if(options.begin(), options.end(), 
                [line](const Option* opt){ 
                  return RE2::PartialMatch(line.c_str(), *opt->re); 
                });
    if(opt != options.end()) {
      printf("\t\t\tran|%s|\n", line.c_str());
      return (*opt)->call(line);
    } else {
      std::cout << "Unknown command='\033[31m" << line << "\033[00m'\n";
    }
    return true;
  }
  
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
  const std::string prompt;
  const char*       history;
  int               fd_o, fd_i;
  int32_t           screen_cols, screen_sz;
  uint8_t*          screen_buf;
  uint8_t*          screen_ptr;

  struct termios    tc_attrs_in_orig;
  struct termios    tc_attrs_in;
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
      new Option("select", "query, select [where_clause [Columns-Intervals or Cells-Intervals]] [Flags];", 
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

#endif // swc_utils_Shell_h