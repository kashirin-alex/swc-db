/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/config/Properties.h"
#include "swcdb/core/config/PropertiesParser.h"

#include <string>


namespace {


void set_owned(SWC::Config::Properties& props,
               const char* name,
               SWC::Config::Property::Value::Ptr value) {
  props.set(name, value);
  delete value;
}


void test_typed_get_set() {
  SWC::Config::Properties props;
  SWC_ASSERT(!props.has("flag"));

  set_owned(props, "flag", SWC::Config::boo(true));
  set_owned(props, "count", SWC::Config::i32(42));
  set_owned(props, "name", SWC::Config::str(std::string("swcdb")));

  SWC_ASSERT(props.has("flag"));
  SWC_ASSERT(props.has("count"));
  SWC_ASSERT(props.has("name"));

  SWC_ASSERT(props.get_bool("flag"));
  SWC_ASSERT(props.get_i32("count") == 42);
  SWC_ASSERT(props.get_str("name") == "swcdb");

  SWC_ASSERT(props.get_bool("missing", false) == false);
  SWC_ASSERT(props.get_i32("missing", 7) == 7);
  SWC_ASSERT(props.get_str("missing", "def") == "def");

  SWC_ASSERT(
    props.get_ptr("flag")->type() ==
    SWC::Config::Property::Value::TYPE_BOOL);
  SWC_ASSERT(
    props.get_ptr("count")->type() ==
    SWC::Config::Property::Value::TYPE_INT32);
  SWC_ASSERT(
    props.get_ptr("name")->type() ==
    SWC::Config::Property::Value::TYPE_STRING);
}


void test_alias_remove_reset() {
  SWC::Config::Properties props;
  set_owned(props, "primary", SWC::Config::i32(11));
  props.alias("primary", "alias");

  SWC_ASSERT(props.has("alias"));
  SWC_ASSERT(props.get_i32("alias") == 11);

  props.remove("primary");
  SWC_ASSERT(!props.has("primary"));
  SWC_ASSERT(!props.has("alias"));

  set_owned(props, "tmp", SWC::Config::boo(false));
  SWC_ASSERT(props.has("tmp"));
  props.reset();
  SWC_ASSERT(!props.has("tmp"));
}


void test_missing_throws() {
  SWC::Config::Properties props;
  bool threw = false;
  try {
    (void)props.get_i32("nope");
  } catch(const SWC::Error::Exception& e) {
    threw = true;
    SWC_ASSERT(e.code() == SWC::Error::CONFIG_GET_ERROR);
  }
  SWC_ASSERT(threw);
}


void test_update_value() {
  SWC::Config::Properties props;
  set_owned(props, "n", SWC::Config::i32(1));
  SWC_ASSERT(props.get_i32("n") == 1);

  set_owned(props, "n", SWC::Config::i32(99));
  SWC_ASSERT(props.get_i32("n") == 99);
}


} // namespace


int main() {
  test_typed_get_set();
  test_alias_remove_reset();
  test_missing_throws();
  test_update_value();

  std::cout << "properties_api OK\n";
  return 0;
}
