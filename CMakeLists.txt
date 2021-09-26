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

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif ()
SET(FIND_LIBRARY_USE_LIB64_PATHS ON)



# C flags
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_LARGEFILE_SOURCE -m64 -D_FILE_OFFSET_BITS=64")
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror") # -Wextra
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat-security")


# C++ flags
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LARGEFILE_SOURCE -m64 -D_FILE_OFFSET_BITS=64")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASIO_STANDALONE -DASIO_NO_DEPRECATED") # -DASIO_USE_TS_EXECUTOR_AS_DEFAULT")
# SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpack-struct=1 -fshort-enums")

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wpedantic") # -Weffc++
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security -Wformat-nonliteral")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align -Wnon-virtual-dtor")

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wzero-as-null-pointer-constant -Wno-error=zero-as-null-pointer-constant")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast -Wno-error=old-style-cast")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnull-dereference -Wno-error=null-dereference")


if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-signedness")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnoexcept -Wno-error=noexcept")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override -Wno-error=suggest-override")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wuseless-cast -Wno-error=useless-cast")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-null-sentinel -Wno-error=strict-null-sentinel")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wno-error=duplicated-cond")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-branches -Wno-error=duplicated-branches")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op -Wno-error=logical-op")

  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow=compatible-local -Wshadow=local -Wshadow=global -Wshadow")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstack-usage=262144")

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wtrampolines")

  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=const -Wno-error=suggest-attribute=const")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=pure -Wno-error=suggest-attribute=pure")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=malloc -Wno-error=suggest-attribute=malloc")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=cold -Wno-error=suggest-attribute=cold")
  # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=noreturn -Wno-error=suggest-attribute=noreturn")

  # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Walloc-zero -Wno-error=alloc-zero")
  # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-final-types -Wno-error=suggest-final-types")

  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0.0)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wtsan")
  endif()

else()
  SET (SWC_MALLOC_NOT_INSISTENT ON)

endif()


if(SWC_ENABLE_SANITIZER STREQUAL "address")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=pointer-subtract -fsanitize=pointer-compare -fsanitize-address-use-after-scope")
elseif(SWC_ENABLE_SANITIZER STREQUAL "thread")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

if(SWC_IMPL_COMPARATORS_BASIC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWC_IMPL_COMPARATORS_BASIC")
endif()



##

if(SWC_MALLOC_NOT_INSISTENT)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWC_MALLOC_NOT_INSISTENT")
endif(SWC_MALLOC_NOT_INSISTENT)


set(SWC_DEFAULT_PATH_FLAGS )
if(SWC_PATH_ETC)
  set(SWC_DEFAULT_PATH_FLAGS ${SWC_DEFAULT_PATH_FLAGS} -DSWC_PATH_ETC="${SWC_PATH_ETC}")
endif()
if(SWC_PATH_LOG)
  set(SWC_DEFAULT_PATH_FLAGS ${SWC_DEFAULT_PATH_FLAGS} -DSWC_PATH_LOG="${SWC_PATH_LOG}")
endif()
if(SWC_PATH_RUN)
  set(SWC_DEFAULT_PATH_FLAGS ${SWC_DEFAULT_PATH_FLAGS} -DSWC_PATH_RUN="${SWC_PATH_RUN}")
endif()


if(NOT SWC_BUILD_PKG)
  set(SWC_BUILD_PKG "")
endif()

# -----------------  OPTIMIZATIONS and BUILD TYPE

# Something nice for GUI like ccmake
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Options: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

message(STATUS "Optimization Level: " ${O_LEVEL})
if(NOT O_LEVEL AND NOT O_LEVEL STREQUAL "0")
  SET (CMAKE_CXX_FLAGS_RELEASE      "-O3 -DNDEBUG")
  SET (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG")

else()
  SET (CMAKE_CXX_FLAGS_RELEASE      "-DNDEBUG")
  SET (CMAKE_CXX_FLAGS_DEBUG        "-ggdb3")
  SET (CMAKE_C_FLAGS_RELEASE        "-DNDEBUG")
  SET (CMAKE_C_FLAGS_DEBUG          "-ggdb3")

  if(O_LEVEL STREQUAL "0")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")

  elseif(O_LEVEL MATCHES "[1-2]")
      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
      SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

    if (O_LEVEL STREQUAL "2")

      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
      SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")
      if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-linker-plugin -ffat-lto-objects")
        SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-linker-plugin -ffat-lto-objects")

        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -floop-interchange")
        SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -floop-interchange")
      endif()
    endif ()

  elseif(O_LEVEL MATCHES "[3-7]")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdevirtualize-at-ltrans")

      # if (O_LEVEL STREQUAL "3")
      #    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fipa-pta")
      #    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fipa-pta")
      #  endif()
    endif()

    if(O_LEVEL MATCHES "[4-7]")

      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
      SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")
      if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-linker-plugin -ffat-lto-objects")
        SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-linker-plugin -ffat-lto-objects")
      endif()
      # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-early-inlining")
      # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsection-anchors")

      if(O_LEVEL MATCHES "[5-7]")
        set(BUILD_LINKING "STATIC")
        if(NOT TEST_LINKING)
          set(TEST_LINKING "DUAL")
        endif ()
      endif ()

      if(O_LEVEL MATCHES "[6-7]")
        set(BUILD_LINKING_CORE "STATIC")
      endif ()

      if(O_LEVEL MATCHES "[7-7]")
        set(LIBS_SHARED_LINKING "STATIC")
      endif ()
    endif ()

  endif ()

endif ()



set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
  ${CMAKE_SOURCE_DIR}/docmake
)

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
