/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_Time_h
#define swc_core_Time_h

#include "swcdb/core/Compat.h"

#include <iostream>


#include <chrono>


namespace std { namespace chrono {

class fast_clock {
  public:
  typedef microseconds      duration;
  typedef duration::rep     rep;
  typedef duration::period  period;
  typedef chrono::time_point<fast_clock> time_point;
  static constexpr bool is_steady = false;

  static time_point now() noexcept;

  static time_t to_time_t (const time_point& __t) noexcept;

  static time_point from_time_t(time_t __t) noexcept;
};

}} // namespace std::chrono



namespace SWC { 
  
typedef std::chrono::fast_clock ClockT;

namespace Time {

const int64_t now_ns();

std::ostream &hires_now_ns(std::ostream &out);

}}


#ifdef SWC_IMPL_SOURCE
#include "../../../lib/swcdb/core/Time.cc"
#endif 

#endif // swc_core_Time_h
