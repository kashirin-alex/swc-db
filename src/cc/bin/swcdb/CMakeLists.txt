#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


#add_subdirectory(manager)
#add_subdirectory(ranger)

#add_subdirectory(thriftbroker)

get_property(SWC_LIB_CORE_SHARED GLOBAL PROPERTY SWC_LIB_CORE_SHARED)
get_property(SWC_LIB_CORE_STATIC GLOBAL PROPERTY SWC_LIB_CORE_STATIC)

set(FLAGS )
set(TARGETS )
if(SWC_IMPL_SOURCE)
	set(FLAGS "-DSWC_IMPL_SOURCE")
else()
	set(TARGETS "swcdb_core;swcdb_core_config;${BUILTIN_FS_TARGETS}")
endif()


# dbRanger
ADD_EXEC_TARGET(
	NAME dbRanger
	SRCS  ranger/main.cc
	TARGETS ${TARGETS}

	SHARED  ${SWC_LIB_CORE_SHARED}
	STATIC  ${SWC_LIB_CORE_STATIC}
	FLAGS 	${FLAGS}
)

# dbManager
ADD_EXEC_TARGET(
	NAME dbManager
	SRCS  manager/main.cc
	TARGETS ${TARGETS}

	SHARED  ${SWC_LIB_CORE_SHARED}
	STATIC  ${SWC_LIB_CORE_STATIC}
	FLAGS 	${FLAGS}
)



# dbFsBroker
ADD_EXEC_TARGET(
	NAME dbFsBroker
	SRCS  fsbroker/main.cc
	TARGETS ${TARGETS}

	SHARED  ${SWC_LIB_CORE_SHARED}
	STATIC  ${SWC_LIB_CORE_STATIC}
	FLAGS "-DFS_BROKER_APP" ${FLAGS}

	ONLY_DYN TRUE
)



# swcdb
ADD_EXEC_TARGET(
	NAME swcdb
	SRCS  utils/main.cc
	TARGETS ${TARGETS}

	SHARED  ${SWC_LIB_CORE_SHARED}
	STATIC  ${SWC_LIB_CORE_STATIC}
	FLAGS 	${FLAGS}

	ONLY_DYN TRUE
)
