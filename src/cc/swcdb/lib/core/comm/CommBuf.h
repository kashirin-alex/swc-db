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

/** @file
 * Declarations for CommBuf.
 * This file contains type declarations for CommBuf, a class to create and
 * manipulate messages to be transmitted over a network.
 */

#ifndef swc_core_comm_CommBuf_h
#define swc_core_comm_CommBuf_h

#include <memory>
#include <string>

#include "../Logger.h"
#include "../Serializable.h"
#include "../StaticBuffer.h"

#include "CommHeader.h"

namespace SWC {


class CommBuf {
  public:

  typedef std::shared_ptr<CommBuf> Ptr;

  inline Ptr static make(uint32_t reserve=0) {
    return std::make_shared<CommBuf>(reserve);
  }

  inline Ptr static make(const Serializable& params, uint32_t reserve=0) {
    return std::make_shared<CommBuf>(params, reserve);
  }

  inline Ptr static make(const Serializable& params, StaticBuffer& buffer, 
                         uint32_t reserve=0) {
    return std::make_shared<CommBuf>(params, buffer, reserve);
  }

  inline Ptr static make(StaticBuffer& buffer) {
    return std::make_shared<CommBuf>(buffer);
  }

  CommBuf(uint32_t reserve=0) {
    if(reserve)
      set_data(reserve);
  }

  CommBuf(const Serializable& params, uint32_t reserve=0) {
    set_data(params, reserve);
  }

  CommBuf(const Serializable& params, StaticBuffer& buffer, 
          uint32_t reserve=0) : buf_ext(buffer) {
    set_data(params, reserve);
  }

  CommBuf(StaticBuffer& buffer) : buf_ext(buffer) {
  }

  virtual ~CommBuf() { }

  void set_data(uint32_t sz) {
    buf_data.reallocate(sz);
    data_ptr = buf_data.base; 
  }

  void set_data(const Serializable& params, uint32_t reserve) {
    set_data(reserve + params.encoded_length());

    data_ptr = buf_data.base + reserve;
    params.encode(&data_ptr);
    data_ptr = buf_data.base;
  }

  void write_header() {
    if(buf_data.size) {
      header.data_size   = buf_data.size;
      header.data_chksum = fletcher32(buf_data.base, buf_data.size);
    }
    if(buf_ext.size) {  
      header.data_ext_size   = buf_ext.size;
      header.data_ext_chksum = fletcher32(buf_ext.base, buf_ext.size);
    }
    buf_header.reallocate(header.encoded_length());
    uint8_t *buf = buf_header.base;
    header.encode(&buf);
  }

  void get(std::vector<asio::const_buffer>& buffers) {

    write_header();
    buffers.push_back(asio::buffer(buf_header.base, buf_header.size));

    if(buf_data.size) 
      buffers.push_back(asio::buffer(buf_data.base, buf_data.size));
    if(buf_ext.size) 
      buffers.push_back(asio::buffer(buf_ext.base, buf_ext.size));
  }

  /** Returns the primary buffer internal data pointer
   */
  void *get_data_ptr() { return data_ptr; }

  /** Returns address of the primary buffer internal data pointer
   */
  uint8_t **get_data_ptr_address() { return &data_ptr; }

  /** Advance the primary buffer internal data pointer by <code>len</code>
   * bytes
   * @param len the number of bytes to advance the pointer by
   * @return returns the advanced internal data pointer
   */
  void *advance_data_ptr(size_t len) { data_ptr += len; return data_ptr; }

  /** Appends a boolean value to the primary buffer.  After appending, this
   * method advances the primary buffer internal data pointer by 1
   * @param bval Boolean value to append to primary buffer
   */
  void append_bool(bool bval) { Serialization::encode_bool(&data_ptr, bval); }

  /** Appends a byte of data to the primary buffer.  After appending, this
   * method advances the primary buffer internal data pointer by 1
   * @param bval byte value to append into buffer
   */
  void append_byte(uint8_t bval) { *data_ptr++ = bval; }

  /** Appends a sequence of bytes to the primary buffer.  After appending,
   * this method advances the primary buffer internal data pointer by the
   * number of bytes appended
   * @param bytes Starting address of byte sequence
   * @param len Number of bytes in sequence
   */
  void append_bytes(const uint8_t *bytes, uint32_t len) {
    memcpy(data_ptr, bytes, len);
    data_ptr += len;
  }

  /** Appends a c-style string to the primary buffer.  A string is encoded
   * as a 16-bit length, followed by the characters, followed by
   * a terminating '\\0'.
   *
   * @param str c-style string to append
   * @see Serialization::encode_str16
   */
  void append_str16(const char *str) {
    Serialization::encode_str16(&data_ptr, str);
  }

  /**
   * Appends a String to the primary buffer.  A string is encoded as
   * a 16-bit length, followed by the characters, followed by
   * a terminating '\\0'.
   *
   * @param str std string to append
   * @see Serialization::encode_str16
   */
  void append_str16(const String &str) {
    Serialization::encode_str16(&data_ptr, str);
  }

  /**
   * Appends a 16-bit integer to the primary buffer.  The integer is encoded
   * in little endian order and the primary buffer internal data pointer is
   * advanced to the position immediately following the encoded integer.
   * @param sval Two-byte short integer to append into buffer
   */
  void append_i16(uint16_t sval) {
    Serialization::encode_i16(&data_ptr, sval);
  }

  /** Appends a 32-bit integer to the primary buffer.  The integer is encoded
   * in little endian order and the primary buffer internal data pointer is
   * advanced to the position immediately following the encoded integer.
   * @param ival Four-byte integer value to append into buffer
   */
  void append_i32(uint32_t ival) {
    Serialization::encode_i32(&data_ptr, ival);
  }

  /** Appends a 64-bit integer to the primary buffer.  The integer is encoded
   * in little endian order and the primary buffer pointer is advanced
   * to the position immediately following the encoded integer.
   * @param lval Eight-byte long integer value to append into buffer
   */
  void append_i64(uint64_t lval) {
    Serialization::encode_i64(&data_ptr, lval);
  }

  /** Appends a c-style string to the primary buffer.  A string is encoded
   * as a vint64 length, followed by the characters, followed by
   * a terminating '\\0'.
   *
   * @param str C-style string to append
   * @see Serialization::encode_vstr
   */
  void append_vstr(const char *str) {
    Serialization::encode_vstr(&data_ptr, str);
  }

  /** Appends a String to the primary buffer.  A string is encoded as
   * a vint64 length, followed by the characters, followed by
   * a terminating '\\0'.
   * @param str C++ string to append
   * @see Serialization::encode_vstr
   */
  void append_vstr(const String &str) {
    Serialization::encode_vstr(&data_ptr, str);
  }

  /** Appends a variable sized string to the primary buffer.  The
   * string is encoded as a vint length, followed by the bytes
   * (followed by a terminating '\\0').
   *
   * @param str C-style string to encode
   * @param len Length of string
   * @see Serialization::encode_vstr
   */
  void append_vstr(const void *str, uint32_t len) {
    Serialization::encode_vstr(&data_ptr, str, len);
  }

  CommHeader            header;         //!< Comm header
  StaticBuffer          buf_header;     //!< Header buffer
  StaticBuffer          buf_data;       //!< Primary data buffer
  StaticBuffer          buf_ext;        //!< Extended buffer

  protected:

  /// Write pointer into #buf_data
  uint8_t   *data_ptr;

};


} // namespace SWC


#endif // swc_core_comm_CommBuf_h
