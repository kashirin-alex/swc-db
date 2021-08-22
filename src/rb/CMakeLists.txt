#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


if(NOT SWC_WITHOUT_RB AND THRIFT_RUBY_GEN_PATH AND RUBY_FOUND)

#### PREPARE SWC-DB PKG DIR
set(SWCDB_VERSION_RUBY "${SWCDB_VERSION}.0")

set(RB_PKG_PATH ${CMAKE_CURRENT_BINARY_DIR}/package)

execute_process(COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/package ${CMAKE_CURRENT_BINARY_DIR}/)

configure_file(${RB_PKG_PATH}/swcdb.gemspec.in ${RB_PKG_PATH}/swcdb.gemspec @ONLY)
execute_process(COMMAND rm ${RB_PKG_PATH}/swcdb.gemspec.in)
configure_file(package/README.md ${RB_PKG_PATH}/README.md @ONLY)


install( CODE "execute_process(COMMAND  cmake -E chdir ${RB_PKG_PATH} gem build swcdb.gemspec;)")
install( CODE "execute_process(COMMAND  cmake -E chdir ${RB_PKG_PATH} gem install swcdb-${SWCDB_VERSION_RUBY}.gem;)")

endif()

