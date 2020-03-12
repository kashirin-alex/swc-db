#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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
	NAME "HADOOP_JVM" 
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



SET_DEPS(
	NAME "HADOOP" 
	LIB_PATHS ""
	INC_PATHS ""
	STATIC libhdfspp.a
	SHARED hdfspp 
	INCLUDE hdfspp/hdfspp.h
)
SET_DEPS(
	NAME "PROTOBUF" 
	LIB_PATHS ""
	INC_PATHS ""
	STATIC libprotobuf.a
	SHARED protobuf 
	INCLUDE google/protobuf/stubs/common.h
)




set(BUILTIN_FS_TARGETS "")
if(SWC_BUILTIN_FS)

	if (SWC_BUILTIN_FS STREQUAL "all")
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILTIN_FS_ALL")
	else()
	
		string(REPLACE "," ";" SWC_BUILTIN_FS "${SWC_BUILTIN_FS}")
		list(REMOVE_DUPLICATES SWC_BUILTIN_FS)
		foreach(fs ${SWC_BUILTIN_FS})
			string(TOLOWER ${fs} fs)
			set(BUILTIN_FS_TARGETS ${BUILTIN_FS_TARGETS} swcdb_fs_${fs})
			string(TOUPPER ${fs} fs)
			set("BUILTIN_FS_${fs}" ON)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILTIN_FS_${fs}")
		endforeach()
	endif()


	# if (SWC_BUILTIN_FS STREQUAL "all" OR SWC_BUILTIN_FS STREQUAL "broker")
	#	set(BUILTIN_FS_TARGETS ${BUILTIN_FS_TARGETS} swcdb_fs_broker)
	#	if (NOT SWC_BUILTIN_FS STREQUAL "all")
	#		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILTIN_FS_BROKER")
	#	endif()
	# endif()

endif()