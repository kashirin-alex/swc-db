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

#include "SystemInfo.h"

#include "swcdb/lib/core/Logger.h"
#include "swcdb/lib/core/FileUtils.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <vector>

using namespace SWC;
using namespace std;
using namespace boost::filesystem;

string System::install_dir;
string System::exe_name;
long System::tm_gmtoff;
string System::tm_zone;
bool   System::ms_initialized = false;
mutex  System::ms_mutex;

String System::locate_install_dir(const char *argv0) {
  lock_guard<mutex> lock(ms_mutex);
  return _locate_install_dir(argv0);
}

String System::_locate_install_dir(const char *argv0) {

  if (!install_dir.empty())
    return install_dir;

  exe_name = Path(argv0).filename().generic_string();

  Path exepath(proc_info().exe);

  // Detect install_dir/bin/exe_name: assumed install layout
  if (exepath.parent_path().filename() == "bin")
    install_dir = exepath.parent_path().parent_path().string();
  else
    install_dir = exepath.parent_path().string();

  return install_dir;
}

void System::_init(const String &install_directory) {

  if (install_directory.empty()) {
    install_dir = _locate_install_dir(proc_info().exe.c_str());
  }
  else {
    // set installation directory
    install_dir = install_directory;
    while (boost::ends_with(install_dir, "/"))
    install_dir = install_dir.substr(0, install_dir.length()-1);
  }

  if (exe_name.empty())
    exe_name = Path(proc_info().args[0]).filename().generic_string();

  // initialize logging system
  Logger::initialize(exe_name);
}

void System::initialize_tm(struct tm *tmval) {
  memset(tmval, 0, sizeof(struct tm));
  tmval->tm_isdst = -1;
#if !defined(__sun__)
  tmval->tm_gmtoff = System::tm_gmtoff;
  tmval->tm_zone = (char *)System::tm_zone.c_str();
#endif
}

int32_t System::get_processor_count() {
  return cpu_info().total_cores;
}

namespace {
  int32_t drive_count = 0;
}

int32_t System::get_drive_count() {
  if (drive_count > 0)
    return drive_count;

#if defined(__linux__)
  String device_regex = "sd[a-z][a-z]?|hd[a-z][a-z]?";
#elif defined(__APPLE__)
  String device_regex = "disk[0-9][0-9]?";
#else
  ImplementMe;
#endif

  vector<struct dirent> listing;

  FileUtils::readdir("/dev", device_regex, listing);

  drive_count = listing.size();

  return drive_count;
}

int32_t System::get_pid() {
  return proc_info().pid;
}
