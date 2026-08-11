/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Error.h"

#include <sstream>
#include <string>


namespace {


void test_get_text() {
  SWC_ASSERT(std::string(SWC::Error::get_text(SWC::Error::OK)).size() > 0);
  SWC_ASSERT(
    std::string(SWC::Error::get_text(SWC::Error::SERIALIZATION_INPUT_OVERRUN))
      .size() > 0);
  SWC_ASSERT(
    std::string(SWC::Error::get_text(SWC::Error::ENCODER_ENCODE)).size() > 0);
  SWC_ASSERT(
    std::string(SWC::Error::get_text(SWC::Error::COMM_BAD_HEADER)).size() > 0);

  std::ostringstream out;
  SWC::Error::print(out, SWC::Error::OK);
  SWC_ASSERT(!out.str().empty());
}


void test_throw_and_catch() {
  bool caught = false;
  try {
    SWC_THROW(SWC::Error::INVALID_ARGUMENT, "unit-test-message");
  } catch(const SWC::Error::Exception& e) {
    caught = true;
    SWC_ASSERT(e.code() == SWC::Error::INVALID_ARGUMENT);
    SWC_ASSERT(std::string(e.what()) == "unit-test-message");
    SWC_ASSERT(e.line() > 0);
    SWC_ASSERT(e.func() != nullptr);
    SWC_ASSERT(e.file() != nullptr);
    SWC_ASSERT(e.message().find("unit-test-message") != std::string::npos);
  }
  SWC_ASSERT(caught);
}


void test_throwf() {
  bool caught = false;
  try {
    SWC_THROWF(SWC::Error::BAD_FORMAT, "bad value=%d", 7);
  } catch(const SWC::Error::Exception& e) {
    caught = true;
    SWC_ASSERT(e.code() == SWC::Error::BAD_FORMAT);
    SWC_ASSERT(std::string(e.what()).find("7") != std::string::npos);
  }
  SWC_ASSERT(caught);
}


void test_nested_current_exception() {
  bool caught = false;
  try {
    try {
      SWC_THROW(SWC::Error::IO_ERROR, "inner");
    } catch(...) {
      throw SWC_CURRENT_EXCEPTION("outer");
    }
  } catch(const SWC::Error::Exception& e) {
    caught = true;
    SWC_ASSERT(e.code() == SWC::Error::IO_ERROR);
    SWC_ASSERT(std::string(e.what()).find("outer") != std::string::npos ||
               std::string(e.what()).find("inner") != std::string::npos);
  }
  SWC_ASSERT(caught);
}


} // namespace


int main() {
  test_get_text();
  test_throw_and_catch();
  test_throwf();
  test_nested_current_exception();

  std::cout << "error_exception OK\n";
  return 0;
}
