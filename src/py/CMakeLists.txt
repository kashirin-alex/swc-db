#
# Copyright (C) 2019-2020 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


#### PREPARE HYPERTABLE PKG DIR
add_custom_target(ThriftPy ALL)

set(PY_PKG_PATH ${CMAKE_CURRENT_BINARY_DIR}/swcdb)

add_custom_command(
  TARGET   ThriftPy
  COMMAND  cp 
  ARGS     -r ${CMAKE_CURRENT_SOURCE_DIR}/swcdb/pkg  ${PY_PKG_PATH}/
)

configure_file(swcdb/setup.py.in ${PY_PKG_PATH}/setup.py @ONLY)
configure_file(swcdb/requirements.txt ${PY_PKG_PATH}/requirements.txt @ONLY)
configure_file(swcdb/README.md ${PY_PKG_PATH}/README.txt @ONLY)
#configure_file(swcdb/MANIFEST ${PY_PKG_PATH}/MANIFEST @ONLY)

set(thrift_ver_gen_path ${CMAKE_CURRENT_SOURCE_DIR}/swcdb/thriftgen-${THRIFT_VERSION})
execute_process(COMMAND rm -rf ${thrift_ver_gen_path} ) ## temporary

#### GENERATE THRIFT module

if(THRIFT_FOUND AND NOT EXISTS ${thrift_ver_gen_path})
	execute_process(COMMAND mkdir -p ${thrift_ver_gen_path} ) ## temporary

  set(thrift_service ${CMAKE_CURRENT_SOURCE_DIR}/../cc/lib/swcdb/thrift/Service.thrift)
	add_custom_command(
		TARGET 		ThriftPy
	  COMMAND   thrift
	  ARGS      -r --gen py:slots,no_utf8strings -o ${thrift_ver_gen_path}
			  		  ${thrift_service}
		DEPENDS   ${thrift_service}
	  COMMENT   "Thrift Python Gen"
	)
		
endif ()


#### SET THRIFT module to SWCDB PKG
add_custom_command(
	TARGET 	  ThriftPy
	COMMAND   mkdir 
	ARGS      -p ${PY_PKG_PATH}/pkg/thrift/gen
)
add_custom_command(
	TARGET 	  ThriftPy
	COMMAND   cp 
	ARGS      -r ${thrift_ver_gen_path}/gen-py/swcdb/thrift/gen ${PY_PKG_PATH}/pkg/thrift/
)


list(GET PYTHON_EXECUTABLES 0 py)
install( CODE "execute_process(COMMAND  cmake -E chdir ${PY_PKG_PATH} ${py} setup.py sdist;)")
install(FILES "${PY_PKG_PATH}/dist/swcdb-${SWC_VERSION}.tar.gz" DESTINATION lib/py/)
	

foreach(py ${PYTHON_EXECUTABLES})

	#add_test(Thrift-${py} env ${py} ${CMAKE_CURRENT_SOURCE_DIR}/tests/thrift_client_test.py)
	#add_test(Thrift-${py}-scw env ${py} ${CMAKE_CURRENT_SOURCE_DIR}/tests/thrift_client_test_scw.py)
	#add_test(Thrift-${py}-scr env ${py} ${CMAKE_CURRENT_SOURCE_DIR}/tests/thrift_client_test_scr.py)
	#add_test(Thrift-${py} 
	#					env LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${INSTALL_DIR}/lib 
	#					${py} ${CMAKE_CURRENT_SOURCE_DIR}/tests/thrift_client_test.py)

	install( CODE "execute_process(COMMAND  cmake -E chdir ${PY_PKG_PATH}/dist ${py} -m pip install --verbose --upgrade swcdb-${SWC_VERSION}.tar.gz;)")
	install( CODE "execute_process(COMMAND  cmake -E chdir ${PY_PKG_PATH} ${py} -m pip install --upgrade -r requirements.txt;)")

endforeach()

