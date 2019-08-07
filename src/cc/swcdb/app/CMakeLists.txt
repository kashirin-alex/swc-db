#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


#add_subdirectory(manager)
#add_subdirectory(rangeserver)

#add_subdirectory(thriftbroker)

# swcdbRangeServer
ADD_EXEC_TARGET(
	NAME dbRangeServer
	SRCS  rangeserver/main.cc
	TARGETS swcdb_core_config swcdb_db swcdb_core_comm ${BUILTIN_FS_TARGETS}
)

# dbManager
ADD_EXEC_TARGET(
	NAME dbManager
	SRCS  manager/main.cc
	TARGETS swcdb_core_config swcdb_db swcdb_core_comm ${BUILTIN_FS_TARGETS}
)



# dbFsBroker
ADD_EXEC_TARGET(
	NAME dbFsBroker
	SRCS  fsbroker/main.cc
	TARGETS swcdb_core_config swcdb_db swcdb_core_comm ${BUILTIN_FS_TARGETS}
	FLAGS "-DFS_BROKER_APP"
)
