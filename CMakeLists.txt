#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#


cmake_minimum_required(VERSION 3.13)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif ()


project(SWCDB)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_MICRO "1")
set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}")
set(VERSION_PATCH ) 
set(VERSION_MISC_SUFFIX  "")

if(VERSION_PATCH)
  set(VERSION_STRING "${VERSION_STRING}.${VERSION_PATCH}")
endif()
if(VERSION_MISC_SUFFIX)
  set(VERSION_STRING "${VERSION_STRING}.${VERSION_MISC_SUFFIX}")
endif()

set(SWC_VERSION ${VERSION_STRING})

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif ()


# C++ flags
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")
# SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U__STRICT_ANSI__")
# SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LARGEFILE_SOURCE -m64 -D_FILE_OFFSET_BITS=64")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
 -ffile-prefix-map=${CMAKE_SOURCE_DIR}/src/cc/include/=\
 -ffile-prefix-map=${CMAKE_SOURCE_DIR}/src/cc/lib/=")

# -----------------  OPTIMIZATIONS and BUILD TYPE

# Something nice for GUI like ccmake
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Options: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

message(STATUS "Optimization Level: " ${O_LEVEL})
if(NOT O_LEVEL AND NOT O_LEVEL STREQUAL "0")
  SET (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")

else()
  SET (CMAKE_CXX_FLAGS_RELEASE        "-DNDEBUG")
  SET (CMAKE_CXX_FLAGS_DEBUG          "")

  if(O_LEVEL STREQUAL "0")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")

  elseif(O_LEVEL MATCHES "[1-2]")
      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    
    if (O_LEVEL STREQUAL "2")
  	  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fuse-linker-plugin -ffat-lto-objects")
      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -floop-interchange")
    endif ()

  elseif(O_LEVEL MATCHES "[3-7]")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    
    if(O_LEVEL MATCHES "[4-7]")
  	  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fuse-linker-plugin -ffat-lto-objects")
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



set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(LOOKUP_LIB_PATHS 	
	/opt/local/lib 
	/usr/local/lib  
	/usr/lib  
	/lib 
)

set(LOOKUP_INCLUDE_PATHS 
	/opt/local/include 
	/usr/local/include 
	/usr/include
)			

# Utilities on Building
include(Utils)

# Packages Core
include(PackagesCore)

# Packages Common
include(PackagesCommon)

# Packages Common
include(Doc)


add_subdirectory("src")
