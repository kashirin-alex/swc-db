#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#




########### SHELL UTILITY ###########

set(swcdb_utils_shell_sources
	cli/Shell.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_utils_shell
	SRCS	${swcdb_utils_shell_sources}
  TARGETS 
  
	SHARED	${EDITLINE_LIBRARIES_SHARED}
	STATIC	${EDITLINE_LIBRARIES_STATIC}
)
