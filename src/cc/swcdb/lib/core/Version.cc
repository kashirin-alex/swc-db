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
 * SWC version information
 */

#include "Compat.h"
#include "Version.h"

namespace SWC {

const int version_major = SWC_VERSION_MAJOR;
const int version_minor = SWC_VERSION_MINOR;
const int version_micro = SWC_VERSION_MICRO;
const int version_patch = SWC_VERSION_PATCH;
const std::string version_misc_suffix = SWC_VERSION_MISC_SUFFIX;

const char *version_string() {
  return SWC_VERSION_STRING;
}

} // namespace SWC
