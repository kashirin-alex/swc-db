#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>


set(CORE_LIBS_STATIC_FLAGS "")

# pam_swcdb_max_retries.so
ADD_LIB_TARGET(
  NAME    pam_swcdb_max_retries
  SRCS    pam_swcdb_max_retries.c
  TARGETS swcdb_thrift_c 
  SHARED  ${PAM_LIBRARIES_SHARED}
  ONLY_SHARED TRUE
  # ONLY_STATIC_SHARED TRUE
)
