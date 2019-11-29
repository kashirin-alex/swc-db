#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

# set(_src_path ${CMAKE_CURRENT_SOURCE_DIR}/../../../source/pah)




# lib-swcdb_core,  #ifndef swc_core_common_FileName_h
set(swcdb_core_sources
	String.cc
	Error.cc
	Logger.cc
	Version.cc
	FileUtils.cc
	Random.cc
)
configure_file(Version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.h)
ADD_LIB_TARGET(
	NAME 		swcdb_core
	SRCS 		${swcdb_core_sources}
	SHARED  ${ATOMIC_LIBRARIES_SHARED} ${RE2_LIBRARIES_SHARED} 
					${ZLIB_LIBRARIES_SHARED} ${LZMA_LIBRARIES_SHARED} ${BZIP2_LIBRARIES_SHARED}
	STATIC  ${ATOMIC_LIBRARIES_STATIC} ${RE2_LIBRARIES_STATIC} 
					${ZLIB_LIBRARIES_STATIC} ${LZMA_LIBRARIES_STATIC} ${BZIP2_LIBRARIES_STATIC}
)


# lib-swcdb_core_config, #ifndef swc_core_config_FileName_h
set(swcdb_core_config_sources
	config/Settings.cc
	# config/System.cc
	# config/SystemInfo.cc
)

ADD_LIB_TARGET(
	NAME 		swcdb_core_config
	SRCS 		${swcdb_core_config_sources}
)

ADD_TEST_TARGET(
	NAME core-serialization
	SRCS tests/sertest.cc
	TARGETS swcdb_core 
)


ADD_TEST_EXEC(
	NAME properties_parser_test
	SRCS tests/properties_parser_test.cc
	TARGETS swcdb_core swcdb_core_config 
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/properties_parser_test.cfg ${CMAKE_CURRENT_BINARY_DIR}/properties_parser_test.cfg)
add_test(properties_parser_test ${TEST_DIFF}
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/properties_parser_test.golden
  ./properties_parser_test argOne --i16 1k --i32 64K --i64 1G --boo true --strs=1 -strs 2 -strs 3 --strs 4 -strs=5 --strs=6 --b=1 --is_true  -config=properties_parser_test.cfg
)
	
ADD_EXEC_TARGET(
	NAME      check_mutex
	SRCS      tests/mutex.cc
	TARGETS swcdb_core
)
