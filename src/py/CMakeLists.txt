#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#


install(
	DIRECTORY sbin/swcdb 
	FILE_PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE 
									 OWNER_WRITE 
									 OWNER_READ GROUP_READ WORLD_READ
	DESTINATION sbin
)


if(THRIFT_PY_GEN_PATH)

#### PREPARE SWC-DB PKG DIR

set(PY_PKG_PATH ${CMAKE_CURRENT_BINARY_DIR}/swcdb)

execute_process(COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/swcdb/pkg  ${PY_PKG_PATH}/)
configure_file(swcdb/setup.py.in ${PY_PKG_PATH}/setup.py @ONLY)
configure_file(swcdb/requirements.txt.in ${PY_PKG_PATH}/requirements.txt @ONLY)
configure_file(swcdb/README.md ${PY_PKG_PATH}/README.txt @ONLY)
#configure_file(swcdb/MANIFEST ${PY_PKG_PATH}/MANIFEST @ONLY)


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

endif()

