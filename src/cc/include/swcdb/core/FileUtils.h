/* -*- c++ -*-
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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

#ifndef swcdb_core_FileUtils_h
#define swcdb_core_FileUtils_h

#include "swcdb/core/Exception.h"


extern "C" {
//#include <sys/socket.h>
//#include <sys/types.h>
#include <sys/stat.h>
}

namespace SWC { namespace FileUtils {

  /** Reads a whole file into a std::string
   *
   * @param fname The file name
   * @param contents A reference to a std::string which will receive the data
   * @return <i>true</i> on success, <i>false</i> on error
   */
  bool read(const std::string& fname, std::string& contents);

  /** Reads data from a file descriptor into a buffer
   *
   * @param fd The open file descriptor
   * @param vptr Pointer to the memory buffer
   * @param n Maximum size to read, in bytes
   * @return The number of bytes read, or -1 on error
   */
  ssize_t read(int fd, void *vptr, size_t n) noexcept;

  /** Reads a full file into a new buffer; the buffer is allocated with
   * operator new[], and the caller has to delete[] it.
   *
   * @param fname The file name
   * @param lenp Receives the length of the buffer, in bytes
   * @return A pointer allocated with new[]; needs to be delete[]d by
   *          the caller. Returns 0 on error (sets errno)
   */
  char *file_to_buffer(const std::string& fname, off_t *lenp);

  /** Checks if a file or directory exists
   *
   * @return true if the file or directory exists, otherwise false
   */
  bool exists(const std::string& fname) noexcept;

}

}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/FileUtils.cc"
#endif

#endif // swcdb_core_FileUtils_h

