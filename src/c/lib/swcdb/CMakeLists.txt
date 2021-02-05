#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


if(THRIFT_C_GEN_PATH)

add_subdirectory(thrift)


if(NOT WITHOUT_PAM AND (NOT SWC_BUILD_PKG OR SWC_BUILD_PKG MATCHES "^lib-pam"))
  add_subdirectory(pam)
endif()


endif(THRIFT_C_GEN_PATH)