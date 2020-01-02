/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/// @file
/// Time related declarations.

#include "swcdb/core/Time.h"

#include <cassert>
#include <ratio>

#include <iomanip>
#include <ctime>

extern "C" {
#include <sys/time.h>
}


namespace std { namespace chrono {
fast_clock::time_point fast_clock::now() noexcept {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return fast_clock::time_point(
    microseconds((tv.tv_sec * 1000000LL) + tv.tv_usec));
  }

time_t fast_clock::to_time_t(const time_point& __t) noexcept {
  return (time_t)(__t.time_since_epoch().count() / 1000000LL);
}

fast_clock::time_point fast_clock::from_time_t(time_t __t) noexcept {
  return fast_clock::time_point(microseconds(__t * 1000000LL));
}
}} // namespace std::chrono



namespace SWC { namespace Time {

const int64_t now_ms() {
  return (int64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}


const int64_t now_ns() {
  assert((
    std::ratio_less_equal<
      std::chrono::system_clock::duration::period, 
      std::chrono::nanoseconds::period
    >::value
  ));
  return (int64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

const int64_t parse_ns(int& err, const std::string& buf) {
  const char* ptr = buf.c_str();
  if(buf.find("/") == std::string::npos) {  
    while(*ptr != 0 && *ptr == '0') ptr++;
    char *last;
    errno = 0;
    int64_t ns = strtoll(ptr, &last, 0);
    if(errno || (*ptr != 0 && ptr == last))
      err = errno ? errno : EINVAL;
    return ns;
  }

  int64_t ns = 0;

  struct tm info;
  if(strptime(ptr, "%Y/%m/%d %H:%M:%S", &info) == NULL) {
    err = EINVAL;
    return ns;
  } 
  ns += mktime(&info) * 1000000000;

  while(*ptr != 0 && *ptr++ != '.');

  if(*ptr != 0) {
    char *last;
    uint32_t res = strtoul(ptr, &last, 0);
    if(ptr == last || res > 999999999) {
      err = EINVAL;
      return ns;
    }
    ns += res;
  }
  return ns;
}

const std::string fmt_ns(int64_t ns) {
  int64_t secs = ns/1000000000;
  time_t t_secs = (time_t)secs;
  char res[20];
  std::strftime(res, 20, "%Y/%m/%d %H:%M:%S", std::gmtime(&t_secs));
  std::string nanos = std::to_string(ns-secs*1000000000);
  if(nanos.size() < 9)
    nanos.insert(nanos.begin(), 9-nanos.size(), '0');
  return std::string(res) + "." + nanos;
}

std::ostream &hires_now_ns(std::ostream &out) {
  auto now = std::chrono::system_clock::now();
  return out << std::chrono::duration_cast<std::chrono::seconds>(
                  now.time_since_epoch()).count() 
             <<'.'<< std::setw(9) << std::setfill('0') 
             << (std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch()).count() % 1000000000LL);
}

}}

