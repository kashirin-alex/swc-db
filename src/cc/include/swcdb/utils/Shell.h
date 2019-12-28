/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_utils_Shell_h
#define swc_lib_utils_Shell_h

#include "swcdb/core/config/Settings.h"

#include "swcdb/core/comm/Settings.h"

#include "swcdb/client/Clients.h"
#include "swcdb/client/AppContext.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Common/req/Query.h"

#include "swcdb/db/Cells/SpecsBuilderSql.h"

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
    bool cmd_end = false;
    bool escape = false;
    bool comment = false;;
    bool quoted_1 = false;
    bool quoted_2 = false;
    bool is_quoted = false;
    bool next_line = false;
    std::string cmd;
    std::queue<std::string> queue;

    while(!stop && (ptr = line = readline(prompt_state))) {
      //std::cout << "line=" << line << "\n";
      prompt_state = ""; // "-> ";
      do {
        c = *ptr++;
        if(next_line = c == 0)
          c = '\n';

        //std::cout << " c=" << c << "(" << (int)c << ") \n";
        if(c == '\n' && cmd_end) {
          while(!queue.empty()) {
            auto& run_cmd = queue.front();
            add_history(run_cmd.c_str());
            write_history(history);
            run_cmd.pop_back();
            if(stop = !cmd_option(run_cmd))
              break;
            queue.pop();
          }
          cmd_end = false;
          prompt_state = prompt;
          break;

        } else if(c == '\n' && cmd.empty()) {
          prompt_state = prompt;
          break;

        } else if(c == ' ' && cmd.empty()) {
          continue;

        } else if(!is_quoted && !escape) {
          if(c == '#') {
            comment = true;
            continue;
          } else if(c == '\n' && comment) {
            comment = false;
            break;
          } else if(comment)
            continue;
        }

        cmd += c;

        if(escape) {
          escape = false;
          continue;
        } else if(c == '\\') {
          escape = true;
          continue;
        }
        
        if((!is_quoted || quoted_1) && c == '\'')
          is_quoted = quoted_1 = !quoted_1;
        else if((!is_quoted || quoted_2) && c == '"')
          is_quoted = quoted_2 = !quoted_2;
        else if(c == ';' && !is_quoted) {
          cmd_end = true;
          queue.push(cmd);
          cmd.clear();
        }

      } while(!next_line);
      
	    free(line);
    }

    return 0;
  }
  

  protected:

  struct Option final {
    typedef std::function<bool(std::string&)> Call_t;

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
    new Option(
      "quit", 
      "Quit or Exit the Console", 
      [ptr=this](std::string& cmd){return ptr->quit(cmd);}, 
      new re2::RE2("(?i)^(quit|exit)(\\s+|$)")
    ),
    new Option(
      "help", 
      "Commands help information", 
      [ptr=this](std::string& cmd){return ptr->help(cmd);}, 
      new re2::RE2("(?i)^(help)(\\s+|$)")
    )
  };
  
  virtual bool quit(std::string& cmd) const {
    return false;
  }

  virtual bool help(std::string& cmd) const {
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

  const bool cmd_option(std::string& cmd) const {
    auto opt = std::find_if(options.begin(), options.end(), 
                [cmd](const Option* opt){ 
                  return RE2::PartialMatch(cmd.c_str(), *opt->re); 
                });
    if(opt != options.end())
      return (*opt)->call(cmd);
    std::cout << "Unknown command='\033[31m" << cmd << ";\033[00m'\n";
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
      new Option(
        "select", 
        "select [where_clause [Columns-Intervals or Cells-Intervals]] [Flags];",
        [ptr=this](std::string& cmd){return ptr->select(cmd);}, 
        new re2::RE2("(?i)^(select)(\\s+|$)")
      )
    );
  
    SWC::Env::Clients::init(
      std::make_shared<SWC::client::Clients>(
        nullptr,
        std::make_shared<SWC::client::AppContext>()
      )
    );
  }

  static void display(Protocol::Common::Req::Query::Select::Result::Ptr result) {
    std::cout << "CB completion=" << result->completion.load() << "\n";
    for(auto col : result->columns) {
      std::cout << " cid=" << col.first 
                << ": sz=" << col.second->cells.size() << "\n";
    int num=0;
    for(auto cell : col.second->cells)
      std::cout << "  " << ++num << ":" << cell->to_string() << "\n";  
    }
  }

  const bool error(int err, const std::string& message) {
    std::cout << "\033[31mERROR\033[00m: " << message;
    return true;
  }

  const bool select(std::string& cmd) {
    int err = Error::OK;
    auto req = std::make_shared<Protocol::Common::Req::Query::Select>(display);
    std::string message;
    DB::Specs::Builder::parse_sql_select(err, cmd, req->specs, message);
    if(err) 
      return error(err, message);

    req->scan();
    req->wait();
    // time-took ?with
    return true;
  }
};




}}} // namespace SWC::Utils::shell

#endif // swc_lib_utils_Shell_h