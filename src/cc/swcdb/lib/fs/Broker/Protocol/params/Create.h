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
/// Declarations for Create parameters.

#ifndef swc_lib_fs_Broker_Protocol_params_Create_h
#define swc_lib_fs_Broker_Protocol_params_Create_h

#include "swcdb/lib/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class CreateReq : public Serializable {
  public:

  CreateReq() {}

  CreateReq(const std::string &fname, uint32_t flags, int32_t bufsz,
	          int32_t replication, int64_t blksz)
            : m_fname(fname), m_flags(flags), m_bufsz(bufsz),
	            m_replication(replication), m_blksz(blksz) {}

  const std::string get_name() { return m_fname; }

  uint32_t get_flags() { return m_flags; }

  int32_t get_buffer_size() { return m_bufsz; }

  int32_t get_replication() { return m_replication; }

  int64_t get_block_size() { return m_blksz; }

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
    return 20 + Serialization::encoded_length_vstr(m_fname);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, m_flags);
    Serialization::encode_i32(bufp, m_bufsz);
    Serialization::encode_i32(bufp, m_replication);
    Serialization::encode_i64(bufp, m_blksz);
    Serialization::encode_vstr(bufp, m_fname);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    m_flags = Serialization::decode_i32(bufp, remainp);
    m_bufsz = (int32_t)Serialization::decode_i32(bufp, remainp);
    m_replication = (int32_t)Serialization::decode_i32(bufp, remainp);
    m_blksz = (int64_t)Serialization::decode_i64(bufp, remainp);
    m_fname.clear();
    m_fname.append(Serialization::decode_vstr(bufp, remainp));
  }

  /// File name
  std::string m_fname;

  /// Create flags
  uint32_t m_flags;

  /// Buffer size
  int32_t m_bufsz;

  /// Replication
  int32_t m_replication;

  /// Block size
  int64_t m_blksz;
};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Create_h
