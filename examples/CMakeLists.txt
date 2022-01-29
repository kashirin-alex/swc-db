#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
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
ADD_TEST_EXEC(
  NAME    geospatial_analysis-xyz_and_properties
  SRCS    geospatial_analysis-xyz_and_properties.cc
  TARGETS swcdb
)


# example - Criteo-1TB_Click_Logs_dataset
ADD_TEST_EXEC(
  NAME    Criteo-1TB_Click_Logs_dataset
  SRCS    Criteo-1TB_Click_Logs_dataset.cc
  TARGETS swcdb
)


# example - queue-example
ADD_TEST_EXEC(
  NAME    queue-example
  SRCS    queue-example.cc
  TARGETS swcdb
)