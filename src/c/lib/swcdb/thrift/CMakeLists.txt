#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#



# libswcdb_thrift_c
ADD_LIB_TARGET(
  NAME    swcdb_thrift_c
  SRCS    ${THRIFT_C_GEN_PATH}/gen-c_glib/swcdb_thrift_service_types.c
          ${THRIFT_C_GEN_PATH}/gen-c_glib/swcdb_thrift_service.c
  SHARED  ${THRIFT_C_LIBRARIES_SHARED}
  STATIC  ${THRIFT_C_LIBRARIES_STATIC}
)
