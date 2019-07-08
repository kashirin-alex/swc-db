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

/** @file
 * Random number generator for int32, int64, double and ascii arrays.
 */

#ifndef swc_core_Random_h
#define swc_core_Random_h

#include <chrono>

namespace SWC {

  /// @addtogroup Common
  /// @{

  /// Convenience interface for random number and data generation.
  class Random {
  public:

    /// Sets the seed of the random number generator.
    /// @param s Seed value
    static void seed(unsigned int s);

    /// Returns a random 32-bit unsigned integer.
    /// @return Random 32-bit integer in the range [0,<code>maximum</code>)
    static uint32_t number32(uint32_t maximum = 0);

    /// Returns a random 64-bit unsigned integer.
    /// @return Random 64-bit integer in the range [0,<code>maximum</code>)
    static int64_t number64(int64_t maximum = 0);

    /// Returns a random double.
    /// @return Random double in the range [0.0, 1.0)
    static double uniform01();

    /// Returns a random millisecond duration.
    /// @return Random millisecond duration in the range [0,<code>maximum</code>)
    static std::chrono::milliseconds duration_millis(uint32_t maximum);

  };

  /// @}

}

#endif // swc_core_Random_h
