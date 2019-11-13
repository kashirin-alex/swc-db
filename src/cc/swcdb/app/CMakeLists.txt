#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


#add_subdirectory(manager)
#add_subdirectory(ranger)

#add_subdirectory(thriftbroker)

# dbRanger
ADD_EXEC_TARGET(
	NAME dbRanger
	SRCS  ranger/main.cc
	TARGETS swcdb_core_config ${BUILTIN_FS_TARGETS}

	SHARED  ${SNAPPY_LIBRARIES_SHARED}
	STATIC  ${SNAPPY_LIBRARIES_STATIC} 
)

# dbManager
ADD_EXEC_TARGET(
	NAME dbManager
	SRCS  manager/main.cc
	TARGETS swcdb_core_config ${BUILTIN_FS_TARGETS}
)



# dbFsBroker
ADD_EXEC_TARGET(
	NAME dbFsBroker
	SRCS  fsbroker/main.cc
	TARGETS swcdb_core_config ${BUILTIN_FS_TARGETS}
	FLAGS "-DFS_BROKER_APP"
)
