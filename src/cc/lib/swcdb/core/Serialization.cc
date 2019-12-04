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


#include "swcdb/core/Serialization.h"


namespace SWC { namespace Serialization {

void encode_i8(uint8_t** bufp, uint8_t val) {
  HT_ENCODE_I8(*bufp, val);
}

uint8_t decode_i8(const uint8_t** bufp, size_t* remainp) {
  HT_DECODE_NEED(*remainp, 1);
  return *(*bufp)++;
}

uint8_t decode_byte(const uint8_t** bufp, size_t* remainp) {
  return decode_i8(bufp, remainp);
}

void encode_bool(uint8_t** bufp, bool bval) {
  HT_ENCODE_BOOL(*bufp, bval);
}

bool decode_bool(const uint8_t** bufp, size_t* remainp) {
  return decode_i8(bufp, remainp) != 0;
}

void encode_i16(uint8_t** bufp , uint16_t val) {
  HT_ENCODE_I16(*bufp, val);
}

uint16_t decode_i16(const uint8_t** bufp, size_t* remainp) {
  uint16_t val;
  HT_DECODE_I16(*bufp, *remainp, val);
  return val;
}

void encode_i32(uint8_t** bufp, uint32_t val) {
  HT_ENCODE_I32(*bufp, val);
}

uint32_t decode_i32(const uint8_t** bufp, size_t* remainp) {
  uint32_t val;
  HT_DECODE_I32(*bufp, *remainp, val);
  return val;
}

void encode_i64(uint8_t** bufp, uint64_t val) {
  HT_ENCODE_I64(*bufp, val);
}

uint64_t decode_i64(const uint8_t** bufp, size_t* remainp) {
  uint64_t val;
  HT_DECODE_I64(*bufp, *remainp, val);
  return val;
}

int encoded_length_vi32(uint32_t val) {
  return HT_ENCODED_LEN_VI32(val);
}

int encoded_length_vi64(uint64_t val) {
  return HT_ENCODED_LEN_VI64(val);
}

void encode_vi32(uint8_t** bufp, uint32_t val) {
  HT_ENCODE_VI32(*bufp, val, return);
}

void encode_vi64(uint8_t** bufp, uint64_t val) {
  HT_ENCODE_VI64(*bufp, val, return);
}

uint32_t decode_vi32(const uint8_t** bufp, size_t* remainp) {
  uint32_t n;
  HT_DECODE_VI32(*bufp, *remainp, n, return n);
}

uint64_t decode_vi64(const uint8_t** bufp, size_t* remainp) {
  uint64_t n;
  HT_DECODE_VI64(*bufp, *remainp, n, return n);
}

uint32_t decode_vi32(const uint8_t** bufp) {
  size_t remain = 6;
  return decode_vi32(bufp, &remain);
}

uint64_t decode_vi64(const uint8_t** bufp) {
  size_t remain = 12;
  return decode_vi64(bufp, &remain);
}

size_t encoded_length_bytes32(int32_t len) {
  return len + 4;
}

void encode_bytes32(uint8_t** bufp, const void* data, int32_t len) {
  HT_ENCODE_BYTES32(*bufp, data, len);
}

uint8_t* decode_bytes32(const uint8_t** bufp, size_t* remainp, uint32_t* lenp) {
  uint8_t* out;
  HT_DECODE_BYTES32(*bufp, *remainp, out, *lenp);
  return out;
}

void encode_bytes(uint8_t** bufp, const void* data, int32_t len) {
  HT_ENCODE_BYTES(*bufp, data, len);
}

uint8_t* decode_bytes(const uint8_t** bufp, size_t* remainp, uint32_t len) {
  uint8_t* out;
  HT_DECODE_BYTES(*bufp, *remainp, out, len);
  return out;
}

size_t encoded_length_str16(const char* str) {
  return 2 + ((str == 0) ? 0 : strlen(str)) + 1;
}

size_t encoded_length_str16(const std::string& str) {
  return 2 + str.length() + 1;
}

void encode_str16(uint8_t** bufp, const void* str, uint16_t len) {
  HT_ENCODE_STR16(*bufp, str, len);
}

void encode_str16(uint8_t** bufp, const char* str) {
  uint16_t len = (str == 0) ? 0 : strlen(str);
  encode_str16(bufp, str, len);
}

const char* decode_str16(const uint8_t** bufp, size_t* remainp) {
  const char* str;
  uint16_t len;
  HT_DECODE_STR16(*bufp, *remainp, str, len);
  return str;
}

char* decode_str16(const uint8_t** bufp, size_t* remainp, uint16_t *lenp) {
  char* str;
  HT_DECODE_STR16(*bufp, *remainp, str, *lenp);
  return str;
}

size_t encoded_length_vstr(size_t len) {
  return encoded_length_vi64(len) + len + 1;
}

size_t encoded_length_vstr(const char* s) {
  return encoded_length_vstr(s ? strlen(s) : 0);
}

size_t encoded_length_vstr(const std::string& s) {
  return encoded_length_vstr(s.length());
}

void encode_vstr(uint8_t** bufp, const void* buf, size_t len) {
  HT_ENCODE_VSTR(*bufp, buf, len);
}

void encode_vstr(uint8_t** bufp, const char* s) {
  encode_vstr(bufp, s, s ? strlen(s) : 0);
}

char* decode_vstr(const uint8_t** bufp, size_t* remainp) {
  char* buf;
  size_t len;
  HT_DECODE_VSTR(*bufp, *remainp, buf, len);
  (void)len; // avoid warnings because len is assigned but never used
  return buf;
}

char* decode_vstr(const uint8_t** bufp, size_t* remainp, uint32_t* lenp) {
  char* buf;
  HT_DECODE_VSTR(*bufp, *remainp, buf, *lenp);
  return buf;
}

void encode_double(uint8_t** bufp, double val) {
  int64_t lod = (int64_t)val;
  int64_t rod = (int64_t)((val - (double)lod) * (double)1000000000000000000.00);
  HT_ENCODE_I64(*bufp, lod);
  HT_ENCODE_I64(*bufp, rod);
}

double decode_double(const uint8_t** bufp, size_t* remainp) {
  int64_t lod, rod;
  HT_DECODE_I64(*bufp, *remainp, lod);
  HT_DECODE_I64(*bufp, *remainp, rod);
  return (double)lod + ((double)rod / (double)1000000000000000000.00);
}

int encoded_length_double() {
  return 16;
}

bool equal(double a, double b) {
  int64_t lod_a = (int64_t)a;
  int64_t rod_a = (int64_t)((a - (double)lod_a) * (double)1000000000000000000.00);
  double aprime = (double)lod_a + ((double)rod_a / (double)1000000000000000000.00);
  int64_t lod_b = (int64_t)b;
  int64_t rod_b = (int64_t)((b - (double)lod_b) * (double)1000000000000000000.00);
  double bprime = (double)lod_b + ((double)rod_b / (double)1000000000000000000.00);
  return aprime == bprime;
}



}}
