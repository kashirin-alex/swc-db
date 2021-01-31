/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Time_h
#define swcdb_core_Time_h

#include "swcdb/core/Compat.h"

#include <iostream>
#include <chrono>


namespace SWC {


/**
 * @brief The SWC-DB Date and Time C++ namespace 'SWC::Time'
 *
 * \ingroup Core
 */
namespace Time {


void checkings();

SWC_CAN_INLINE
extern
int64_t now_ms() noexcept {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

SWC_CAN_INLINE
extern
int64_t now_ns() noexcept {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t parse_ns(int& err, const std::string& buf);

std::string fmt_ns(int64_t ns);

std::ostream &hires_now_ns(std::ostream &out);


}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Time.cc"
#endif

#endif // swcdb_core_Time_h
