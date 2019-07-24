#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#




########### LOCAL FILESYSTEM ###########

set(swcdb_fsLocal_sources
Local/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb_fs_local
	SRCS 		${swcdb_fsLocal_sources}
	TARGETS swcdb_core_comm
)


########### CEPH FILESYSTEM ###########

set(swcdb_fsCeph_sources
Ceph/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb_fs_ceph
	SRCS 		${swcdb_fsCeph_sources}
	TARGETS swcdb_core_comm
)


########### HADOOP FILESYSTEM ###########

set(swcdb_fsHadoop_sources
Hadoop/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb_fs_hadoop
	SRCS 		${swcdb_fsHadoop_sources}
	TARGETS swcdb_core_comm
	SHARED 	${HADOOP_LIBRARIES_SHARED} ${JAVA_LIBRARIES_SHARED} 
	STATIC 	${HADOOP_LIBRARIES_SHARED} ${JAVA_LIBRARIES_SHARED} 
)
