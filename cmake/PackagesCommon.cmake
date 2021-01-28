#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>

SET_DEPS(NAME "ASIO"  REQUIRED TRUE LIB_PATHS "" INC_PATHS ${ASIO_INCLUDE_PATH} STATIC  SHARED  INCLUDE asio.hpp)

SET_DEPS(NAME "RE2" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libre2.a SHARED re2 INCLUDE re2/re2.h INSTALL TRUE)


SET_DEPS(NAME "ZLIB"  REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libz.a SHARED z INCLUDE zlib.h INSTALL TRUE)

SET_DEPS(NAME "SNAPPY" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libsnappy.a SHARED snappy INCLUDE snappy.h INSTALL TRUE)

SET_DEPS(NAME "ZSTD"  REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libzstd.a SHARED zstd INCLUDE zstd.h INSTALL TRUE)


if(SWC_DEFAULT_ENCODER)
  if(NOT SWC_DEFAULT_ENCODER STREQUAL ZSTD AND 
     NOT SWC_DEFAULT_ENCODER STREQUAL ZLIB AND 
     NOT SWC_DEFAULT_ENCODER STREQUAL SNAPPY AND 
     NOT SWC_DEFAULT_ENCODER STREQUAL PLAIN )
    message(FATAL_ERROR 
      "Requested SWC_DEFAULT_ENCODER=${SWC_DEFAULT_ENCODER} is not available, ENCODER options: PLAIN|ZSTD|SNAPPY|ZLIB")
  endif()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWC_DEFAULT_ENCODER=${SWC_DEFAULT_ENCODER}")
endif()


SET_DEPS(
  NAME      "SSL" 
  REQUIRED  TRUE 
  LIB_PATHS /usr/local/ssl/lib
  INC_PATHS /usr/local/ssl/include
  STATIC    libssl.a libcrypto.a 
  SHARED    ssl crypto
  INCLUDE   openssl/ssl.h openssl/crypto.h
  INSTALL   TRUE
)


if(NOT USE_GNU_READLINE)
  SET_DEPS(
    NAME      "EDITLINE" 
    REQUIRED  FALSE 
    LIB_PATHS "" INC_PATHS "" 
    STATIC    libeditline.a
    SHARED    editline 
    INCLUDE   editline.h
    INSTALL   TRUE
  )
endif()
if(NOT EDITLINE_FOUND)
  SET_DEPS(
    NAME      "EDITLINE" 
    REQUIRED  TRUE 
    LIB_PATHS "" INC_PATHS "" 
    STATIC    libreadline.a 
    SHARED    readline 
    INCLUDE   readline/readline.h readline/history.h
    INSTALL   TRUE
  )
endif()




find_package(Languages REQUIRED)


find_package(FileSystems REQUIRED)


find_package(Thrift REQUIRED)








#SET_DEPS(NAME "ATOMIC"  REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libatomic.a SHARED atomic INCLUDE )

#SET_DEPS(NAME "BZIP2" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libbz2.a SHARED bz2 INCLUDE bzlib.h)

#SET_DEPS(NAME "LZMA"  REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC liblzma.a SHARED lzma INCLUDE lzma.h)

#SET_DEPS(NAME "LZ4" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC liblz4.a SHARED lz4 INCLUDE lz4.h)


#SET_DEPS(NAME "EXPAT" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libexpat.a SHARED expat INCLUDE expat.h)

#SET_DEPS(NAME "SSP" REQUIRED TRUE LIB_PATHS "" INC_PATHS "/usr/local/lib/gcc/x86_64-ubuntu-linux-gnu/8.3.0/include" STATIC libssp.a SHARED ssp INCLUDE ssp/ssp.h)

#SET_DEPS(NAME "NCURSESW" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libncursesw.a SHARED ncursesw INCLUDE ncurses.h)

# find_package(Libssh REQUIRED)
