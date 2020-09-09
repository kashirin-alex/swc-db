#!/usr/bin/env bash
#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>
#
# Copyright (C) 2007-2016 Hypertable, Inc.
#
# This file is part of Hypertable.
#
# Hypertable is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or any later version.
#
# Hypertable is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hypertable. If not, see <http://www.gnu.org/licenses/>
#

# Figure out soname of a shared lib. Handles platforms with GNU binutils, e.g.,
# Linux etc; Solaris with elfdump and Mac OS X with otool
#
lib=$1

if o=`(objdump -p $lib | grep SONAME)  2>/dev/null` ||
   o=`(elfdump -d $lib | grep SONAME) 2>/dev/null`
then
  echo $o | sed 's/.* \([^ ][^ ]*\)/\1/' | cut -f 1 -d' '
elif o=`otool -D $lib 2>/dev/null`; then
  echo $o | tail -1 | sed 's/.*\/\([^\/][^\/]*\)/\1/' | cut -f 1 -d' ' | sed 's/:$//'
else
  o=`ld -t $lib 2> /dev/null | tail -n 1`;
  if [ -z ${o} ]; then
    o=$lib
  fi;
  echo $o | awk 'BEGIN {FS="/"} { print $NF; }' | cut -f 1 -d' '
fi
