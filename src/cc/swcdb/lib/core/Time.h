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

#ifndef swc_core_Time_h
#define swc_core_Time_h

#include "Compat.h"

#include <iosfwd>

namespace SWC {namespace Time {

  /// Returns the current time in nanoseconds as a 64bit number
  int64_t now_ns();

  /// Prints the current time as seconds and nanoseconds, delimited by '.'
  std::ostream &hires_now_ns(std::ostream &);

#if defined(__sun__)
  time_t timegm(struct tm *t);
#endif

}}

#endif // swc_core_Time_h
