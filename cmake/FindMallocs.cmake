#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>

set(MALLOC_FLAGS )

if(USE_SWC_MALLOC)
  set(MALLOC_LIBRARIES_SHARED )
  set(MALLOC_LIBRARIES_STATIC )
  set(flags "-DSWC_MALLOC")# -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")

  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}")
  SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flags}")

elseif (USE_GLIBC_MALLOC OR (
              CMAKE_SYSTEM_PROCESSOR STREQUAL "i386" OR
              CMAKE_SYSTEM_PROCESSOR STREQUAL "i586" OR
              CMAKE_SYSTEM_PROCESSOR STREQUAL "i686"))
  set(MALLOC_LIBRARIES_SHARED )
  set(MALLOC_LIBRARIES_STATIC )


elseif (USE_JEMALLOC)
  SET_DEPS(NAME "JEMALLOC" REQUIRED FALSE LIB_PATHS "" INC_PATHS "" STATIC libjemalloc.a SHARED jemalloc INCLUDE jemalloc/jemalloc.h INSTALL TRUE)
  if (JEMALLOC_FOUND)
    set(MALLOC_LIBRARIES_SHARED ${JEMALLOC_LIBRARIES_SHARED})
    set(MALLOC_LIBRARIES_STATIC ${JEMALLOC_LIBRARIES_STATIC})
    set(MALLOC_INCLUDE_PATHS    ${JEMALLOC_INCLUDE_PATHS})
  else ()
    message(FATAL_ERROR "Unable to use jemalloc: library not found")
  endif ()


elseif (USE_HOARD)
  SET_DEPS(NAME "HOARD" REQUIRED FALSE LIB_PATHS "" INC_PATHS "" STATIC libhoard.a SHARED hoard INCLUDE hoard.h INSTALL TRUE)
  if (Hoard_FOUND)
    set(MALLOC_LIBRARIES_SHARED ${HOARD_LIBRARIES_SHARED})
    set(MALLOC_LIBRARIES_STATIC ${HOARD_LIBRARIES_STATIC})
    set(MALLOC_INCLUDE_PATHS    ${HOARD_INCLUDE_PATHS})
  else ()
    message(FATAL_ERROR "Unable to use hoard malloc: library not found")
  endif ()


elseif (USE_MIMALLOC)
  SET_DEPS(NAME "MIMALLOC" REQUIRED FALSE 
    LIB_PATHS "/usr/local/lib/mimalloc-1.6/" 
    INC_PATHS "/usr/local/lib/mimalloc-1.6/include" 
    STATIC libmimalloc.a SHARED mimalloc INCLUDE mimalloc.h INSTALL TRUE)
  if (MIMALLOC_FOUND)
    set(MALLOC_LIBRARIES_SHARED ${MIMALLOC_LIBRARIES_SHARED})
    set(MALLOC_LIBRARIES_STATIC ${MIMALLOC_LIBRARIES_STATIC})
    set(MALLOC_INCLUDE_PATHS    ${MIMALLOC_INCLUDE_PATHS})
    set(MALLOC_FLAGS -DMIMALLOC)
  else ()
    message(FATAL_ERROR "Unable to use MIMALLOC: library not found")
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

  SET_DEPS(
    NAME "TCMALLOC" 
    REQUIRED FALSE 
    LIB_PATHS "" 
    INC_PATHS "" 
    STATIC ${Tcmalloc_static_NAMES} 
    SHARED ${Tcmalloc_NAMES} 
    INCLUDE ${Tcmalloc_header_NAMES}
    INSTALL TRUE
  )  
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
elseif(USE_SWC_MALLOC)
  message(STATUS "Using MALLOC: SWC-DB Implementation ")
else ()
  message(STATUS "Using MALLOC: GLIBC_MALLOC ")
endif ()

