/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_lib_utils_Shell_h
#define swc_lib_utils_Shell_h

#include "swcdb/core/config/Settings.h"

#include <re2/re2.h>
#include <ncurses.h>

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
            : prompt(prompt), history(history) {
    ::initscr(); 
    ::cbreak(); 
    ::noecho();
    //::nonl();
    ::intrflush(stdscr, FALSE);
    ::keypad(stdscr, TRUE);
    //::wclear(stdscr);
    
    ::start_color();	
    ::init_pair(1, COLOR_GREEN, COLOR_BLACK);
    ::init_pair(2, COLOR_CYAN, COLOR_BLACK);
    ::init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    ::init_pair(4, COLOR_RED, COLOR_BLACK);
  }
  
  virtual ~Interface() {
    for(auto o : options)
      delete o;
    ::wclear(stdscr);
    ::endwin();
  }

  void display_prompt() {
    attron(COLOR_PAIR(1));	
    addstr("SWC-DB(");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(2));	
    addnstr(prompt.data(), prompt.length());
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(1));	
    addch(')');
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(3));	
    addch('>');
    addch(' ');
    attroff(COLOR_PAIR(3));
    //::refresh();
	  ::doupdate();
  }

  int run() { 
  
    //read_history(history);

    //rl_initialize();

    display_prompt();
    
    bool cmd_end  = false;
    bool comment  = false;
    bool quoted_1 = false;
    bool quoted_2 = false;
    std::string cmd;
    int c;
    while(1) {
      errno = 0;
      c = getch();
      if(errno)
        break;

      switch (c) {
        case KEY_BACKSPACE:
        erase();
          continue;
        case KEY_UP:
        erase();
          break;
        case KEY_DOWN:
        erase();
          break;
        default:
          break;
      }

      addch(c);

      if(c == '\n' && cmd_end) {
        //add_history(cmd.c_str());
        //::endwin();
        //::erase();
        if(!cmd_option(cmd))
          break;
        //::initscr(); 
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

      char chr = (char)c;
      cmd.append(std::string(&chr, 1));

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
      return (*opt)->call(line);
    } else {
      addstr(" Unknown command='");
      attron(COLOR_PAIR(4));	
      addnstr(line.data(), line.length());
      attroff(COLOR_PAIR(4));
      addstr("'\n");
    }
    return true;
  }
  
  virtual bool quit(const std::string& line) {
    return false;
  }

  virtual bool help(const std::string& line) {
    addstr("Usage Help:  ");
    attron(A_UNDERLINE);
    addstr("'command' [options];");
    attroff(A_UNDERLINE);
 
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
    for(auto opt : options) {
      addstr("\n  ");
      addnstr(opt->name.data(), opt->name.length());
      for(int n=offset_name;n>opt->name.length();n--)
        addch(' ');
      addnstr(opt->desc.data(), opt->desc.length());
      for(int n=offset_desc;n>opt->desc.length();n--)
        addch(' ');
    }
    addch('\n');
	  ::doupdate();
    return true;
  }


  private:
  const std::string prompt;
  const char*       history;
}; 




class Rgr : public Interface {
  public:
  Rgr() 
    : Interface("rgr", "/tmp/.swc-cli-ranger-history") {
  }
};

class Mngr : public Interface {
  public:
  Mngr() 
    : Interface("mmngr", "/tmp/.swc-cli-manager-history") {
  }
};

class FsBroker : public Interface {
  public:
  FsBroker() 
    : Interface("fsbroker", "/tmp/.swc-cli-fsbroker-history") {
  }
};

class DbClient : public Interface {
  public:
  DbClient() 
    : Interface("client", "/tmp/.swc-cli-dbclient-history") {
    options.push_back(
      new Option("select", "query, select [where_clause [Columns-Intervals or Cells-Intervals]] [Flags];", 
                  [ptr=this](const std::string& line){return ptr->select(line);}, 
                  new re2::RE2("(?i)(^|\\s+)select.*"))
    );
  }

  bool select(const std::string& line) {
    
    addstr("select(cmd) '");
    addnstr(line.data(), line.length());
    addstr("'\n");
	  ::doupdate();
    return true;
  }
};



}}} // namespace SWC::Utils::shell

#endif // swc_lib_utils_Shell_h