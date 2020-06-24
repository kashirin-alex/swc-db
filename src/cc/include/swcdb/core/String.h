/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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


#ifndef swc_common_STRING_H
#define swc_common_STRING_H

#include <string>
#include <sstream>
#include <limits>

namespace SWC {

  /**
   * Returns a std::string using printf like format facilities
   * Vanilla snprintf is about 1.5x faster than this, which is about:
   *   10x faster than boost::format;
   *   1.5x faster than std::string append (operator+=);
   *   3.5x faster than std::string operator+;
   *
   * @param fmt A printf-like format string
   * @return A new std::string with the formatted text
   */
  std::string format(const char *fmt, ...) __attribute__((format (printf, 1, 2)));

  /**
   * Return a string presentation of a sequence. Is quiet slow but versatile,
   * as it uses ostringstream.
   *
   * @param seq A STL-compatible sequence with forward-directional iterators
   * @param sep A separator which is inserted after each list item
   * @return A new std::string with the formatted text
   */
  template <class SequenceT>
  std::string format_list(const SequenceT &seq, const char *sep = ", ") {
    typedef typename SequenceT::const_iterator Iterator;
    Iterator it = seq.begin(), end = seq.end();
    std::ostringstream out;
    out << '[';

    if (it != end) {
      out << *it;
      ++it;
    }
    for (; it != end; ++it)
      out << sep << *it;

    out << ']';
    return out.str();
  }



}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/String.cc"
#endif 

#endif 
