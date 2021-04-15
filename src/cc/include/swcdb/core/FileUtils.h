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

  /** Reads positional data from a file descriptor into a buffer
   *
   * @param fd The open file descriptor
   * @param offset The start offset in the file
   * @param vptr Pointer to the memory buffer
   * @param n Maximum size to read, in bytes
   * @return The number of bytes read, or -1 on error
   */
  ssize_t pread(int fd, off_t offset, void *vptr, size_t n) noexcept;

  /** Writes a std::string buffer to a file; the file is overwritten if it
   * already exists
   *
   * @param fname Path of the file that is (over)written
   * @param contents The string contents that are written to the file
   * @return Number of bytes written, or -1 on error
   */
  ssize_t write(const std::string& fname, const std::string& contents);

  /** Writes a memory buffer to a file descriptor
   *
   * @param fd Open file handle
   * @param vptr Pointer to the memory buffer
   * @param n Size of the memory buffer, in bytes
   * @return Number of bytes written, or -1 on error
   */
  ssize_t write(int fd, const void *vptr, size_t n) noexcept;

  /** Writes a string to a file descriptor
   *
   * @param fd Open file handle
   * @param str std::string to write to file
   * @return Number of bytes written, or -1 on error
   */
  inline ssize_t write(int fd, const std::string& str) {
    return write(fd, str.c_str(), str.length());
  }

  /** Reads a full file into a new buffer; the buffer is allocated with
   * operator new[], and the caller has to delete[] it.
   *
   * @param fname The file name
   * @param lenp Receives the length of the buffer, in bytes
   * @return A pointer allocated with new[]; needs to be delete[]d by
   *          the caller. Returns 0 on error (sets errno)
   */
  char *file_to_buffer(const std::string& fname, off_t *lenp);

  /** Reads a full file into a std::string
   *
   * @param fname The file name
   * @return A string with the data, or an empty string on error (sets errno)
   */
  std::string file_to_string(const std::string& fname);

  /** Creates a directory (with all parent directories, if required)
   *
   * @param dirname The directory name to create
   * @return true on success, otherwise falls (sets errno)
   */
  bool mkdirs(const std::string& dirname);

  /** Checks if a file or directory exists
   *
   * @return true if the file or directory exists, otherwise false
   */
  bool exists(const std::string& fname) noexcept;

  /** Unlinks (deletes) a file or directory
   *
   * @return true on success, otherwise false (sets errno)
   */
  bool unlink(const std::string& fname);

  /** Renames a file or directory
   *
   * @param oldpath The path of the file (or directory) to rename
   * @param newpath The new filename
   * @return true on success, otherwise false (sets errno)
   */
  bool rename(const std::string& oldpath, const std::string& newpath);

  /** Returns the size of a file (0 on error)
   *
   * @param fname The path of the file
   * @return The file size (in bytes) or 0 on error (sets errno)
   */
  uint64_t size(const std::string& fname) noexcept;

  /** Returns the size of a file (-1 on error)
   *
   * @param fname The path of the file
   * @return The file size (in bytes) or -1 on error (sets errno)
   */
  off_t length(const std::string& fname) noexcept;

  /** Returns the last modification time
   *
   * @param fname The path of the file
   * @return The file modification time_t or 0 on error (sets errno)
   */
  time_t modification(const std::string& fname) noexcept;

}

}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/FileUtils.cc"
#endif

#endif // swcdb_core_FileUtils_h

