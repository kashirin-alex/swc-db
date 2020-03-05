/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/core/config/Settings.h"

namespace SWC{ namespace Config {

void Settings::init_app_options() {
  cmdline_desc
  .definition("Usage: %s [Options] [args]\nOptions")
  .add_options()
   ("i16", i16(1), "16-bit integer")
   ("i32", i32(), "32-bit integer")
   ("i64", i64(1), "64-bit integer")
   ("int64s", i64s(), "a list of 64-bit integers")
   ("boo,b", boo(false), "a boolean arg")
   ("string,str,s", str("A Default std::string Value"), "a string arg")
   ("strs", strs(), "a list of strings")
   ("float64", f64(3.12), "a double arg")
   ("f64s", f64s(), "a list of doubles")
   ("skip1", boo()->zero_token(), "a skipped token")
   ("skip2", "a skipped token")
   ("false", boo(false)->zero_token(), "token defaults to false")
   ("true", boo(true)->zero_token(), "token defaults to true")
   ("is_true", boo(false)->zero_token(), "if set token true")

   ("app",    str(),  "application filename, zero pos skipped")
   ("app",    0)
   ("args",  -1 ) // -1, accumalate unknonw options
   ("args",   strs(), "rest of arguments")
   ("arg-1",  str(),  "argument one")
   ("arg-1",  1)
   ;   
   //("a.cmd.arg.qouted", str(), "a qouted string arg with spaces") require cmake escaping to testdiff  
}

void Settings::init_post_cmd_args(){ }
} }

using namespace SWC;

int main(int argc, char *argv[]) {
  Env::Config::init(argc, argv);
  auto settings = Env::Config::settings();
  
  // remove env-dynamic configs
  settings->cmdline_desc.remove("swc.logging.path");
  settings->cmdline_desc.remove("swc.cfg.path");
  

  std::cout << std::string("\nConfig::cmdline_desc");
  std::cout << settings->cmdline_desc;
  settings->print(std::cout);

  
  // cfg file parse test definitions
  settings->file_desc.add_options()
   ("is_true_bool", boo(false), "a boolean arg")
   ("is_yes_bool", boo(false), "a boolean arg")
   ("is_one_bool", boo(false), "a boolean arg")
   ("is_no_bool", boo(true), "a boolean arg")
   ("is_rest_bool", boo(true), "a boolean arg")

   ("boo", boo(false), "a boolean arg")
   ("i16", i16(1), "16-bit integer")
   ("i32", i32(), "32-bit integer")
   ("i64", i64(1), "64-bit integer")
   ("int64s", i64s(), "a list of 64-bit integers")
   ("string", str("A Default std::string Value"), "a string arg")
   ("string.qouted", str("A Default std::string Value"), "a string arg")
   ("strs", strs(), "a list of strings")
   ("float64", f64(3.12), "a double arg")
   ("f64s", f64s(), "a list of doubles")
   ("missing", str("a missing string in cfg file, default applies"), "a string arg")
   ("aGroupOne.arg.1", i16(0), "a group arg")
   ("aGroupOne.arg.2", i16(0), "a group arg")
   ("aGroupOne.arg.3", i16(0), "a group arg")
   ("aGroupOne.arg.4", i16(0), "a group arg")
   ("aGroupOne.arg.5", i16(0), "a group arg")
    ;
    
  std::cout << std::string("\nConfig::file_desc");
  std::cout << settings->file_desc;

  std::ifstream in("properties_parser_test.cfg");
  Config::Parser prs_file(false);
  prs_file.config.add(settings->file_desc);
  prs_file.config.add(settings->cmdline_desc);
  prs_file.parse_filedata(in);
  
  std::cout << std::string("\nConfig::file_desc");
  std::cout << settings->file_desc; // original configuration (left untouched)

  std::cout << prs_file.config;  // configurations options with values
  std::cout << prs_file; // Raw(std::strings) Parsed
  prs_file.print_options(std::cout);
  
  Properties props;
  props.set("dummy", boo(true));
  std::cout << props.to_string("dummy") << "\n";

  props.load_from(prs_file.get_options());

  props.print(std::cout, true);

  return 0;
}