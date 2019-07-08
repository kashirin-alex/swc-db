#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#




ADD_EXEC_TARGET(
	NAME      test_mngr_client
	SRCS      test/test_mngr_client.cc
	TARGETS   swcdb_db swcdb_core_comm swcdb_core_config
)