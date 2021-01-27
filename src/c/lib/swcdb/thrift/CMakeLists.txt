#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



# libswcdb_thrift_c
ADD_LIB_TARGET(
  NAME    swcdb_thrift_c
  SRCS    ${THRIFT_C_GEN_PATH}/gen-c_glib/swcdb_thrift_service_types.c
          ${THRIFT_C_GEN_PATH}/gen-c_glib/swcdb_thrift_service.c
          ${SWC_VERSION_SRC}
  SHARED  ${THRIFT_C_LIBRARIES_SHARED}
  STATIC  ${THRIFT_C_LIBRARIES_STATIC}
  #ONLY_SHARED TRUE
)
