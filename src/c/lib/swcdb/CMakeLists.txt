#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#


if(THRIFT_C_GEN_PATH)

add_subdirectory(thrift)


if(NOT WITHOUT_PAM)
  add_subdirectory(pam)
endif(NOT WITHOUT_PAM)


endif(THRIFT_C_GEN_PATH)