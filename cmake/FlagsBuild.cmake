#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>




LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif ()
SET(FIND_LIBRARY_USE_LIB64_PATHS ON)



# C flags
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_LARGEFILE_SOURCE -m64 -D_FILE_OFFSET_BITS=64")


# C++ flags
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LARGEFILE_SOURCE -m64 -D_FILE_OFFSET_BITS=64")



if(SWC_ENABLE_SANITIZER STREQUAL "address")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=pointer-subtract -fsanitize=pointer-compare -fsanitize-address-use-after-scope")
elseif(SWC_ENABLE_SANITIZER STREQUAL "thread")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()


if(SWC_IMPL_COMPARATORS_BASIC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWC_IMPL_COMPARATORS_BASIC")
endif()



if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET (SWC_MALLOC_NOT_INSISTENT ON)
endif()
  
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
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdevirtualize-at-ltrans")
        SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-linker-plugin -ffat-lto-objects")

        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -floop-interchange")
        SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -floop-interchange")
      endif()
    endif ()

  elseif(O_LEVEL MATCHES "[3-7]")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-loop-ivcanon -fivopts")
      # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodulo-sched -fmodulo-sched-allow-regmoves")
      # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fgcse-sm -fgcse-las")
      # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flive-range-shrinkage")

      # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops")
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
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdevirtualize-at-ltrans")
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

# SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpack-struct=1 -fshort-enums")
