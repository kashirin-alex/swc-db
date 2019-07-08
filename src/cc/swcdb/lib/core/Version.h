/**
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
 * SWC version information. This file is modified by cmake!
 */

#ifndef swc_core_config_Version_h
#define swc_core_config_Version_h

#include <cstdio>
#include <string>

// version macros for detecting header/library mismatch
#define SWC_VERSION_MAJOR        0
#define SWC_VERSION_MINOR        9
#define SWC_VERSION_MICRO        9
#define SWC_VERSION_PATCH        0
#define SWC_VERSION_MISC_SUFFIX  ""
#define SWC_VERSION_STRING       ""

namespace SWC {

  /** @addtogroup Common
   *  @{
   */

  /** The major version */
  extern const int version_major;

  /** The minor version */
  extern const int version_minor;

  /** The micro version */
  extern const int version_micro;

  /** The patch version (usually not used) */
  extern const int version_patch;

  /**
   * Returns SWC_VERSION_MISC_SUFFIX as it was compiled/linked into Hypertable,
   * and is not inlined like %check_version below
   */
  extern const std::string version_misc_suffix;

  /**
   * Returns SWC_VERSION_STRING as it was compiled/linked into Hypertable,
   * and is not inlined like %check_version below
   */
  extern const char *version_string();

  /**
   * Compares the version from this header file with the version of the
   * compiled Hypertable libraries; will exit() if not equal.
   *
   * This function is inlined, otherwise it would not use the header file
   * versions but also those from the compiled libraries.
   */
  inline void check_version() {
    if (version_major != SWC_VERSION_MAJOR ||
        version_minor != SWC_VERSION_MINOR ||
        version_micro != SWC_VERSION_MICRO ||
        version_patch != SWC_VERSION_PATCH ||
        version_misc_suffix != SWC_VERSION_MISC_SUFFIX) {
      std::fprintf(stderr, "SWC-DB header/library version mismatch:\n"
              " header: %s\nlibrary: %s\n", SWC_VERSION_STRING,
              version_string());
      exit(1);
    }
  }

  /** @} */
}

#endif // swc_core_config_Version_h
