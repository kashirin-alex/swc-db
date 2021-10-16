/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/config/Settings.h"
#include <fstream>


namespace SWC{ namespace Config {

void init_app_options(Settings* settings) {
  settings->cmdline_desc
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

} }

using namespace SWC;

void run() {
  auto settings = Env::Config::settings();

  // remove env-dynamic configs
  settings->cmdline_desc.remove("swc.logging.path");
  settings->cmdline_desc.remove("swc.cfg.path");


  std::cout << std::string("\nConfig::cmdline_desc");
  std::cout << settings->cmdline_desc;
  settings->print(std::cout);


  // cfg file parse test definitions
  settings->file_desc.add_options()
   ("is_true_bool", Config::boo(false), "a boolean arg")
   ("is_yes_bool", Config::boo(false), "a boolean arg")
   ("is_one_bool", Config::boo(false), "a boolean arg")
   ("is_no_bool", Config::boo(true), "a boolean arg")
   ("is_rest_bool", Config::boo(true), "a boolean arg")

   ("boo", Config::boo(false), "a boolean arg")
   ("i16", Config::i16(1), "16-bit integer")
   ("i32", Config::i32(), "32-bit integer")
   ("i64", Config::i64(1), "64-bit integer")
   ("int64s", Config::i64s(), "a list of 64-bit integers")
   ("string", Config::str("A Default std::string Value"), "a string arg")
   ("string.qouted", Config::str("A Default std::string Value"), "a string arg")
   ("strs", Config::strs(), "a list of strings")
   ("float64", Config::f64(3.12), "a double arg")
   ("f64s", Config::f64s(), "a list of doubles")
   ("missing", Config::str("a missing string in cfg file, default applies"), "a string arg")
   ("aGroupOne.arg.1", Config::i16(0), "a group arg")
   ("aGroupOne.arg.2", Config::i16(0), "a group arg")
   ("aGroupOne.arg.3", Config::i16(0), "a group arg")
   ("aGroupOne.arg.4", Config::i16(0), "a group arg")
   ("aGroupOne.arg.5", Config::i16(0), "a group arg")
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

  Config::Properties props;
  Config::Property::Value_bool dummy(true);
  props.set("dummy", &dummy);
  std::cout << props.to_string("dummy") << "\n";

  props.load_from(prs_file.get_options());

  props.print(std::cout, true);
}


int main(int argc, char *argv[]) {
  SWC::Env::Config::init(argc, argv, &SWC::Config::init_app_options, nullptr);
  run();
  Env::Config::reset();
  return 0;
}