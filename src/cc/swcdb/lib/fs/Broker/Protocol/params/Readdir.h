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
/// Declarations for Readdir parameters.
/// This file contains declarations for Readdir, a class for encoding and
/// decoding paramters to the <i>exists</i> file system broker function.

#ifndef swc_lib_fs_Broker_Protocol_params_Readdir_h
#define swc_lib_fs_Broker_Protocol_params_Readdir_h

#include "swcdb/lib/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class ReaddirReq : public Serializable {
  public:

  ReaddirReq() {}

  ReaddirReq(const std::string &dirname) : m_dirname(dirname) {}

  const std::string get_dirname() { return m_dirname; }

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
    return Serialization::encoded_length_vstr(m_dirname);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_vstr(bufp, m_dirname);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    m_dirname.clear();
    m_dirname.append(Serialization::decode_vstr(bufp, remainp));
  }

  std::string m_dirname;
};

class ReaddirRsp : public Serializable {
  public:
  
  ReaddirRsp() {}

  ReaddirRsp(DirentList &listing) : m_listing(listing) {}

  void get_listing(DirentList &listing) {
    listing = m_listing;
  }

  private:

  uint8_t encoding_version() const override {
    return 1;
  }

  size_t encoded_length_internal() const override {
    size_t length = 4;
    for (const Dirent &entry : m_listing)
      length += entry.encoded_length();
    return length;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, m_listing.size());
    for (const Dirent &entry : m_listing)
      entry.encode(bufp);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
	                     size_t *remainp) override {
    (void)version;
    int32_t count = (int32_t)Serialization::decode_i32(bufp, remainp);
    m_listing.reserve(count);
    Dirent entry;
    for (int32_t i=0; i<count; i++) {
      entry.decode(bufp, remainp);
      m_listing.push_back(entry);
    }
  }
  
  DirentList m_listing;
};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Readdir_h
