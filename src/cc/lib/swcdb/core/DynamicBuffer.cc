/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/core/DynamicBuffer.h"
#include <cstdint>
#include <cstring>

namespace SWC {

DynamicBuffer::DynamicBuffer(size_t initial_size, bool own_buffer)
                            : size(initial_size), own(own_buffer),
                              base(size ? new uint8_t[size] : 0), 
                              ptr(base), mark(base) {
}

DynamicBuffer::~DynamicBuffer() {
  if (own && base)
    delete [] base;
}

size_t DynamicBuffer::remaining() const { 
  return size - (ptr - base); 
}

size_t DynamicBuffer::fill() const { 
  return ptr - base; 
}

bool DynamicBuffer::empty() const { 
  return ptr == base; 
}

void DynamicBuffer::ensure(size_t len) {
  if(len > remaining())
    grow((fill() + len) * 3 / 2);
}

void DynamicBuffer::reserve(size_t len, bool nocopy) {
  if (len > remaining())
    grow(fill() + len, nocopy);
}

uint8_t* DynamicBuffer::add_unchecked(const void *data, size_t len) {
  if (!data)
    return ptr;
  uint8_t *rptr = ptr;
  memcpy(ptr, data, len);
  ptr += len;
  return rptr;
}

uint8_t* DynamicBuffer::add(const void *data, size_t len) {
  ensure(len);
  return add_unchecked(data, len);
}

void DynamicBuffer::set(const void *data, size_t len) {
  clear();
  reserve(len);
  add_unchecked(data, len);
}

void DynamicBuffer::clear() {
  ptr = base;
}

void DynamicBuffer::set_mark() {
  mark = ptr;
}

void DynamicBuffer::free() {
  if (own)
    delete [] base;
  base = ptr = mark = 0;
  size = 0;
}

uint8_t* DynamicBuffer::release(size_t *lenp) {
  uint8_t *rbuf = base;
  if (lenp)
    *lenp = fill();
  ptr = base = mark = 0;
  size = 0;
  return rbuf;
}

void DynamicBuffer::grow(size_t new_size, bool nocopy) {
  uint8_t *new_buf = new uint8_t[new_size];

  if(!nocopy && base)
    memcpy(new_buf, base, ptr-base);

  ptr = new_buf + (ptr-base);
  mark = new_buf + (mark-base);
  if(own)
    delete [] base;
  base = new_buf;
  size = new_size;
}

}