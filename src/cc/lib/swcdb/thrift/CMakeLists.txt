#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



if(THRIFT_CPP_GEN_PATH)

# libswcdb_thrift
ADD_LIB_TARGET(
  NAME    swcdb_thrift
  SRCS    ${THRIFT_CPP_GEN_PATH}/gen-cpp/Service_types.cpp
          ${THRIFT_CPP_GEN_PATH}/gen-cpp/Service.cpp
          # Converters.cc
            # Client.cc
          ${SWC_VERSION_SRC}
  SHARED  ${THRIFT_CPP_LIBRARIES_SHARED}
  STATIC  ${THRIFT_CPP_LIBRARIES_STATIC}
  FLAGS   -Wno-error=suggest-override
)

endif()
