/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/utils/cli/Shell.h"
#include "swcdb/utils/cli/Shell_DbClient.h"
#include "swcdb/utils/cli/Shell_Manager.h"
#include "swcdb/utils/cli/Shell_Ranger.h"
#include "swcdb/utils/cli/Shell_Fs.h"
#include "swcdb/utils/cli/Shell_Statistics.h"
#include <iomanip>
#include <queue>


#if defined(USE_REPLXX)
  #include <fstream>
  #include "replxx.hxx"
#elif defined(USE_GNU_READLINE)
  #include <readline/readline.h>
  #include <readline/history.h>
#else
  #include <editline.h> // github.com/troglobit/editline
#endif


int el_hist_size = 4000;


namespace SWC { namespace Utils { namespace shell {


int run() {
  try {
    uint8_t cli;
    {
    auto settings = SWC::Env::Config::settings();
    if(settings->has("ranger"))
      cli = CLI::RANGER;
    else if(settings->has("manager"))
      cli = CLI::MANAGER;
    else if(settings->has("filesystem"))
      cli = CLI::FILESYSTEM;
    else if(settings->has("statistics"))
      cli = CLI::STATISTICS;
    else
      cli = CLI::DBCLIENT;
    }
    for(std::unique_ptr<Interface> ptr; cli; cli=ptr->run()) {
      ptr.reset(nullptr);
      switch(cli) {
        case CLI::DBCLIENT:
          ptr.reset(new DbClient);
          break;
        case CLI::MANAGER:
          ptr.reset(new Mngr);
          break;
        case CLI::RANGER:
          ptr.reset(new Rgr);
          break;
        case CLI::FILESYSTEM:
          ptr.reset(new Fs);
          break;
        case CLI::STATISTICS:
          ptr.reset(new Statistics);
          break;
      }
    }
  } catch(...) {
    SWC_PRINT << SWC_CURRENT_EXCEPTION("") << SWC_PRINT_CLOSE;
    SWC_CAN_QUICK_EXIT(EXIT_FAILURE);
    return 1;
  }
  SWC_CAN_QUICK_EXIT(EXIT_SUCCESS);
  return 0;
}



struct Interface::Option final {
  Option(std::string&& a_name, Core::Vector<std::string>&& a_desc,
         OptCall_t&& a_call, const char* a_re) noexcept
        : name(std::move(a_name)),
          desc(std::move(a_desc)),
          call(std::move(a_call)),
          re(a_re) {
  }
  ~Option() noexcept { }
  const std::string               name;
  const Core::Vector<std::string> desc;
  const OptCall_t                 call;
  const re2::RE2                  re;
};



Interface::Interface(std::string&& a_prompt, std::string&& a_history, CLI state)
                    : err(Error::OK), _state(state),
                      prompt(std::move(a_prompt)),
                      history(std::move(a_history)),
                      options() {
  init();
}

Interface::~Interface() {
  for(auto o : options)
    delete o;
}

CLI Interface::run() {
  #if defined(USE_REPLXX)
    replxx::Replxx rx;
    rx.install_window_change_handler();
	  rx.set_max_history_size(el_hist_size);
	  rx.set_max_hint_rows(3);
	  rx.set_indent_multiline(true);
	  rx.set_prompt(prompt);
    {
	  	std::ifstream history_file(history.c_str());
		  rx.history_load(history_file);
	  }
    rx.bind_key(
      replxx::Replxx::KEY::control( 'C' ),
      [this, &rx](char32_t c) {
        _state.store(CLI::QUIT_CLI);
        return rx.invoke(replxx::Replxx::ACTION::ABORT_LINE, c);
      }
    );
    const char* line = nullptr;
    const char* ptr = nullptr;
  #else
    read_history(history.c_str());
    char* line = nullptr;
    char* ptr = nullptr;
  #endif

  bool prompt_state = true;
  char c;

  bool stop = false;
  bool cmd_end = false;
  bool escape = false;
  bool comment = false;
  bool quoted_1 = false;
  bool quoted_2 = false;
  bool is_quoted = false;
  bool next_line = false;
  std::string cmd;
  std::queue<std::string> queue;

  errno = 0;
  while(!stop && _state != CLI::QUIT_CLI &&
  #if defined(USE_REPLXX)
        (ptr = line = rx.input(prompt_state ? prompt : std::string()))
  #else
        (ptr = line = readline(prompt_state ? prompt.c_str() : ""))
  #endif
        && _state != CLI::QUIT_CLI) {

    prompt_state = false;
    do {

      if(errno) {
        auto errtmp = errno;
        errno = 0;
        if(errtmp != ENOTTY && errtmp != EINTR) {
          if(errtmp != EAGAIN) {
            SWC_PRINT << "\033[31mERROR\033[00m: unexpected error="
                      << errtmp << '(' << Error::get_text(errtmp) << ')'
                      << SWC_PRINT_CLOSE;
          }
          break;
        }
      }

      c = *ptr;
      ++ptr;
      if((next_line = !c))
        c = '\n';

      if(c == '\n' && cmd_end) {
        while(!queue.empty()) {
          auto& run_cmd = queue.front();
          #if defined(USE_REPLXX)
            rx.history_add(run_cmd);
	          rx.history_sync(history);
          #else
            add_history(run_cmd.c_str());
            write_history(history.c_str());
          #endif
          run_cmd.pop_back();
          if((stop = !cmd_option(run_cmd)))
            break;
          queue.pop();
        }
        cmd_end = false;
        prompt_state = true;
        break;

      } else if(!comment && c == '\n' && cmd.empty()) {
        prompt_state = true;
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

    #if !defined(USE_REPLXX)
      free(line);
    #endif
  }

  return _state;
}

void Interface::add_option(const char* a_name,
                           Core::Vector<std::string>&& a_desc,
                           OptCall_t&& a_call,
                           const char* a_re) {
  options.emplace_back(new Option(
    std::move(a_name), std::move(a_desc), std::move(a_call), a_re));
}

void Interface::init() {
  add_option(
    "quit",
    {"Quit or Exit the Console"},
    [ptr=this](std::string& cmd){return ptr->quit(cmd);},
    "(?i)^(quit|exit)(\\s+|$)"
  );
  add_option(
    "help",
    {"Commands help information"},
    [ptr=this](std::string& cmd){return ptr->help(cmd);},
    "(?i)^(help)(\\s+|$)"
  );
  add_option(
    "switch to",
    {"Switch to other CLI, options: rgr|mngr|fs|stats|client"},
    [ptr=this](std::string& cmd){return ptr->switch_to(cmd);},
    "(?i)^(switch\\s+to)(\\s+|$)"
  );
}

bool Interface::switch_to(std::string& cmd)  {
  std::string message;
  client::SQL::Reader parser(cmd, message);
  bool found;

  parser.seek_space();
  parser.expect_token("switch", 6, found = false);
  if(parser.err) {
    err = parser.err;
    return error(message);
  }
  parser.seek_space();
  parser.expect_token("to", 2, found = false);
  if(parser.err) {
    err = parser.err;
    return error(message);
  }
  parser.seek_space();
  if(parser.found_token("client", 6)) {
    _state.store(CLI::DBCLIENT);
  } else if(parser.found_token("mngr", 4)) {
    _state.store(CLI::MANAGER);
  } else if(parser.found_token("rgr", 3)) {
    _state.store(CLI::RANGER);
  } else if(parser.found_token("fs", 2)) {
    _state.store(CLI::FILESYSTEM);
  } else if(parser.found_token("stats", 5)) {
    _state.store(CLI::STATISTICS);
  } else {
    err = Error::INVALID_ARGUMENT;
    return error("Bad Interface Option");
  }
  return false;
}

bool Interface::help(std::string&) const {
  SWC_PRINT << "Usage Help:  \033[4m'command' [options];\033[00m\n";
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
    SWC_LOG_OSTREAM << std::left << std::setw(2) << " "
                    << std::left << std::setw(offset_name) << opt->name;
    first = true;
    for(const auto& desc : opt->desc) {
      if(!first)
        SWC_LOG_OSTREAM << std::left << std::setw(2) << " "
                        << std::left << std::setw(offset_name) << " ";
      SWC_LOG_OSTREAM << std::setw(offset_desc) << desc << std::endl;
      first = false;
    }

  }
  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  return true;
}

bool Interface::error(const std::string& message) {
  SWC_PRINT << "\033[31mERROR\033[00m: " << message
            << " error=" << err << '(' << Error::get_text(err) << ')'
            << SWC_PRINT_CLOSE;
  return true; /// ? err
}

bool Interface::cmd_option(std::string& cmd) const {
  err = Error::OK;
  auto opt = std::find_if(options.cbegin(), options.cend(),
              [cmd](const Option* _opt){
                return Condition::re(_opt->re, cmd.c_str(), cmd.size());
              });
  if(opt != options.cend())
    return (*opt)->call(cmd);
  SWC_PRINT << "Unknown command='\033[31m" << cmd << ";\033[00m'"
            << SWC_PRINT_CLOSE;
  return true;
}


}}} // namespace SWC::Utils::shell

extern "C" {
int swcdb_utils_run() {
  return SWC::Utils::shell::run();
}
void swcdb_utils_apply_cfg(SWC::Env::Config::Ptr env){
  SWC::Env::Config::set(env);
}
}
