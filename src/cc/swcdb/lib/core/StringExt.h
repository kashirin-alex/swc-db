/* -*- c++ -*-
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

/** @file
 * String extensions and helpers: sets, maps, append operators etc.
 */

#ifndef swc_core_StringExt_h
#define swc_core_StringExt_h

#include "String.h"

#include <cstdio>
#include <set>
#include <map>

/** @addtogroup Common
 *  @{
 */

using namespace SWC;

/** STL Set managing Strings */
typedef std::set<String> StringSet;

/** STL Strict Weak Ordering for comparing c-style strings */
struct LtCstr {
  bool operator()(const char *s1, const char *s2) const {
    return strcmp(s1, s2) < 0;
  }
};

/** STL Set managing c-style strings */
typedef std::set<const char *, LtCstr> CstrSet;

/** STL map from c-style string to int32_t */
typedef std::map<const char *, int32_t, LtCstr> CstrToInt32Map;

/** STL map from c-style string to int64_t */
typedef std::map<const char *, int64_t, LtCstr> CstrToInt64MapT;

/** Append operator for shorts */ 
inline String operator+(const String &s1, short sval) {
  return s1 + Int16Formatter(sval).c_str();
}

/** Append operator for ushorts */
inline String operator+(const String &s1, uint16_t sval) {
  return s1 + UInt16Formatter(sval).c_str();
}

/** Append operator for integers */ 
inline String operator+(const String &s1, int ival) {
  return s1 + Int32Formatter(ival).c_str();
}

/** Append operator for unsigned integers */ 
inline String operator+(const String &s1, uint32_t ival) {
  return s1 + UInt32Formatter(ival).c_str();
}

/** Append operator for 64bit integers */ 
inline String operator+(const String &s1, int64_t llval) {
  return s1 + Int64Formatter(llval).c_str();
}

/** Append operator for 64bit unsigned integers */ 
inline String operator+(const String &s1, uint64_t llval) {
  return s1 + UInt64Formatter(llval).c_str();
}

/** @} */

#endif // swc_core_StringExt_h
