#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


install(FILES sbin/swcdb_cluster DESTINATION sbin
  PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE 
              OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
)


if(NOT SWC_WITHOUT_PY AND THRIFT_PY_GEN_PATH)

#### PREPARE SWC-DB PKG DIR
set(SWCDB_VERSION_PYTHON "${SWCDB_VERSION}.0")

set(PY_PKG_PATH ${CMAKE_CURRENT_BINARY_DIR}/package)

execute_process(COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/package/swcdb  ${PY_PKG_PATH}/)
execute_process(COMMAND rm -rf  ${PY_PKG_PATH}/MANIFEST )
configure_file(package/setup.py.in ${PY_PKG_PATH}/setup.py @ONLY)
configure_file(package/requirements.txt.in ${PY_PKG_PATH}/requirements.txt @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/docs/use/thriftclient/python/README.md ${PY_PKG_PATH}/README.md @ONLY)


list(GET PYTHON_EXECUTABLES 0 py)
install( CODE "execute_process(COMMAND  cmake -E chdir ${PY_PKG_PATH} ${py} setup.py sdist;)")
install(FILES "${PY_PKG_PATH}/dist/swcdb-${SWCDB_VERSION_PYTHON}.tar.gz" DESTINATION lib/py/)
  

foreach(py ${PYTHON_EXECUTABLES})

  #add_test(Thrift-${py} env ${py} ${CMAKE_CURRENT_SOURCE_DIR}/tests/thrift_client_test.py)
  #add_test(Thrift-${py}-scw env ${py} ${CMAKE_CURRENT_SOURCE_DIR}/tests/thrift_client_test_scw.py)
  #add_test(Thrift-${py}-scr env ${py} ${CMAKE_CURRENT_SOURCE_DIR}/tests/thrift_client_test_scr.py)
  #add_test(Thrift-${py} 
  #          env LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${INSTALL_DIR}/lib 
  #          ${py} ${CMAKE_CURRENT_SOURCE_DIR}/tests/thrift_client_test.py)

  install( CODE "execute_process(COMMAND  cmake -E chdir ${PY_PKG_PATH}/dist ${py} -m pip install --verbose --upgrade swcdb-${SWCDB_VERSION_PYTHON}.tar.gz;)")
  install( CODE "execute_process(COMMAND  cmake -E chdir ${PY_PKG_PATH} ${py} -m pip install --upgrade -r requirements.txt;)")

endforeach()

endif()

