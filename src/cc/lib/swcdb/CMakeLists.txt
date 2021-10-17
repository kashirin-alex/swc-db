#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


if(MINGW)
  include (GenerateExportHeader)
endif()

get_property(SWC_VERSION_SRC GLOBAL PROPERTY SWC_VERSION_SRC)



if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-core")
  add_subdirectory(core)          #ifndef swcdb_core_path_FileName_h
endif()


if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG MATCHES "^lib-fs")
  add_subdirectory(fs)            #ifndef swcdb_fs_FileName_h
endif()


if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib")
  add_subdirectory(db)            #ifndef swcdb_db_FileName_h
endif()


if(NOT SWC_BUILD_PKG)
  #add_subdirectory(ranger)        #ifndef swcdb_ranger_FileName_h

  #add_subdirectory(manager)      #ifndef swcdb_manager_FileName_h

  #add_subdirectory(fsbroker)      #ifndef swcdb_fsbroker_FileName_h
endif()


if((NOT SWC_BUILD_PKG AND NOT SWC_WITHOUT_THRIFT) OR SWC_BUILD_PKG STREQUAL "lib-thrift")
  add_subdirectory(thrift)        #ifndef swcdb_thrift_FileName_h
endif()


if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "utils")
  add_subdirectory(utils)          #ifndef swcdb_fsbroker_FileName_h
endif()

