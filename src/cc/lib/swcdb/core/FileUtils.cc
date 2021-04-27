/*
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


#include "swcdb/core/FileUtils.h"

extern "C" {
#include <fcntl.h>
#include <errno.h>
}


namespace SWC { namespace FileUtils {



bool read(const std::string& fname, std::string& contents) {
  off_t len {};
  char *buf = file_to_buffer(fname, &len);
  if(buf) {
    contents.append(buf, len);
    delete [] buf;
    return true;
  }
  return false;
}


ssize_t read(int fd, void *vptr, size_t n) noexcept {
  char* ptr = static_cast<char*>(vptr);
  size_t nleft = n;
  ssize_t nread;
  while (nleft) {
    if ((nread = ::read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR)
        nread = 0;/* and call read() again */
      else if (errno == EAGAIN)
        break;
      else
        return -1;
    }
    else if (!nread)
      break; /* EOF */

    nleft -= nread;
    ptr   += nread;
  }
  return n - nleft;
}

char* file_to_buffer(const std::string& fname, off_t *lenp) {
  struct stat statbuf;
  int fd;

  *lenp = 0;
  errno = 0;

  if((fd = open(fname.c_str(), O_RDONLY)) < 0) {
    int saved_errno = errno;
    SWC_LOGF(LOG_ERROR, "open(\"%s\") failure - %s", fname.c_str(),
            Error::get_text(saved_errno));
    errno = saved_errno;
    return nullptr;
  }

  if(fstat(fd, &statbuf) < 0) {
    int saved_errno = errno;
    SWC_LOGF(LOG_ERROR, "fstat(\"%s\") failure - %s", fname.c_str(),
           Error::get_text(saved_errno));
    errno = saved_errno;
    return nullptr;
  }

  *lenp = statbuf.st_size;

  char *rbuf = new char [*lenp + 1];

  ssize_t nread = read(fd, rbuf, *lenp);

  ::close(fd);

  if(nread == -1) {
    int saved_errno = errno;
    SWC_LOGF(LOG_ERROR, "read(\"%s\") failure - %s", fname.c_str(),
            Error::get_text(saved_errno));
    errno = saved_errno;
    delete [] rbuf;
    *lenp = 0;
    return nullptr;
  }

  if(nread < *lenp) {
    SWC_LOGF(LOG_WARN, "short read (%ld of %ld bytes)", nread, *lenp);
    *lenp = nread;
  }

  rbuf[nread] = 0;
  return rbuf;
}

bool exists(const std::string& fname) noexcept {
  struct stat statbuf;
  return !stat(fname.c_str(), &statbuf);
}

}}
