/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_utils_Shell_h
#define swc_lib_utils_Shell_h

#include "swcdb/core/config/Settings.h"

#include "histedit.h"

#include <editline/readline.h>

extern "C" {
int   swc_utils_run();
void  swc_utils_apply_cfg(SWC::Env::Config::Ptr env);
}

namespace SWC { namespace Utils { namespace shell {

int run();

namespace Rgr {
int run() {
  std::cout << "Running SWC::Utils::shell::Rgr\n";  
  return 0;
}
}
namespace Mngr {
int run() {
  std::cout << "Running SWC::Utils::shell::Mngr\n"; 
  return 0;
}
}
namespace FsBroker {
int run() {
  std::cout << "Running SWC::Utils::shell::FsBroker\n";  
  return 0;
}
}
namespace DbClient {
  
const char* s_prompt = "SWC-DB(client) # ";

const char* prompt(EditLine *e) {
  return s_prompt;
}

int run() {
  std::cout << "Running SWC::Utils::shell::DbClient\n";  
  std::string s;
  std::cout << s_prompt << std::flush;
  std::cin >> s;
  std::cout << s_prompt << std::flush;
  std::cout << "READ '" << s << "'\n"; 
  std::cout << "finished - std::cin/cout\n";  

  /* This holds all the state for our line editor */
  EditLine *el;

  /* This holds the info for our history */
  History *myhistory;

  /* Temp variables */
  int count;
  const char *line;
  HistEvent ev;

  /* Initialize the EditLine state to use our prompt function and
  emacs style editing. */
    
  el = el_init("", stdin, stdout, stderr);
  el_set(el, EL_PROMPT, &prompt);
  el_set(el, EL_EDITOR, "emacs");

  /* Initialize the history */
  myhistory = history_init();
  if (myhistory == 0) {
    fprintf(stderr, "history could not be initialized\n");
    return 1;
  }

  /* Set the size of the history */
  history(myhistory, &ev, H_SETSIZE, 800);

  /* This sets up the call back functions for history functionality */
  el_set(el, EL_HIST, history, myhistory);

  while((line = el_gets(el, &count)) && count > 0) {
    if(count == 1)
      continue;

    history(myhistory, &ev, H_ENTER, line);

    if(strcmp(line, "quit;\n") == 0)
      break;
  }
  
  /* Clean up our memory */
  history_end(myhistory);
  el_end(el);
  
  return 0;
}
}

}}} // namespace SWC::Utils::shell

#endif // swc_lib_utils_Shell_h