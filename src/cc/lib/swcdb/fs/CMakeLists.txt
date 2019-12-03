#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#




########### LOCAL FILESYSTEM ###########

set(swcdb_fsLocal_sources
Local/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_fs_local
	SRCS	${swcdb_fsLocal_sources}
	TARGETS	swcdb_core swcdb_core_config
)


########### CEPH FILESYSTEM ###########

#set(swcdb_fsCeph_sources
# Ceph/FileSystem.cc
#)
#ADD_LIB_TARGET(
#	NAME 	swcdb_fs_ceph
#	SRCS 	${swcdb_fsCeph_sources}
#	TARGETS swcdb_core swcdb_core_config
#)


########### HADOOP FILESYSTEM ###########

set(swcdb_fsHadoop_sources
Hadoop/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_fs_hadoop
	SRCS	${swcdb_fsHadoop_sources}
	TARGETS	swcdb_core swcdb_core_config
	SHARED	${HADOOP_LIBRARIES_SHARED} ${JAVA_LIBRARIES_SHARED} 
	STATIC	${HADOOP_LIBRARIES_SHARED} ${JAVA_LIBRARIES_SHARED} 
)


########### FSBROKER FILESYSTEM ###########

set(swcdb_fsBroker_sources
Broker/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_fs_broker
	SRCS	${swcdb_fsBroker_sources}
	TARGETS	swcdb_core swcdb_core_config
)



ADD_TEST_TARGET(
	NAME	fs_client
	SRCS	tests/test_fs_client.cc
	TARGETS swcdb_core swcdb_core_config swcdb_core_comm ${BUILTIN_FS_TARGETS}
	ARGS	"--config=/root/swc-db/conf/swc.cfg"
)