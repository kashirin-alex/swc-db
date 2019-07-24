#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

SET_DEPS(
  NAME "JAVA" 
  
	LIB_PATHS $ENV{JAVA_HOME}/jre/lib/amd64
	          $ENV{JAVA_HOME}/jre/lib/amd64/server  
	          $ENV{JAVA_HOME}/lib/server
	          $ENV{JAVA_HOME}/lib
	INC_PATHS $ENV{JAVA_HOME}/include
	# STATIC libjvm.a libjava.a libverify.a # libjawt.a 
	SHARED  jvm java verify # jawt(requires Xrender, Xtst, Xi)
	INCLUDE jni.h
)

SET_DEPS(
	NAME "HADOOP" 
	LIB_PATHS /opt/mapr/lib  
						$ENV{HADOOP_HOME}/lib/native
						${HADOOP_LIB_PATH}/lib/native
	INC_PATHS $ENV{HADOOP_HOME}/include 
			  		/opt/mapr/hadoop/hadoop-0.20.2/src/c++/libhdfs
						${HADOOP_INCLUDE_PATH}/include 
	STATIC libhdfs.a 
	SHARED hdfs 
	INCLUDE hdfs.h
)


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
	if (BUILTIN_FS STREQUAL "all" OR BUILTIN_FS STREQUAL "hadoop")
		set(BUILTIN_FS_TARGETS ${BUILTIN_FS_TARGETS} swcdb_fs_hadoop)
		if (NOT BUILTIN_FS STREQUAL "all")
			SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILTIN_FS_HADOOP")
		endif()
	endif()
	
endif()