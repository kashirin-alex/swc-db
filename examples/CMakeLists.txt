#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>



SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/="
)


include_directories(
  ../src/cc/include
  ../src/cc/lib
)




# example - geospatial_analysis-xyz_and_properties
ADD_EXEC_TARGET(
  NAME    geospatial_analysis-xyz_and_properties
  SRCS    geospatial_analysis-xyz_and_properties.cc
  TARGETS swcdb
)
