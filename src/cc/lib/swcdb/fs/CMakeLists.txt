#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


########### FILESYSTEM CORE/BASE ###########

set(swcdb_fscore_sources
	Dirent.cc
	SmartFd.cc
	Settings.cc
	FileSystem.cc
	Interface.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_fs
	SRCS	${swcdb_fscore_sources}
	TARGETS	swcdb_core swcdb_core_config
)



########### LOCAL FILESYSTEM ###########

set(swcdb_fsLocal_sources
	Local/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_fs_local
	SRCS	${swcdb_fsLocal_sources}
	TARGETS	swcdb_fs swcdb_core swcdb_core_config
)



########### CEPH FILESYSTEM ###########

#set(swcdb_fsCeph_sources
# Ceph/FileSystem.cc
#)
#ADD_LIB_TARGET(
#	NAME 	swcdb_fs_ceph
#	SRCS 	${swcdb_fsCeph_sources}
#	TARGETS swcdb_fs swcdb_core swcdb_core_config
#)



########### HADOOP FILESYSTEM ###########

set(swcdb_fsHadoop_sources
	Hadoop/FileSystem.cc
)

ADD_LIB_TARGET(
	NAME	swcdb_fs_hadoop
	SRCS	${swcdb_fsHadoop_sources}
	TARGETS	swcdb_fs
	SHARED	${HADOOP_LIBRARIES_SHARED} ${PROTOBUF_LIBRARIES_SHARED}
	STATIC	${HADOOP_LIBRARIES_STATIC} ${PROTOBUF_LIBRARIES_STATIC}
)


set(SWC_LIB_FS_SHARED )
set(SWC_LIB_FS_STATIC )
if(BUILTIN_FS_HADOOP)
	set(SWC_LIB_FS_SHARED ${SWC_LIB_FS_SHARED} ${HADOOP_LIBRARIES_SHARED} ${PROTOBUF_LIBRARIES_SHARED})
	set(SWC_LIB_FS_STATIC ${SWC_LIB_FS_STATIC} ${HADOOP_LIBRARIES_STATIC} ${PROTOBUF_LIBRARIES_STATIC})
endif()

if(SWC_LIB_FS_SHARED)
	set_property(GLOBAL PROPERTY SWC_LIB_FS_SHARED ${SWC_LIB_FS_SHARED})
endif()
if(SWC_LIB_FS_STATIC)
	set_property(GLOBAL PROPERTY SWC_LIB_FS_STATIC ${SWC_LIB_FS_STATIC})
endif()

#########################


########### HADOOP_JVM FILESYSTEM ###########

set(swcdb_fsHadoop_jvm_sources
	HadoopJVM/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_fs_hadoop_jvm
	SRCS	${swcdb_fsHadoop_jvm_sources}
	TARGETS	swcdb_fs
	SHARED	${HADOOP_JVM_LIBRARIES_SHARED} ${JAVA_LIBRARIES_SHARED} 
	STATIC	${HADOOP_JVM_LIBRARIES_SHARED} ${JAVA_LIBRARIES_SHARED} 
)



########### FSBROKER FILESYSTEM ###########

set(swcdb_fsBroker_sources
	Broker/FileSystem.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_fs_broker
	SRCS	${swcdb_fsBroker_sources}
	TARGETS	swcdb_fs swcdb_core swcdb_core_config swcdb_core_comm
)



ADD_TEST_TARGET(
	NAME	fs_client
	SRCS	tests/test_fs_client.cc
	TARGETS swcdb_fs swcdb_core swcdb_core_config swcdb_core_comm ${BUILTIN_FS_TARGETS}
	ARGS	"--config=/root/swc-db/conf/swc.cfg"
)