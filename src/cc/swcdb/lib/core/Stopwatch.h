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

/// @file
/// The Stopwatch measures elapsed time.

#ifndef swc_core_Stopwatch_h
#define swc_core_Stopwatch_h

#include <chrono>
#include <cstring>

namespace SWC {

  /// @addtogroup Common
  /// @{

  /**
   * The Stopwatch class measures elapsed time between instantiation (or a
   * call to @a start) and a call to @a stop.
   */
  class Stopwatch {
  public:
    /** Constructor; if @a start_running is true then the Stopwatch is started
     * immediately */
    Stopwatch(bool start_running = true) {
      if (start_running)
        start();
    }

    /** Starts the Stopwatch */
    void start() {
      if (!m_running) {
        start_time = std::chrono::steady_clock::now();
        m_running = true;
      }
    }

    /** Stops the Stopwatch. Has no effect if the Stopwatch was not running. */
    void stop() {
      if (m_running) {
        elapsed_time += std::chrono::steady_clock::now() - start_time;
        m_running = false;
      }
    }

    /** Resets the Stopwatch */
    void reset() {
      elapsed_time = std::chrono::steady_clock::duration::zero();
    }

    /** Returns the elapsed time. Can be called while the Stopwatch is running;
     * in this case the Stopwatch will continue to run. */
    double elapsed() {
      if (m_running) {
        stop();
        start();
      }
      return ((double)elapsed_time.count() *
              std::chrono::steady_clock::duration::period::num) /
        (double)std::chrono::steady_clock::duration::period::den;
    }

    /// Returns elapsed time in milliseconds.
    /// @return Elapsed time in milliseconds.
    int64_t elapsed_millis() {
      if (m_running) {
        stop();
        start();
      }
      return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count();
    }

  private:
    /// Flag whether the Stopwatch is currently running
    bool m_running {};

    /// The start time
    std::chrono::steady_clock::time_point start_time;

    /// The elapsed time
    std::chrono::steady_clock::duration elapsed_time {0};
  };

  /// @}

}

#endif // swc_core_Stopwatch_h
