#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#



include_directories(
  cc
  ${CMAKE_CURRENT_BINARY_DIR}/cc
)

# add_subdirectory(cc/hypertable)

set(BUILTIN_FS_TARGETS "")
if(BUILTIN_FS)

	if (BUILTIN_FS STREQUAL "all")
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILTIN_FS_ALL")
	endif()

	if (BUILTIN_FS STREQUAL "all" OR BUILTIN_FS STREQUAL "local")
		set(BUILTIN_FS_TARGETS ${BUILTIN_FS_TARGETS} swcdb_fs_local)
		if (NOT BUILTIN_FS STREQUAL "all")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILTIN_FS_LOCAL")
		endif()
	endif()

	if (BUILTIN_FS STREQUAL "all" OR BUILTIN_FS STREQUAL "ceph")
		set(BUILTIN_FS_TARGETS ${BUILTIN_FS_TARGETS} swcdb_fs_ceph)
		if (NOT BUILTIN_FS STREQUAL "all")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILTIN_FS_CEPH")
		endif()
	endif()
	
endif()

add_subdirectory(cc/swcdb/lib)
add_subdirectory(cc/swcdb/app)




