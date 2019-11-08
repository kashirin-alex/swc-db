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
 * Retrieves system information (hardware, installation directory, etc)
 */

#ifndef swc_core_config_System_h
#define swc_core_config_System_h

#include "../String.h"
#include "../Version.h"

#include <ctime>
#include <memory>
#include <mutex>

namespace SWC {

  /** @addtogroup Common
   *  @{
   */

  struct CpuInfo;
  struct CpuStat;
  struct LoadAvgStat;
  struct MemStat;
  struct DiskStat;
  struct OsInfo;
  struct SwapStat;
  struct NetInfo;
  struct NetStat;
  struct ProcInfo;
  struct ProcStat;
  struct FsStat;
  struct TermInfo;

  /** @file
   * Retrieves system information (hardware, installation directory, etc)
   */
  class System {
  public:
    /**
     * Initializes the static class members; checks header version vs. library
     * version, initializes Logging and other stuff.
     *
     * Must be inlined to do proper version check
     *
     * @param install_directory Path of the installation directory. If empty
     *      then will try to find the installation directory by using argv[0]
     *      in combination with some heuristics
     */
    static inline void initialize(const String &install_directory = String()) {
      std::lock_guard<std::mutex> lock(ms_mutex);

      if (ms_initialized)
        return;

      // Set timezone
      tzset();

#if !defined(__sun__)
      // Set timezone variables
      {
        struct tm tmbuf;
        time_t now = time(0);
        localtime_r(&now, &tmbuf);
        tm_gmtoff = tmbuf.tm_gmtoff;
        tm_zone = String(tmbuf.tm_zone);
      }
#endif

      check_version();
      _init(install_directory);
      ms_initialized = true;
    }


    /** Returns the installation directory
     *
     * @param argv0 argv[0] of the main method
     * @return The (presumed) installation directory
     */
    static String locate_install_dir(const char *argv0);

    /** Returns the installation directory; same as %locate_install_dir but
     * does not lock %ms_mutex.
     *
     * @param argv0 argv[0] of the main method
     * @return The (presumed) installation directory
     */
    static String _locate_install_dir(const char *argv0);

    /** The installation directory */
    static String install_dir;

    /** The exe file name */
    static String exe_name;

    /// Seconds east of UTC
    static long tm_gmtoff;

    /// Timezone abbreviation
    static String tm_zone;

    /// Initialize struct tm
    static void initialize_tm(struct tm *tmval);

    /** The processor count */
    static int32_t get_processor_count();

    /** The pid of the current process */
    static int32_t get_pid();

    /** Returns the number of drives */
    static int32_t get_drive_count();

    /** Retrieves updated CPU information (see SystemInfo.h) */
    static const CpuInfo &cpu_info();

    /** Retrieves updated CPU statistics (see SystemInfo.h) */
    static const CpuStat &cpu_stat();

    /** Retrieves updated Memory statistics (see SystemInfo.h) */
    static const MemStat &mem_stat();

    /** Retrieves updated Disk statistics (see SystemInfo.h) */
    static const DiskStat &disk_stat();

    /** Retrieves updated Operating system information (see SystemInfo.h) */
    static const OsInfo &os_info();

    /** Retrieves updated Swap statistics (see SystemInfo.h) */
    static const SwapStat &swap_stat();

    /** Retrieves updated Network information (see SystemInfo.h) */
    static const NetInfo &net_info();

    /** Retrieves updated Network statistics (see SystemInfo.h) */
    static const NetStat &net_stat();

    /** Retrieves updated Process information (see SystemInfo.h) */
    static const ProcInfo &proc_info();

    /** Retrieves updated Process statistics (see SystemInfo.h) */
    static const ProcStat &proc_stat();

    /** Retrieves updated Filesystem statistics (see SystemInfo.h) */
    static const FsStat &fs_stat();

    /** Retrieves updated Terminal information (see SystemInfo.h) */
    static const TermInfo &term_info();

    /** Retrieves updated Load average statistics (see SystemInfo.h) */
    static const LoadAvgStat &loadavg_stat();

  private:
    /** Internal initialization helper */
    static void _init(const String &install_directory);

    /** true if %initialize was already called */
    static bool ms_initialized;

    /** a %Mutex to protect the static members */
    static std::mutex ms_mutex;

  };

  /** @} */

}

#endif // swc_core_config_System_h
