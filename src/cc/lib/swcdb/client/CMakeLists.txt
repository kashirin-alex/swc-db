#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


set(swcdb_client_sources
	Settings.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb_client
	SRCS 		${swcdb_client_sources}
	TARGETS swcdb_core swcdb_core_config swcdb_core_comm
)


ADD_TEST_TARGET(
	NAME      test_echo
	SRCS      test/test_echo.cc
	TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb_client
)

ADD_TEST_TARGET(
	NAME      test_mngr_client
	SRCS      test/test_mngr_client.cc
	TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb_client
)

ADD_TEST_TARGET(
	NAME      test_mngr_mng_column
	SRCS      test/test_mngr_mng_column.cc
	TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb_client
)
ADD_TEST_TARGET(
	NAME      test_query
	SRCS      test/test_query.cc
	TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb_client
)