#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>

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

  if(SWC_ENABLE_SANITIZER STREQUAL "address")
    SET(CORE_LIBS_STATIC_FLAGS ${CORE_LIBS_STATIC_FLAGS} "-static-libasan -static-liblsan -static-libubsan")
  elseif(SWC_ENABLE_SANITIZER STREQUAL "thread")
    SET(CORE_LIBS_STATIC_FLAGS ${CORE_LIBS_STATIC_FLAGS} "-static-libtsan")
  endif()

else()
  if(SWC_ENABLE_SANITIZER STREQUAL "address")
    SET(CORE_LIBS ${CORE_LIBS} rt asan lsan ubsan)
  elseif(SWC_ENABLE_SANITIZER STREQUAL "thread")
    SET(CORE_LIBS ${CORE_LIBS} rt tsan)
  endif()

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



#message(STATUS ${CMAKE_LIBCXX_LIBRARIES})

if(SWC_INSTALL_DEP_LIBS)
  SET_DEPS(
    NAME      "CXX"
    REQUIRED  TRUE
    LIB_PATHS "" INC_PATHS ""
    STATIC    libstdc++.a
    SHARED    stdc++ gcc_s
    INCLUDE
    INSTALL   TRUE
  )
endif()

find_package(Mallocs)
# find_package(Boost REQUIRED)


