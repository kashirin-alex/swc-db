/*
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
/// Declarations for SockAddrMap.
/// This file contains declarations for SockAddrMap, a template specialization
/// for an unordered_map with InetAddr keys.

#ifndef swc_core_SockAddrMap_h
#define swc_core_SockAddrMap_h

#include "InetAddr.h"

#include <unordered_map>

namespace SWC {

  /// @addtogroup Common
  /// @{

  /// Hash functor class for InetAddr.
  class SockAddrHash {
  public:
    size_t operator () (const InetAddr &addr) const {
      return (size_t)(addr.sin_addr.s_addr ^ addr.sin_port);
    }
  };

  /// Equality predicate functor class for InetAddr.
  struct SockAddrEqual {
    bool operator()(const InetAddr &addr1, const InetAddr &addr2) const {
      return (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr)
        && (addr1.sin_port == addr2.sin_port);
    }
  };

  /// Unordered map specialization for InetAddr keys.
  template<typename TypeT, typename addr = InetAddr>
    class SockAddrMap : public std::unordered_map<addr, TypeT, SockAddrHash, SockAddrEqual> {
  };

  /// @}

} // namespace SWC

#endif // swc_core_SockAddrMap_h
