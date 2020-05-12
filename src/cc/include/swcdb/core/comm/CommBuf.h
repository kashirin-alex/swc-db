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

/** @file
 * Declarations for CommBuf.
 * This file contains type declarations for CommBuf, a class to create and
 * manipulate messages to be transmitted over a network.
 */

#ifndef swc_core_comm_CommBuf_h
#define swc_core_comm_CommBuf_h

#include <asio.hpp>
#include "swcdb/core/Serializable.h"
#include "swcdb/core/StaticBuffer.h"
#include "swcdb/core/comm/CommHeader.h"

#include <memory>
#include <string>
#include <vector>

namespace SWC {


class CommBuf final {
  public:

  typedef std::shared_ptr<CommBuf> Ptr;

  static Ptr make(uint32_t reserve=0);

  static Ptr make(const Serializable& params, uint32_t reserve=0);

  static Ptr make(const Serializable& params, StaticBuffer& buffer, 
                  uint32_t reserve=0);

  static Ptr make(StaticBuffer& buffer, uint32_t reserve=0);

  static Ptr create_error_message(int error, const char *msg);


  CommBuf(uint32_t reserve=0);

  CommBuf(const Serializable& params, uint32_t reserve=0);

  CommBuf(const Serializable& params, StaticBuffer& buffer, 
          uint32_t reserve=0);

  CommBuf(StaticBuffer& buffer, uint32_t reserve=0);

  ~CommBuf();

  void set_data(uint32_t sz);

  void set_data(const Serializable& params, uint32_t reserve);

  void write_header();

  std::vector<asio::const_buffer> get_buffers();

  /** Returns the primary buffer internal data pointer
   */
  void *get_data_ptr();

  /** Returns address of the primary buffer internal data pointer
   */
  uint8_t **get_data_ptr_address();

  /** Advance the primary buffer internal data pointer by <code>len</code>
   * bytes
   * @param len the number of bytes to advance the pointer by
   * @return returns the advanced internal data pointer
   */
  void *advance_data_ptr(size_t len);

  /** Appends a boolean value to the primary buffer.  After appending, this
   * method advances the primary buffer internal data pointer by 1
   * @param bval Boolean value to append to primary buffer
   */
  void append_bool(bool bval);

  /** Appends a byte of data to the primary buffer.  After appending, this
   * method advances the primary buffer internal data pointer by 1
   * @param bval byte value to append into buffer
   */
  void append_byte(uint8_t bval);

  /** Appends a sequence of bytes to the primary buffer.  After appending,
   * this method advances the primary buffer internal data pointer by the
   * number of bytes appended
   * @param bytes Starting address of byte sequence
   * @param len Number of bytes in sequence
   */
  void append_bytes(const uint8_t *bytes, uint32_t len);

  /** Appends a c-style string to the primary buffer.  A string is encoded
   * as a 16-bit length, followed by the characters, followed by
   * a terminating '\\0'.
   *
   * @param str c-style string to append
   * @see Serialization::encode_str16
   */
  void append_str16(const char *str);

  /**
   * Appends a std::string to the primary buffer.  A string is encoded as
   * a 16-bit length, followed by the characters, followed by
   * a terminating '\\0'.
   *
   * @param str std string to append
   * @see Serialization::encode_str16
   */
  void append_str16(const std::string &str);

  /**
   * Appends a 16-bit integer to the primary buffer.  The integer is encoded
   * in little endian order and the primary buffer internal data pointer is
   * advanced to the position immediately following the encoded integer.
   * @param sval Two-byte short integer to append into buffer
   */
  void append_i16(uint16_t sval);

  /** Appends a 32-bit integer to the primary buffer.  The integer is encoded
   * in little endian order and the primary buffer internal data pointer is
   * advanced to the position immediately following the encoded integer.
   * @param ival Four-byte integer value to append into buffer
   */
  void append_i32(uint32_t ival);

  /** Appends a 64-bit integer to the primary buffer.  The integer is encoded
   * in little endian order and the primary buffer pointer is advanced
   * to the position immediately following the encoded integer.
   * @param lval Eight-byte long integer value to append into buffer
   */
  void append_i64(uint64_t lval);

  /** Appends a c-style string to the primary buffer.  A string is encoded
   * as a vint64 length, followed by the characters, followed by
   * a terminating '\\0'.
   *
   * @param str C-style string to append
   * @see Serialization::encode_vstr
   */
  void append_vstr(const char *str);

  /** Appends a std::string to the primary buffer.  A string is encoded as
   * a vint64 length, followed by the characters, followed by
   * a terminating '\\0'.
   * @param str C++ string to append
   * @see Serialization::encode_vstr
   */
  void append_vstr(const std::string &str);

  /** Appends a variable sized string to the primary buffer.  The
   * string is encoded as a vint length, followed by the bytes
   * (followed by a terminating '\\0').
   *
   * @param str C-style string to encode
   * @param len Length of string
   * @see Serialization::encode_vstr
   */
  void append_vstr(const void *str, uint32_t len);

  CommHeader            header;         //!< Comm header
  StaticBuffer          buf_header;     //!< Header buffer
  StaticBuffer          buf_data;       //!< Primary data buffer
  StaticBuffer          buf_ext;        //!< Extended buffer

  protected:

  /// Write pointer into #buf_data
  uint8_t   *data_ptr;

};


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/CommBuf.cc"
#endif 

#endif // swc_core_comm_CommBuf_h
