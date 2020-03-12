#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#

set(MALLOC_FLAGS )
if (USE_GLIBC_MALLOC OR (
              CMAKE_SYSTEM_PROCESSOR STREQUAL "i386" OR
              CMAKE_SYSTEM_PROCESSOR STREQUAL "i586" OR
              CMAKE_SYSTEM_PROCESSOR STREQUAL "i686"))
  set(MALLOC_LIBRARIES_SHARED )
  set(MALLOC_LIBRARIES_STATIC )


elseif (USE_JEMALLOC)
  SET_DEPS(NAME "JEMALLOC" REQUIRED FALSE LIB_PATHS "" INC_PATHS "" STATIC libjemalloc.a SHARED jemalloc INCLUDE jemalloc/jemalloc.h)
  if (JEMALLOC_FOUND)
    set(MALLOC_LIBRARIES_SHARED ${JEMALLOC_LIBRARIES_SHARED})
    set(MALLOC_LIBRARIES_STATIC ${JEMALLOC_LIBRARIES_STATIC})
    set(MALLOC_INCLUDE_PATHS    ${JEMALLOC_INCLUDE_PATHS})
  else ()
    message(FATAL_ERROR "Unable to use jemalloc: library not found")
  endif ()


elseif (USE_HOARD)
  SET_DEPS(NAME "HOARD" REQUIRED FALSE LIB_PATHS "" INC_PATHS "" STATIC libhoard.a SHARED hoard INCLUDE hoard.h)
  if (Hoard_FOUND)
    set(MALLOC_LIBRARIES_SHARED ${HOARD_LIBRARIES_SHARED})
    set(MALLOC_LIBRARIES_STATIC ${HOARD_LIBRARIES_STATIC})
    set(MALLOC_INCLUDE_PATHS    ${HOARD_INCLUDE_PATHS})
  else ()
    message(FATAL_ERROR "Unable to use hoard malloc: library not found")
  endif ()


else()  # TCMALLOC_MINIMAL default if found
  
  set(Tcmalloc_header_NAMES gperftools/tcmalloc.h )
  if (USE_TCMALLOC)
    set(Tcmalloc_NAMES tcmalloc unwind)
    set(Tcmalloc_static_NAMES libtcmalloc.a libunwind.a)
  else () 
    set(Tcmalloc_NAMES tcmalloc_minimal)
    set(Tcmalloc_static_NAMES libtcmalloc_minimal.a)
  endif ()

  SET_DEPS(NAME "TCMALLOC" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC ${Tcmalloc_static_NAMES} SHARED ${Tcmalloc_NAMES} INCLUDE ${Tcmalloc_header_NAMES})  
  if (TCMALLOC_FOUND)
    if (TCMALLOC_LIBRARIES_SHARED MATCHES "tcmalloc_minimal")
      SET (MALLOC_FLAGS ${MALLOC_FLAGS} -DTCMALLOC_MINIMAL)
    else ()
      SET (MALLOC_FLAGS ${MALLOC_FLAGS} -DTCMALLOC)
    endif ()

    set(MALLOC_FLAGS ${MALLOC_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free)
    set(MALLOC_LIBRARIES_SHARED ${TCMALLOC_LIBRARIES_SHARED})
    set(MALLOC_LIBRARIES_STATIC ${TCMALLOC_LIBRARIES_STATIC})
    set(MALLOC_INCLUDE_PATHS    ${TCMALLOC_INCLUDE_PATHS})
  endif ()
  
endif ()



if(MALLOC_LIBRARIES_SHARED)
  message(STATUS "Using MALLOC: ${MALLOC_LIBRARIES_SHARED} ${MALLOC_LIBRARIES_STATIC} ")
  # INSTALL_LIBS(lib ${MALLOC_LIBRARIES_SHARED})
else ()
  message(STATUS "Using MALLOC: GLIBC_MALLOC ")
endif ()

