#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#





set(swcdb_fsLocal_sources
Local/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb_fs_local
	SRCS 		${swcdb_fsLocal_sources}
	TARGETS swcdb_core_comm
)

set(swcdb_fsCeph_sources
Ceph/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb_fs_ceph
	SRCS 		${swcdb_fsCeph_sources}
	TARGETS swcdb_core_comm
)
