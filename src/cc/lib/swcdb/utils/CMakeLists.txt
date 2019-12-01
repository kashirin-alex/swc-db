#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#




########### SHELL UTILITY ###########

set(swcdb_utils_shell_sources
Shell.cc
)
ADD_LIB_TARGET(
	NAME	swcdb_utils_shell
	SRCS	${swcdb_utils_shell_sources}
  TARGETS	swcdb_core swcdb_core_config
  
	SHARED	${EDITLINE_LIBRARIES_SHARED} ${TINFOW_LIBRARIES_SHARED} 
	STATIC	${EDITLINE_LIBRARIES_STATIC} ${TINFOW_LIBRARIES_STATIC} 
)
