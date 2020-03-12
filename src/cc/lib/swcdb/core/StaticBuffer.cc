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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#include "swcdb/core/StaticBuffer.h"

#include <cstring>
#include <cstdlib>

namespace SWC {

StaticBuffer::StaticBuffer() : base(0), size(0), own(true) { }

StaticBuffer::StaticBuffer(size_t len) : size(len), own(true) {
  allocate();
}

void StaticBuffer::allocate() {
  base = new uint8_t[size];
}

void StaticBuffer::reallocate(uint32_t len) {
  free();
  own = true;
  size = len;
  allocate();
}

StaticBuffer::StaticBuffer(void *data, uint32_t len, bool take_ownership)
                          : base((uint8_t *)data), size(len), 
                            own(take_ownership) {
}

StaticBuffer::StaticBuffer(DynamicBuffer &dbuf) {
  base = dbuf.base;
  size = dbuf.fill();
  own = dbuf.own;
  if (own) {
    dbuf.base = dbuf.ptr = 0;
    dbuf.size = 0;
  }
}

StaticBuffer::StaticBuffer(StaticBuffer& other) {
  base = other.base;
  size = other.size;
  own = other.own;
  if (own) {
    other.own = false;
    other.base = 0;
  }
}

void StaticBuffer::set(StaticBuffer& other) {
  free();
  base = other.base;
  size = other.size;
  own = other.own;
  if (own) {
    other.own = false;
    other.base = 0;
    other.size = 0;
  }
}

void StaticBuffer::set(DynamicBuffer &dbuf) {
  free();
  base = dbuf.base;
  size = dbuf.fill();
  own = dbuf.own;
  if (own) {
    dbuf.base = dbuf.ptr = 0;
    dbuf.size = 0;
  }
}

void StaticBuffer::set(uint8_t *data, uint32_t len, bool take_ownership) {
  free();
  base = data;
  size = len;
  own = take_ownership;
}

void StaticBuffer::free() {
  if (own && base)
    delete [] base;
  base = 0;
  size = 0;
}

StaticBuffer::~StaticBuffer() {
  free();
}

bool operator<(const StaticBuffer& sb1, const StaticBuffer& sb2) {
  size_t len = (sb1.size < sb2.size) ? sb1.size : sb2.size;
  int cmp = memcmp(sb1.base, sb2.base, len);
  return (cmp==0) ? sb1.size < sb2.size : cmp < 0;
}

bool operator==(const StaticBuffer& sb1, const StaticBuffer& sb2) {
  if (sb1.size != sb2.size)
    return false;
  return memcmp(
    sb1.base, sb2.base, (sb1.size < sb2.size) ? sb1.size : sb2.size) == 0;
}

bool operator!=(const StaticBuffer& sb1, const StaticBuffer& sb2) {
  return !(sb1 == sb2);
}


}
