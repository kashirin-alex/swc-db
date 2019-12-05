#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


#add_subdirectory(manager)
#add_subdirectory(ranger)

#add_subdirectory(thriftbroker)

get_property(SWC_LIB_CORE_SHARED GLOBAL PROPERTY SWC_LIB_CORE_SHARED)
get_property(SWC_LIB_CORE_STATIC GLOBAL PROPERTY SWC_LIB_CORE_STATIC)

get_property(SWC_LIB_FS_SHARED GLOBAL PROPERTY SWC_LIB_FS_SHARED)
get_property(SWC_LIB_FS_STATIC GLOBAL PROPERTY SWC_LIB_FS_STATIC)

set(SHARED ${SWC_LIB_CORE_SHARED} ${SWC_LIB_FS_SHARED})
set(STATIC ${SWC_LIB_CORE_STATIC} ${SWC_LIB_FS_STATIC})

set(FLAGS )
set(TARGETS )

if(SWC_IMPL_SOURCE)
	set(FLAGS "-DSWC_IMPL_SOURCE")
else()
	set(TARGETS "swcdb_core;swcdb_core_config;swcdb_core_comm;swcdb_fs;${BUILTIN_FS_TARGETS}")
endif()


# dbRanger
ADD_EXEC_TARGET(
	NAME dbRanger
	SRCS  ranger/main.cc
	TARGETS ${TARGETS}

	SHARED  ${SHARED}
	STATIC  ${STATIC}
	FLAGS 	${FLAGS}
)

# dbManager
ADD_EXEC_TARGET(
	NAME dbManager
	SRCS  manager/main.cc
	TARGETS ${TARGETS}

	SHARED  ${SHARED}
	STATIC  ${STATIC}
	FLAGS 	${FLAGS}
)



# dbFsBroker
ADD_EXEC_TARGET(
	NAME dbFsBroker
	SRCS  fsbroker/main.cc
	TARGETS ${TARGETS}

	SHARED  ${SHARED}
	STATIC  ${STATIC}
	FLAGS "-DFS_BROKER_APP" ${FLAGS}

	#ONLY_DYN TRUE
)



# swcdb
ADD_EXEC_TARGET(
	NAME swcdb
	SRCS  utils/main.cc
	TARGETS ${TARGETS}

	SHARED  ${SHARED}
	STATIC  ${STATIC}
	FLAGS 	${FLAGS}

	ONLY_DYN TRUE
)
