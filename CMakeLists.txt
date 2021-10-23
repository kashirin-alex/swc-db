#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


cmake_minimum_required(VERSION 3.13)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif ()


project(SWCDB)
# 0.5.5
set(VERSION_MAJOR "0")
set(VERSION_MINOR "5")
set(VERSION_MICRO "5")
set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}")
set(VERSION_PATCH )
set(VERSION_MISC_SUFFIX  "")

if(VERSION_PATCH)
  set(VERSION_STRING "${VERSION_STRING}.${VERSION_PATCH}")
endif()
if(VERSION_MISC_SUFFIX)
  set(VERSION_STRING "${VERSION_STRING}.${VERSION_MISC_SUFFIX}")
endif()

set(SWCDB_VERSION ${VERSION_STRING})
set(SWCDB_LICENSE_URL
    "https://github.com/kashirin-alex/swc-db/blob/v${VERSION_STRING}/LICENSE")

set(SWCDB_COPYRIGHT
  "# SWC-DB (c) Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>\\n"
  "# SWC-DB is free software: you can redistribute it and/or modify\\n"
  "# it under the terms of the GNU General Public License as published by\\n"
  "# the Free Software Foundation, version GPLv3.\\n"
  "# \\n"
  "# SWC-DB is distributed in the hope that it will be useful,\\n"
  "# but WITHOUT ANY WARRANTY; without even the implied warranty of\\n"
  "# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\\n"
  "# GNU General Public License for more details.\\n"
  "# \\n"
  "# You should have received a copy of the GNU General Public License\\n"
  "# along with this program.\\n"
  "# If not, see <${SWCDB_LICENSE_URL}>.\\n"
)
string(CONCAT SWCDB_COPYRIGHT ${SWCDB_COPYRIGHT})


set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
  ${CMAKE_SOURCE_DIR}/docmake
)



# Compiler Build Flags
include(FlagsBuild)

# Compiler Warning Flags
include(FlagsWarnings)

# Compiler Profile Flags
include(FlagsProfile)



if(NOT SWC_INSTALL_DEP_LIBS)
  set(UTILS_NO_INSTALL_DEP_LIBS ON)
endif()

set(LOOKUP_LIB_PATHS
  ${LOOKUP_LIB_PATHS}
  /opt/local/lib
  /usr/local/lib
  /usr/lib
  /lib
)

set(LOOKUP_INCLUDE_PATHS
  ${LOOKUP_INCLUDE_PATHS}
  /opt/local/include
  /usr/local/include
  /usr/local/lib
  /usr/include
)

if(NOT SWC_BUILD_PKG OR NOT SWC_BUILD_PKG MATCHES "^(env|doc)")

# Utilities on Building
include(Utils)

# Packages Core
include(PackagesCore)

# Packages Common
include(PackagesCommon)

endif()

# Packages Documentations
if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "doc")
  include(Doc)
endif()


if(NOT SWC_BUILD_PKG OR NOT SWC_BUILD_PKG MATCHES "^(env|doc|pam|lib-thrift)")
  find_package(SWCDB)
endif()


add_subdirectory("src")


if(NOT SWC_BUILD_PKG)
  add_subdirectory("tests")
  add_subdirectory("examples")
endif()


install(
  FILES     README.md
            LICENSE
            NOTICE.md
            COPYING.md
            CHANGELOG.md
  DESTINATION share/swcdb/${SWC_BUILD_PKG}
)
# CONTRIBUTING.md CREDITS.md SPONSORS.md


if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "doc")
  install(DIRECTORY docs DESTINATION share/swcdb)
endif()
