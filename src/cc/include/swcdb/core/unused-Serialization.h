/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef swc_core_Serialization_h
#define swc_core_Serialization_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Error.h"


namespace SWC { namespace Serialization {

/**
 * Encodes a byte into the given buffer. Assumes there is
 * enough space available. Increments buffer pointer.
 *
 * @param bufp Address of the destination buffer
 * @param val The byte
 */
void encode_i8(uint8_t** bufp, uint8_t val);

/**
 * Decode a 8-bit integer (a byte/character)
 *
 * @param bufp The pointer to the source buffer
 * @param remainp The pointer to the remaining size variable
 * @return The decoded value
 */
uint8_t decode_i8(const uint8_t** bufp, size_t* remainp);

/**
 * Decodes a single byte from the given buffer. Increments buffer pointer
 * and decrements remainp on success.
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to the number of bytes remaining in buffer
 * @return The byte value
 */
uint8_t decode_byte(const uint8_t** bufp, size_t* remainp);

/**
 * Encodes a boolean into the given buffer. Assumes there is
 * enough space available. Increments buffer pointer.
 *
 * @param bufp Address of the destination buffer
 * @param bval The boolean value
 */
void encode_bool(uint8_t** bufp, bool bval);

/**
 * Decodes a boolean value from the given buffer. Increments buffer pointer
 * and decrements remainp on success.
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to number of bytes remaining in buffer
 * @return The boolean value
 */
bool decode_bool(const uint8_t** bufp, size_t* remainp);

/**
 * Encode a 16-bit integer in little-endian order
 *
 * @param bufp Pointer to the destination buffer
 * @param val The value to encode
 */
void encode_i16(uint8_t** bufp , uint16_t val);

/**
 * Decode a 16-bit integer in little-endian order
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to the remaining size variable
 * @return The decoded value
 */
uint16_t decode_i16(const uint8_t** bufp, size_t* remainp);

/**
 * Encode a 24-bit integer in little-endian order
 *
 * @param bufp Pointer to the destination buffer pointer
 * @param val The value to encode
 */
void encode_i24(uint8_t** bufp, uint24_t val);

/**
 * Decode a 24-bit integer in little-endian order
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to the remaining size variable
 * @return The decoded value
 */
uint24_t decode_i24(const uint8_t** bufp, size_t* remainp);

/**
 * Encode a 32-bit integer in little-endian order
 *
 * @param bufp Pointer to the destination buffer pointer
 * @param val The value to encode
 */
void encode_i32(uint8_t** bufp, uint32_t val);

/**
 * Decode a 32-bit integer in little-endian order
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to the remaining size variable
 * @return The decoded value
 */
uint32_t decode_i32(const uint8_t** bufp, size_t* remainp);

/**
 * Encode a 64-bit integer in little-endian order
 *
 * @param bufp Pointer to the destination buffer pointer
 * @param val The value to encode
 */
void encode_i64(uint8_t** bufp, uint64_t val);

/**
 * Decode a 64-bit integer in little-endian order
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to the remaining size variable
 * @return The decoded value
 */
uint64_t decode_i64(const uint8_t** bufp, size_t* remainp);

/**
 * Length of a variable length encoded 24-bit integer (up to 4 bytes)
 *
 * @param val The 24-bit integer to encode
 * @return The number of bytes required for serializing this number
 */
int encoded_length_vi24(uint24_t val);

/**
 * Length of a variable length encoded 32-bit integer (up to 5 bytes)
 *
 * @param val The 32-bit integer to encode
 * @return The number of bytes required for serializing this number
 */
int encoded_length_vi32(uint32_t val);

/**
 * Length of a variable length encoded 64-bit integer (up to 9 bytes)
 *
 * @param val The 64-bit integer to encode
 * @return The number of bytes required for serializing this number
 */
int encoded_length_vi64(uint64_t val);

/**
 * Encode a integer (up to 24-bit) in variable length encoding
 *
 * @param bufp Pointer to the destination buffer pointer
 * @param val The value to encode
 */
void encode_vi24(uint8_t** bufp, uint24_t val);

/**
 * Encode a integer (up to 32-bit) in variable length encoding
 *
 * @param bufp Pointer to the destination buffer pointer
 * @param val The value to encode
 */
void encode_vi32(uint8_t** bufp, uint32_t val);

/**
 * Encode a integer (up to 64-bit) in variable length encoding
 *
 * @param bufp The pointer to the destination buffer pointer
 * @param val The value to encode
 */
void encode_vi64(uint8_t** bufp, uint64_t val);

/**
 * Decode a variable length encoded integer up to 24-bit
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to the remaining size variable
 * @return The decoded value
 */
uint24_t decode_vi24(const uint8_t** bufp, size_t* remainp);

/**
 * Decode a variable length encoded integer up to 24-bit
 *
 * @param bufp Pointer to the source buffer pointer
 * @return The decoded value
 */
uint24_t decode_vi24(const uint8_t** bufp);

/**
 * Decode a variable length encoded integer up to 32-bit
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to the remaining size variable
 * @return The decoded value
 */
uint32_t decode_vi32(const uint8_t** bufp, size_t* remainp);
/**
 * Decode a variable length encoded integer up to 64-bit
 *
 * @param bufp Pointer to the source buffer pointer
 * @param remainp Pointer to the remaining size variable
 * @return The decoded value
 */
uint64_t decode_vi64(const uint8_t** bufp, size_t* remainp);

/**
 * Decode a variable length encoded integer up to 32-bit
 *
 * @param bufp Pointer to the source buffer pointer
 * @return The decoded value
 */
uint32_t decode_vi32(const uint8_t** bufp);

/**
 * Decode a variable length encoded integer up to 64-bit
 *
 * @param bufp Pointer to the source buffer pointer
 * @return The decoded value
 */
uint64_t decode_vi64(const uint8_t** bufp);

/**
 * Computes the encoded length of a 32-bit length byte array (i32, bytes)
 *
 * @param len Length of the byte array to be encoded
 * @return The encoded length of a byte array of length len
 */
size_t encoded_length_bytes32(int32_t len);

/**
 * Encodes a variable sized byte array into the given buffer.Encoded as a
 * 4 byte length followed by the data.Assumes there is enough space
 * available.Increments buffer pointer.
 *
 * @param bufp Address of the destination buffer
 * @param data Pointer to array of bytes
 * @param len The length of the byte array
 */
void encode_bytes32(uint8_t** bufp, const void* data, int32_t len);

/**
 * Decodes a variable sized byte array from the given buffer.Byte array i
 * encoded as a 4 byte length followed by the data.Increments buffer
 * pointer and decrements remainp on success.
 *
 * @param bufp Address of buffer containing encoded byte array
 * @param remainp Address of variable containing number of bytes remaining
 * @param lenp Address of length of decoded byte array
 */
uint8_t* decode_bytes32(const uint8_t** bufp, size_t* remainp, uint32_t* lenp);

/**
 * Encodes a fixed-known sized byte array into the given buffer.Encoded as a
 * only with the data.Assumes there is enough space
 * available.Increments buffer pointer.
 *
 * @param bufp Address of the destination buffer
 * @param data Pointer to array of bytes
 * @param len The length of the byte array
 */
void encode_bytes(uint8_t** bufp, const void* data, int32_t len);

/**
 * Decodes a fixed-known sized byte array from the given buffer.Byte array
 * only the data.Increments buffer
 * pointer and decrements remainp on success.
 *
 * @param bufp Address of buffer containing encoded byte array
 * @param remainp Address of variable containing number of bytes remaining
 * @param len The length of the byte array
 */
uint8_t* decode_bytes(const uint8_t** bufp, size_t* remainp, uint32_t len);

/**
 * Computes the encoded length of a string16 encoding
 *
 * @param str Pointer to the c-style string
 * @return The encoded length of str
 */
size_t encoded_length_str16(const char* str);

/**
 * Computes the encoded length of a std::string.
 *
 * @param str Reference to string object
 * @return The encoded length of str
 */
size_t encoded_length_str16(const std::string& str);

/**
 * Encodes a string buffer into the given buffer.
 * Encoded as a 2 byte length followed by the string data, followed
 * by a '\\0' termination byte.The length value does not include
 * the '\\0'.Assumes there is enough space available.Increments
 * buffer pointer.
 *
 * @param bufp Address of the destination buffer
 * @param str The c-style string to encode
 * @param len Length of the string
 */
void encode_str16(uint8_t** bufp, const void* str, uint16_t len);

/**
 * Encodes a c-style null-terminated string into the given buffer.
 * Encoded as a 2 byte length followed by the string data, followed
 * by a '\\0' termination byte.The length value does not include
 * the '\\0'.Assumes there is enough space available.Increments
 * buffer pointer.
 *
 * @param bufp Pointer to pointer of destination buffer
 * @param str The c-style string to encode
 */
void encode_str16(uint8_t** bufp, const char* str);

/**
 * Encodes a string into the given buffer.Encoded as a
 * 2 byte length followed by the string data, followed by a '\\0'
 * termination byte.The length value does not include the '\\0'.
 * Assumes there is enough space available.Increments buffer
 * pointer.
 *
 * @param bufp Pointer to pointer of the destinatin buffer
 * @param str The std::string to encode
 */
template <class StringT>
inline void encode_str16(uint8_t** bufp, const StringT& str) {
  encode_str16(bufp, str.c_str(), str.length());
}

/**
 * Decodes a c-style string from the given buffer.The encoding of the
 * string is a 2 byte length followed by the string, followed by a '\\0'
 * termination byte.The length does not include the '\\0' terminator.
 * The decoded string pointer points back into the encoding buffer.
 * Increments buffer pointer and decrements remainp on success.
 *
 * @param bufp Pointer to pointer of buffer containing encoded string
 * @param remainp Address of variable of number of bytes remaining in buffer
 * @return Pointer to a c-style string
 */
const char* decode_str16(const uint8_t** bufp, size_t* remainp);

/**
 * Decodes a c-style string from the given buffer.The encoding of the
 * string is a 2 byte length followed by the string, followed by a '\\0'
 * termination byte.The length does not include the '\\0' terminator.
 * The decoded string pointer points back into the encoding buffer.
 * Increments buffer pointer and decrements remainp on success.
 *
 * @param bufp Pointer to pointer of buffer containing encoded string
 * @param remainp Address of variable of number of bytes remaining in buffer
 * @param lenp Address of varaible to hold the len of the string
 * @return Pointer to a c-style string
 */
char* decode_str16(const uint8_t** bufp, size_t* remainp, uint16_t *lenp);

/**
 * Decodes a str16 from the given buffer to a StringT.The encoding of the
 * string is a 2 byte length followed by the string, followed by a '\\0'
 * termination byte.The length does not include the '\\0' terminator.
 * Increments buffer pointer and decrements remainp on success.
 *
 * @param bufp Pointer to pointer of buffer containing encoded string
 * @param remainp Address of variable of number of bytes remaining in buffer
 * @return StringT
 */
template <class StringT>
inline StringT decode_str16(const uint8_t** bufp, size_t* remainp) {
  uint16_t len;
  char* str = decode_str16(bufp, remainp, &len);
  return StringT(str, len); // RVO hopeful
}

/**
 * Computes the encoded length of vstr (vint64, data, null)
 *
 * @param len The string length
 * @return The encoded length of str
 */
size_t encoded_length_vstr(size_t len);

/**
 * Computes the encoded length of vstr. Assumes that the string length
 * can be encoded in 32-bit integer
 *
 * @param s Pointer to the the c-style string
 * @return The encoded length of s
 */
size_t encoded_length_vstr(const char* s);

/**
 * Computes the encoded length of vstr. Assumes that the string length
 * can be encoded in 32-bit integer
 *
 * @param s The string to encode
 * @return The encoded length of s
 */
size_t encoded_length_vstr(const std::string& s);

/**
 * Encode a buffer as variable length string (vint64, data, null)
 *
 * @param bufp Pointer to pointer of destination buffer
 * @param buf Pointer to the start of the input buffer
 * @param len Length of the input buffer
 */
void encode_vstr(uint8_t** bufp, const void* buf, size_t len);

/**
 * Encode a c-style string as vstr
 *
 * @param bufp Pointer to pointer of destination buffer
 * @param s Pointer to the start of the string
 */
void encode_vstr(uint8_t** bufp, const char* s);

/**
 * Encode a std::string as vstr
 *
 * @param bufp Pointer to pointer of destination buffer
 * @param s The string to encode
 */
template <class StringT>
inline void encode_vstr(uint8_t** bufp, const StringT& s) {
  encode_vstr(bufp, s.data(), s.length());
}

/**
 * Decode a vstr (vint64, data, null).
 * Note: decoding a vstr longer than 4GiB on a 32-bit platform
 * is obviously futile (throws bad vstr exception)
 *
 * @param bufp Pointer to pointer of the source buffer
 * @param remainp Pointer to the remaining size variable
 * @return Pointer to the decoded string data
 */
char* decode_vstr(const uint8_t** bufp, size_t* remainp);

/**
 * Decode a vstr (vint64, data, null)
 *
 * @param bufp Pointer to pointer of the source buffer
 * @param remainp Pointer to the remaining size variable
 * @param lenp Pointer to the string length
 * @return Pointer to the decoded string
 */
char* decode_vstr(const uint8_t** bufp, size_t* remainp, uint32_t* lenp);

/**
 * Decode a vstr (vint64, data, null)
 *
 * @param bufp Pointer to pointer of the source buffer
 * @param remainp Pointer to the remaining size variable
 * @return The decoded string
 */
template <class StringT>
inline StringT decode_vstr(const uint8_t** bufp, size_t* remainp) {
  uint32_t len;
  char* buf = decode_vstr(bufp, remainp, &len);
  return StringT(buf, len); // RVO
}

/**
 * Encodes a double with 18 decimal digits of precision as 64-bit
 * left-of-decimal, followed by 64-bit right-of-decimal, both in
 * little-endian order
 *
 * @param bufp Pointer to the destination buffer
 * @param val The double to encode
 */
void encode_double(uint8_t** bufp, double val);

/**
 * Decodes a double as 64-bit left-of-decimal, followed by
 * 64-bit right-of-decimal, both in little-endian order
 *
 * @param bufp Pointer to pointer of the source buffer
 * @param remainp Pointer to remaining size variable
 * @return The decoded value
 */
double decode_double(const uint8_t** bufp, size_t* remainp);

/**
 * Length of an encoded double (16 bytes)
 *
 * @return The number of bytes required to encode a double
 */
int encoded_length_double();

/**
 * Compare doubles that may have been serialized and unserialized.
 * The serialization process loses precision, so this function is
 * necessary to compare pre-serialized with post-serialized values
 *
 * @param a First value to compare
 * @param b Second value to compare
 * @return True of values are logically equal, false otherwise
 */
bool equal(double a, double b);


}} // namespace SWC::Serialization


//#ifdef SWC_IMPL_SOURCE 
#include "swcdb/core/Serialization.cc"
//#endif 

#endif
