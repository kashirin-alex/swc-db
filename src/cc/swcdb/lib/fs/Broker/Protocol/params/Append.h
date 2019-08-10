/* -*- c++ -*-
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

/// @file
/// Declarations for Append parameters.

#ifndef swc_lib_fs_Broker_Protocol_params_Append_h
#define swc_lib_fs_Broker_Protocol_params_Append_h

#include "swcdb/lib/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class AppendReq : public Serializable {
  public:

  AppendReq() {}

  AppendReq(int32_t fd, uint32_t size, uint8_t flags)
            : m_fd(fd), m_size(size), m_flags(flags) {}

  int32_t get_fd() { return m_fd; }

  uint32_t get_size() { return m_size; }

  uint8_t get_flags() { return m_flags; }

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 9;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, m_fd);
    Serialization::encode_i32(bufp, m_size);
    Serialization::encode_i8(bufp, m_flags);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    m_fd = (int32_t)Serialization::decode_i32(bufp, remainp);
    m_size = Serialization::decode_i32(bufp, remainp);
    m_flags = Serialization::decode_i8(bufp, remainp);
  }
  
  /// File descriptor to which append applies
  int32_t m_fd {};

  /// Size of data buffer
  uint32_t m_size {};

  /// Flags (FLUSH or SYNC)
  uint8_t m_flags {};
};




class AppendRsp : public Serializable {
  public:

  AppendRsp() {}

  AppendRsp(uint64_t offset, uint32_t amount)
           : m_offset(offset), m_amount(amount) {}

  uint64_t get_offset() { return m_offset; }

  uint32_t get_amount() { return m_amount; }

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 12;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i64(bufp, m_offset);
    Serialization::encode_i32(bufp, m_amount);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    m_offset = Serialization::decode_i64(bufp, remainp);
    m_amount = Serialization::decode_i32(bufp, remainp);
  }
  
  /// Offset at which data was appended
  uint64_t m_offset {};

  /// Amount of data appended
  uint32_t m_amount {};
};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Append_h
