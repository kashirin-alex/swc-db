#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#




########### SHELL UTILITY ###########

set(swcdb_utils_shell_sources
	cli/Shell.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_utils_shell
	SRCS	${swcdb_utils_shell_sources}
  TARGETS	swcdb_core swcdb_core_config swcdb_core_comm
  
	SHARED	${EDITLINE_LIBRARIES_SHARED}
	STATIC	${EDITLINE_LIBRARIES_STATIC}
)
