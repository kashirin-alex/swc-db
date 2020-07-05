#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>



exec_program(env ARGS thrift -version OUTPUT_VARIABLE THRIFT_VERSION
             RETURN_VALUE Thrift_RETURN)

if(THRIFT_VERSION MATCHES "^Thrift version") 
  set(THRIFT_COMPILER_FOUND TRUE)
  message("-- Found THRIFT compiler")
  message("       compiler: ${THRIFT_VERSION}")
  string(REPLACE "\n" " " THRIFT_VERSION ${THRIFT_VERSION})
  string(REPLACE " " ";" THRIFT_VERSION ${THRIFT_VERSION})
  list(GET THRIFT_VERSION -1 THRIFT_VERSION)

else()
  set(THRIFT_COMPILER_FOUND FALSE)
  message("-- Not Found THRIFT compiler: thrift")
  message("       Using existing gen. sources")
  set(THRIFT_VERSION )
endif()
   

SET_DEPS(
  NAME      "THRIFT_CPP" 
  REQUIRED  FALSE 
  LIB_PATHS /usr/local/lib
  INC_PATHS /usr/local/include
            /usr/include
            /opt/local/include
  STATIC    libthrift.a libthriftnb.a  libthriftz.a 
  SHARED    thrift thriftnb thriftz
  INCLUDE   thrift/Thrift.h
)

if(THRIFT_CPP_FOUND)

  if(NOT THRIFT_VERSION)
    list(GET THRIFT_CPP_LIBRARIES_SHARED 0 libthrift)
    exec_program(env ARGS strings ${libthrift} | grep 'User-Agent: Thrift' -A1 | tr '\n' ' ' | cut -f3 -d' '
      OUTPUT_VARIABLE THRIFT_VERSION
      RETURN_VALUE Thrift_RETURN)
  endif()
  
  if(THRIFT_VERSION)
    SET_DEPS(NAME "EVENT" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libevent.a SHARED event INCLUDE event.h)
  
    set(THRIFT_CPP_LIBRARIES_SHARED ${THRIFT_CPP_LIBRARIES_SHARED} ${EVENT_LIBRARIES_SHARED})
    set(THRIFT_CPP_LIBRARIES_STATIC ${THRIFT_CPP_LIBRARIES_STATIC} ${EVENT_LIBRARIES_STATIC}
                                    ${ZLIB_LIBRARIES_STATIC}  ${SSL_LIBRARIES_STATIC})
  endif()

endif(THRIFT_CPP_FOUND)


if(NOT WITHOUT_THRIFT_C)

  SET_DEPS(
    NAME      "THRIFT_C" 
    REQUIRED  FALSE 
    LIB_PATHS /usr/local/lib
    INC_PATHS /usr/local/include
              /usr/include
              /opt/local/include
    STATIC    libthrift_c_glib.a
    SHARED    thrift_c_glib
    INCLUDE   thrift/c_glib/thrift.h
  )

  if(THRIFT_C_FOUND)

    SET_DEPS(
      NAME      "GLIB"
      REQUIRED  TRUE 
      LIB_PATHS ""
      INC_PATHS ${GLIB_INCLUDE_PATH}
      STATIC    libglib-2.0.a libgobject-2.0.a libffi.a libpcre.a
      SHARED    glib-2.0 gobject-2.0
      INCLUDE   glib.h glib-object.h glibconfig.h
    )

    set(THRIFT_C_LIBRARIES_SHARED ${THRIFT_C_LIBRARIES_SHARED} ${GLIB_LIBRARIES_SHARED})
    set(THRIFT_C_LIBRARIES_STATIC ${THRIFT_C_LIBRARIES_STATIC} ${GLIB_LIBRARIES_STATIC}
                                  ${ZLIB_LIBRARIES_STATIC}  ${SSL_LIBRARIES_STATIC})

    if(NOT WITHOUT_PAM)
      SET_DEPS(
        NAME      "PAM"
        REQUIRED  FALSE 
        LIB_PATHS /lib/x86_64-linux-gnu/
        SHARED    libpam.so.0 libpam_misc.so.0
        INCLUDE   security/pam_appl.h security/pam_misc.h
      )
    endif(NOT WITHOUT_PAM)

  endif(THRIFT_C_FOUND)

endif(NOT WITHOUT_THRIFT_C)