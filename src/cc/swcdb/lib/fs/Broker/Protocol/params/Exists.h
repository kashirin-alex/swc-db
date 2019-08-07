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
/// Declarations for Exists request parameters.
/// This file contains declarations for Exists, a class for encoding and
/// decoding paramters to the <i>exists</i> file system broker function.

#ifndef swc_lib_fs_Broker_Protocol_params_ExistsReq_h
#define swc_lib_fs_Broker_Protocol_params_ExistsReq_h

#include "swcdb/lib/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class ExistsReq : public Serializable {
  public:

  ExistsReq() {}

  ExistsReq(const std::string &fname) : m_fname(fname) {}

  const std::string get_fname() { return m_fname; }

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
    return Serialization::encoded_length_vstr(m_fname);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_vstr(bufp, m_fname);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    m_fname.clear();
    m_fname.append(Serialization::decode_vstr(bufp, remainp));
  }

  std::string m_fname;
};

class ExistsRsp : public Serializable {
  public:
  
  ExistsRsp() {}

  ExistsRsp(bool exists) : m_exists(exists) {}

  bool get_exists() { return m_exists; }

  private:

  uint8_t encoding_version() const override {
    return 1;
  }

  size_t encoded_length_internal() const override {
    return 1;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_bool(bufp, m_exists);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
	                     size_t *remainp) override {
    m_exists = Serialization::decode_bool(bufp, remainp);
  }
  
  bool m_exists;
};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_ExistsReq_h
