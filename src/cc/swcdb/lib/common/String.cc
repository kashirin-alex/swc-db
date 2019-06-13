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
 * along with Hypertable. If not, see <http://www.gnu.org/licenses/>
 */

/** @file
 * A String class based on std::string.
 */

#include "String.h"
#include "Compat.h"

#include <cstdarg>
#include <cstdio>
#include <algorithm>


using namespace std;

namespace SWC {

String format(const char *fmt, ...) {
  char buf[1024];       // should be enough for most cases
  int n, size = sizeof(buf);
  char *p = buf;
  va_list ap;

  do {
    va_start(ap, fmt);
    n = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (n > -1 && n < size)
      break;    // worked!

    if (n > -1)         // glibc 2.1+/iso c99
      size = n + 1;     //   exactly what's needed
    else                // glibc 2.0
      size *= 2;        //   double the size and try again

    p = (char *)(p == buf ? malloc(size) : realloc(p, size));

    if (!p)
      throw bad_alloc();
  } while (true);

  if (buf == p)
    return string(p, n);

  string ret(p, n);
  free(p);

  return ret;
}

char const *const digits = "0123456789";

String format_number(int64_t n, int sep) {
  char buf[30], *p = buf, *p0 = buf;
  int ndigits = 0;
  uint64_t num; // for edge cases when -n is still negative when n < 0

  if (n < 0) {
    *p++ = '-';
    p0 = p;
    num = -n;
  }
  else
    num = n;

  if (num == 0)
    *p++ = '0';
  else for (; num != 0; num /= 10) {
    *p++ = digits[num % 10];
    ++ndigits;

    if (num >= 10 && ndigits % 3 == 0)
      *p++ = sep;
  }

  int len = ndigits + (ndigits - 1) / 3;
  std::reverse(p0, p0 + len);

  return String(buf, len + p0 - buf);
}

String
format_bytes(size_t n, const void *buf, size_t len, const char *trailer) {
  if (buf) {
    if (len <= n)
      return String((char *)buf, len);

    String out((char *)buf, n);
    out += trailer;
    return out;
  }
  return "<null>";
}

const char NumericFormatterDigits::DIGITS[] =
  "0001020304050607080910111213141516171819"
  "2021222324252627282930313233343536373839"
  "4041424344454647484950515253545556575859"
  "6061626364656667686970717273747576777879"
  "8081828384858687888990919293949596979899";

}
