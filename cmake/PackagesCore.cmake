#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#

SET(CORE_LIBS )

set(CORE_LIBS_SHARED )
set(CORE_LIBS_STATIC )
set(CORE_LIBS_STATIC_FLAGS )


find_package(Threads REQUIRED)
message(STATUS "Using thread library: ${CMAKE_THREAD_LIBS_INIT}")
string(SUBSTRING ${CMAKE_THREAD_LIBS_INIT} 2 -1 thread_LIB_NAME)


if(BUILD_LINKING_CORE STREQUAL "STATIC")

  SET(CORE_LIBS_STATIC_FLAGS ${CORE_LIBS_STATIC_FLAGS} "-s -static-libgcc -static-libstdc++") 
  # not possible -static-pie -static as long as dl is involved
  
endif()


  #SET(CMAKE_FIND_ROOT_PATH /usr/local /usr/local/glibc)
  #SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
  #SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  #SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

  # set(MATH_LIBRARIES m)
  # set(THREAD_LIBRARIES ${thread_LIB_NAME} c)
  #SET_DEPS(NAME "MATH" REQUIRED TRUE LIB_PATHS "/usr/local/glibc/lib" INC_PATHS "/usr/local/glibc/include" STATIC libm.a SHARED m INCLUDE math.h)
  #SET_DEPS(NAME "THREAD" REQUIRED TRUE LIB_PATHS "/usr/local/glibc/lib" INC_PATHS "/usr/local/glibc/include" STATIC lib${thread_LIB_NAME}.a SHARED ${thread_LIB_NAME} INCLUDE ${thread_LIB_NAME}.h)

SET(CORE_LIBS ${CORE_LIBS} dl pthread) # stdc++fs



find_package(Mallocs)
# find_package(Boost REQUIRED)


