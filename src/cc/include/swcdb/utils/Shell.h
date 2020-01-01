/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_utils_Shell_h
#define swc_lib_utils_Shell_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/client/sql/SQL.h"

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

        } else if(!comment && c == '\n' && cmd.empty()) {
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

    Option(const std::string& name, const std::vector<std::string>& desc, 
            const Call_t& call, const re2::RE2* re) 
          : name(name), desc(desc), call(call), re(re) { }
    ~Option() {
      if(re)
        delete re;
    }
    const std::string              name;
    const std::vector<std::string> desc;
    const Call_t      call;
    const re2::RE2*   re;
  };

  std::vector<Option*> options {
    new Option(
      "quit", 
      {"Quit or Exit the Console"}, 
      [ptr=this](std::string& cmd){return ptr->quit(cmd);}, 
      new re2::RE2("(?i)^(quit|exit)(\\s+|$)")
    ),
    new Option(
      "help", 
      {"Commands help information"}, 
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
      for(const auto& desc : opt->desc) {
        if(offset_desc < desc.length())
          offset_desc = desc.length();
      }
    }
    offset_name += 4;
    offset_desc += 4;
    bool first;
    for(auto opt : options) {
      std::cout << std::left << std::setw(2) << " "
                << std::left << std::setw(offset_name) << opt->name;
      first = true;
      for(const auto& desc : opt->desc) {
        if(!first)
          std::cout << std::left << std::setw(2) << " "
                    << std::left << std::setw(offset_name) << " ";
        std::cout << std::setw(offset_desc) << desc << std::endl;
        first = false;
      }
        
    }
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
        "add column", 
        {"add column|schema (schema definitions [name=value ]);"},
        [ptr=this](std::string& cmd){
          return ptr->mng_column(
            Protocol::Mngr::Req::ColumnMng::Func::CREATE, cmd);
        }, 
        new re2::RE2(
          "(?i)^(add|create)\\s+(column|schema)(.*|$)")
      )
    ); 
    options.push_back(
      new Option(
        "modify column", 
        {"modify column|schema (schema definitions [name=value ]);"},
        [ptr=this](std::string& cmd){
          return ptr->mng_column(
            Protocol::Mngr::Req::ColumnMng::Func::MODIFY, cmd);
        }, 
        new re2::RE2(
          "(?i)^(modify|change|update)\\s+(column|schema)(.*|$)")
      )
    ); 
    options.push_back(
      new Option(
        "delete column", 
        {"delete column|schema (schema definitions [name=value ]);"},
        [ptr=this](std::string& cmd){
          return ptr->mng_column(
            Protocol::Mngr::Req::ColumnMng::Func::DELETE, cmd);
        }, 
        new re2::RE2(
          "(?i)^(delete|remove)\\s+(column|schema)(.*|$)")
      )
    ); 
    options.push_back(
      new Option(
        "list columns", 
        {"list|get column|s [NAME|ID,..];"},
        [ptr=this](std::string& cmd){return ptr->list_columns(cmd);}, 
        new re2::RE2(
          "(?i)^(get|list)\\s+((column(|s))|(schema|s))(.*|$)")
      )
    );    
    options.push_back(
      new Option(
        "select", 
        {"select where [Columns[Cells[Interval Flags]]] Flags Display-Flags;",
        "-> select where COL(NAME|ID,) = ( cells=(Interval Flags) ) AND",
        "     COL(NAME-2|ID-2,) = ( cells=(Interval Flags) AND cells=(",
        "       [F-begin] <= range <= [F-end]                   AND", 
        "       [COMP 'F-start'] <=  key  <= [COMP 'F-finish']  AND", 
        "       'TS-begin' <= timestamp <= 'TS-finish'          AND", 
        "       offset_key = [F] offset_rev='TS'                AND", 
        "       value COMP 'DATA'                                  ",
        "       LIMIT=NUM   OFFSET=NUM  ONLY_KEYS   ONLY_DELETES     )",
        "     ) DISPLAY_* TIMESTAMP / DATETIME / SPECS / STATS / BINARY;"},
        [ptr=this](std::string& cmd){return ptr->select(cmd);}, 
        new re2::RE2(
          "(?i)^(select)(\\s+|$)")
      )
    ); 
    options.push_back(
      new Option(
        "update", 
        {"update cell(FLAG, CID|NAME, KEY, TIMESTAMP, VALUE), CELL(..)      ;",
        "-> update ",
        "     cell(DELETE,                  CID, ['K','E','Y']             );",
        "     cell(DELETE_VERSION,          CID, ['K','E','Y'], TS         );",
        "     cell(DELETE_FRACTION,         CID, ['K','E','Y']             );",
        "     cell(DELETE_FRACTION_VERSION, CID, ['K','E'],     TS         );",
        "     cell(INSERT,                  CID, ['K','E','Y'], ASC, TS, ''),",
        "     cell(INSERT,                  CID, ['K','E','Y'], DESC       ),",
        "     cell(INSERT,                 NAME, ['K','E','Y'], '', 'DATA' ),",
        "     cell(INSERT_FRACTION,        NAME, ['K','E'],     '', 'DATA' );"
        },
        [ptr=this](std::string& cmd){return ptr->update(cmd);}, 
        new re2::RE2(
          "(?i)^(update)(\\s+|$)")
      )
    );
  
    Env::Clients::init(
      std::make_shared<client::Clients>(
        nullptr,
        std::make_shared<client::AppContext>()
      )
    );

  }

  const bool error(int err, const std::string& message) {
    std::cout << "\033[31mERROR\033[00m: " << message << std::flush;
    return true;
  }

  const bool mng_column(Protocol::Mngr::Req::ColumnMng::Func func, 
                        std::string& cmd) {
    std::string message;
    DB::Schema::Ptr schema;
    int err = Error::OK;
    client::SQL::parse_column_schema(err, cmd, func, schema, message);
    if(err)
      return error(err, message);

    std::promise<int> res;
    Protocol::Mngr::Req::ColumnMng::request(
      func,
      schema,
      [await=&res]
      (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, int err) {
        /*if(err && Func::CREATE && err != Error::COLUMN_SCHEMA_NAME_EXISTS) {
          req->request_again();
          return;
        }*/
        await->set_value(err);
      },
      300000
    );
    
    if(err = res.get_future().get()) {
      message.append(Error::get_text(err));
      message.append("\n");
      return error(err, message);
    }
    if(schema->cid != DB::Schema::NO_CID)
      Env::Clients::get()->schemas->remove(schema->cid);
    else
      Env::Clients::get()->schemas->remove(schema->col_name);
    return true;
  }
  
  const bool list_columns(std::string& cmd) {
    int err = Error::OK;
    std::vector<DB::Schema::Ptr> schemas;  
    std::string message;
    client::SQL::parse_list_columns(err, cmd, schemas, message);
    if(err) 
      return error(err, message);

    if(schemas.empty()) { // get all schema
      std::promise<int> res;
      Protocol::Mngr::Req::ColumnList::request(
        [&schemas, await=&res]
        (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, int err, 
         Protocol::Mngr::Params::ColumnListRsp rsp) {
          if(!err)
            schemas = rsp.schemas;
          await->set_value(err);
        },
        300000
      );
      if(err = res.get_future().get()) {
        message.append(Error::get_text(err));
        message.append("\n");
        return error(err, message);
      }
    }

    for(auto& schema : schemas) {
      schema->display(std::cout);
      std::cout << std::endl;
    }
    return true;
  }

  void display(Protocol::Common::Req::Query::Select::Result::Ptr result,
               uint8_t display_flags, 
               size_t& cells_count, size_t& cells_bytes) const {
    DB::Schema::Ptr schema = 0;
    int err = Error::OK;
    DB::Cells::Vector vec; 
    do {
      for(auto cid : result->get_cids()) {
        schema = Env::Clients::get()->schemas->get(err, cid);
        vec.free();
        result->get_cells(cid, vec);
        for(auto& cell : vec.cells) {
          cells_count++;
          cells_bytes += cell->encoded_length();
          cell->display(
            std::cout, 
            err ? Types::Column::PLAIN: schema->col_type,
            display_flags
          );
          std::cout << "\n";  
          delete cell;
          cell = nullptr;
        }
      }
    } while(!vec.cells.empty());
  }

  const bool select(std::string& cmd) {
    int64_t ts = Time::now_ns();
    uint8_t display_flags = 0;
    size_t cells_count = 0;
    size_t cells_bytes = 0;
    auto req = std::make_shared<Protocol::Common::Req::Query::Select>(
      [this, &display_flags, &cells_count, &cells_bytes]
      (Protocol::Common::Req::Query::Select::Result::Ptr result) {
        display(result, display_flags, cells_count, cells_bytes);
      },
      true // cb on partial rsp
    );
    int err = Error::OK;
    std::string message;
    client::SQL::parse_select(err, cmd, req->specs, display_flags, message);
    if(err) 
      return error(err, message);

    if(display_flags & DB::DisplayFlag::SPECS) {
      std::cout << "\n\n";
      req->specs.display(
        std::cout, !(display_flags & DB::DisplayFlag::BINARY));
    }

    req->scan();
    req->wait();

    if(display_flags & DB::DisplayFlag::STATS)
      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);

    return true;
  }

  const bool update(std::string& cmd) {
    int64_t ts = Time::now_ns();
    uint8_t display_flags = 0;

    auto req = std::make_shared<Protocol::Common::Req::Query::Update>();
    auto req_fraction = std::make_shared<Protocol::Common::Req::Query::Update>();
    int err = Error::OK;
    std::string message;
    client::SQL::parse_update(
      err, cmd, 
      *req->columns.get(), *req_fraction->columns.get(), 
      display_flags, message
    );
    if(err) 
      return error(err, message);
    
    size_t cells_count = req->columns->size() 
                       + req_fraction->columns->size();
    size_t cells_bytes = req->columns->size_bytes() 
                       + req_fraction->columns->size_bytes();

    req->timeout_commit += 10*cells_count;
    req->commit();
    req->wait();

    // req->result->errored
    if(display_flags & DB::DisplayFlag::STATS) 
      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);
    return true;
  }

  void display_stats(double took, double bytes, size_t cells_count) {      
    std::cout << "\n\nStatistics:\n";

    char time_base = 'n';
    if(took > 100000 && took < 10000000) { 
      took /= 1000;
      time_base = 'u';
    } else if(took < 10000000000) { 
      took /= 1000000;
      time_base = 'm';
    } else  if(took > 10000000000) {
      took /= 1000000000;
      time_base = 0;
    }

    char byte_base = 0;
    if(bytes > 1000000 && bytes < 1000000000) {
      bytes /= 1000;
      byte_base = 'K';
    } else if (bytes > 1000000000) {
      bytes /= 1000000;
      byte_base = 'M';
    }
        
    std::cout 
      << " Total Time Took:        " << took << " " << time_base  << "s\n"
      << " Total Cells Count:      " << cells_count                << "\n"
      << " Total Cells Size:       " << bytes << " " << byte_base << "B\n"
      << " Average Transfer Rate:  " << bytes/took 
                         << " " << byte_base << "B/" << time_base << "s\n" 
      << " Average Cells Rate:     " << (cells_count?cells_count/took:0)
                                         << " cell/" << time_base << "s\n"
    ;

  }

};




}}} // namespace Utils::shell

#endif // swc_lib_utils_Shell_h