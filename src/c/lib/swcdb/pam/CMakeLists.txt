#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


set(CORE_LIBS_STATIC_FLAGS "")

# pam_swcdb_max_retries.so
ADD_LIB_TARGET(
  NAME    pam_swcdb_max_retries
  SRCS    pam_swcdb_max_retries.c
          ${SWC_VERSION_SRC}
  TARGETS ${SWC_LIB_THRIFT_C_TARGET}
  SHARED  ${SWC_LIB_THRIFT_C_LIBRARIES_SHARED}
          ${PAM_LIBRARIES_SHARED}
  ONLY_SHARED TRUE
  # ONLY_STATIC_SHARED TRUE
)
