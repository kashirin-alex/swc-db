#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

# set(_src_path ${CMAKE_CURRENT_SOURCE_DIR}/../../../source/pah)




# lib-swcdb_core,  #ifndef swc_core_common_FileName_h
set(swcdb_core_sources
	String.cc
	Error.cc
	Logger.cc
	Serializable.cc
	Version.cc
	FileUtils.cc
	fast_clock.cc
	Checksum.cc
	Random.cc
	Time.cc
)
configure_file(Version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.h)
ADD_LIB_TARGET(
	NAME 		swcdb_core
	SRCS 		${swcdb_core_sources}
	SHARED  ${BOOST_LIBRARIES_SHARED} 
					${RE2_LIBRARIES_SHARED} 
					${ZLIB_LIBRARIES_SHARED} ${LZMA_LIBRARIES_SHARED} ${BZIP2_LIBRARIES_SHARED}
	STATIC  ${BOOST_LIBRARIES_STATIC} 
					${RE2_LIBRARIES_STATIC} 
					${ZLIB_LIBRARIES_STATIC} ${LZMA_LIBRARIES_STATIC} ${BZIP2_LIBRARIES_STATIC}
)


# lib-swcdb_core_config, #ifndef swc_core_config_FileName_h
set(swcdb_core_config_sources
	config/Settings.cc
	config/Properties.cc
	config/PropertiesParser.cc
	config/Property.cc
	config/PropertyValueEnumExt.cc
	# config/System.cc
	# config/SystemInfo.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb_core_config
	SRCS 		${swcdb_core_config_sources}
	TARGETS swcdb_core
)



# lib-swcdb_core_comm, #ifndef swcdb_core_comm_FileName_h
set(swcdb_core_comm_sources
	comm/CommHeader.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb_core_comm
	SRCS 		${swcdb_core_comm_sources}
	TARGETS swcdb_core swcdb_core_config
)

