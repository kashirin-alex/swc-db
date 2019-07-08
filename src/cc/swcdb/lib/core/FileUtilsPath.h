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

/** @file
 * Compatibility class for boost::filesystem::path.
 * This class implements several functions that missed in boost prior to 1.36+.
 */

#ifndef swc_core_FileUtilsPath_h
#define swc_core_FileUtilsPath_h

#include <boost/version.hpp>
#include <boost/filesystem.hpp>

#include "String.h"


namespace SWC {

/** @addtogroup Common
 *  @{
 */

/**
 * Compatibility class for boost::filesystem::path.
 * This class implements several functions that missed in boost prior to 1.36+.
 */
class Path : public boost::filesystem::path {
  typedef boost::filesystem::path Parent;

public:
  Path() { }
  Path(const String &s) : Parent(s) { }
  Path(const char *s) : Parent(s) { }
  Path(const Parent &path) : Parent(path) { }

  Path &operator=(const Parent &path) {
    Parent::operator=(path);
    return *this;
  }

#if BOOST_VERSION < 103600
  bool has_parent_path() const { return has_branch_path(); }
  Path parent_path() const { return branch_path(); }
  String filename() const { return leaf(); }
#endif
};

/** @} */

} // namespace SWC

#endif // swc_core_FileUtilsPath_h
