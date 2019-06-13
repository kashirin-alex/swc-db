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
 * Fixed size string buffers.
 * A stringstream, istringstream, ostringstream backed by a fixed size
 * string buffer.
 */

#ifndef swc_common_FIXED_STREAM_H
#define swc_common_FIXED_STREAM_H

#include "String.h"
#include <streambuf>
#include <istream>
#include <ostream>

namespace SWC {

/** @addtogroup Common
 *  @{
 */

/**
 * A simple streambuf with fixed size buffer.
 * Convenient for limitting size of output; faster than ostringstream
 * and friends, which require heap allocations
 */
class FixedStreamBuf : public std::streambuf {
public:
  /** Constructor
   *
   * Constructs a new FixedStreamBuf object backed by a memory buffer provided
   * by the caller. All objects "streamed" into this class will be stored
   * in this memory buffer.
   *
   * @param buf Pointer to the memory buffer
   * @param len Size of that memory buffer
   */
  FixedStreamBuf(char *buf, size_t len) { setp(buf, buf + len); }

  /** Constructor
   *
   * Constructs a new FixedStreamBuf object backed by a memory buffer provided
   * by the caller. All objects are "streamed" from this memory buffer.
   */
  FixedStreamBuf(const char *buf, const char *next, const char *end) {
    setg((char *)buf, (char *)next, (char *)end);
  }

  /** Returns a String object from the output buffer */
  String str() { return String(pbase(), pptr()); }
};

/**
 * Output stream using a fixed buffer
 */
class FixedOstream : private virtual FixedStreamBuf, public std::ostream {
public:
  typedef FixedStreamBuf StreamBuf;

  /** Constructs the stream from an existing buffer; all streamed objects
   * are stored in that buffer
   *
   * @param buf Pointer to the memory buffer
   * @param len Size of that buffer, in bytes
   */
  FixedOstream(char *buf, size_t len)
    : FixedStreamBuf(buf, len), std::ostream(static_cast<StreamBuf *>(this)) {
  }

  /** Returns a pointer to the output buffer */
  char *output() { return StreamBuf::pbase(); }

  /** Returns a pointer to the current position in the output buffer */
  char *output_ptr() { return StreamBuf::pptr(); }

  /** Returns a pointer to the end of the output buffer */
  char *output_end() { return StreamBuf::epptr(); }

  /** Returns a String object from the output buffer */
  String str() { return StreamBuf::str(); }
};

/**
 * Input stream using a fixed buffer
 */
class FixedIstream : private virtual FixedStreamBuf, public std::istream {
public:
  typedef FixedStreamBuf StreamBuf;

  /** Constructs the input stream from an existing buffer; all streamed objects
   * are read from that buffer
   *
   * @param buf Pointer to the beginning of the memory buffer
   * @param end Pointer to the end of the memory buffer
   */
  FixedIstream(const char *buf, const char *end)
    : FixedStreamBuf(buf, buf, end),
    std::istream(static_cast<StreamBuf *>(this)) {
  }

  /** Returns a pointer to the beginning of the input buffer */
  char *input() { return StreamBuf::eback(); }

  /** Returns a pointer to the current position in the input buffer */
  char *input_ptr() { return StreamBuf::gptr(); }

  /** Returns a pointer to the end of the input buffer */
  char *input_end() { return StreamBuf::egptr(); }
};

/** @} */

}

#endif
