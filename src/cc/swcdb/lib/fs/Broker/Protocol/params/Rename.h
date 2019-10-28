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
/// Declarations for Rename parameters.

#ifndef swc_lib_fs_Broker_Protocol_params_Rename_h
#define swc_lib_fs_Broker_Protocol_params_Rename_h

#include "swcdb/lib/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class RenameReq : public Serializable {
  public:

  RenameReq() {}

  RenameReq(const std::string &from, const std::string &to) 
            : m_from(from), m_to(to) {}

  const std::string& get_from() { return m_from; }
  
  const std::string& get_to() { return m_to; }

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
  return Serialization::encoded_length_vstr(m_from)
       + Serialization::encoded_length_vstr(m_to);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_vstr(bufp, m_from);
    Serialization::encode_vstr(bufp, m_to);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    m_from.clear();
    m_from.append(Serialization::decode_vstr(bufp, remainp));
    m_to.clear();
    m_to.append(Serialization::decode_vstr(bufp, remainp));
  }

  std::string m_from;
  std::string m_to;
};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Rename_h
