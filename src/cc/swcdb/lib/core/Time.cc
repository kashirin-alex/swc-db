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
/// Time related definitions


#include "Time.h"

#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <ratio>

namespace SWC { namespace Time {

int64_t now_ns() {
  assert((std::ratio_less_equal<std::chrono::system_clock::duration::period, std::chrono::nanoseconds::period>::value));
  return (int64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

std::ostream &hires_now_ns(std::ostream &out) {
  auto now = std::chrono::system_clock::now();
  return out << std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() 
             <<'.'<< std::setw(9) << std::setfill('0') 
             << (std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count() % 1000000000LL);
}

#if defined(__sun__)
time_t timegm(struct tm *t) {
  time_t tl, tb;
  struct tm *tg;

  tl = mktime (t);
  if (tl == -1)
    {
      t->tm_hour--;
      tl = mktime (t);
      if (tl == -1)
        return -1; /* can't deal with output from strptime */
      tl += 3600;
    }
  tg = gmtime (&tl);
  tg->tm_isdst = 0;
  tb = mktime (tg);
  if (tb == -1)
    {
      tg->tm_hour--;
      tb = mktime (tg);
      if (tb == -1)
        return -1; /* can't deal with output from gmtime */
      tb += 3600;
    }
  return (tl - (tb - tl));
}
#endif

}} 
