#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



get_property(SWC_VERSION_SRC GLOBAL PROPERTY SWC_VERSION_SRC)



add_subdirectory(core)          #ifndef swc_core_path_FileName_h


add_subdirectory(fs)            #ifndef swc_fs_FileName_h

add_subdirectory(db)            #ifndef swc_db_FileName_h


add_subdirectory(ranger)        #ifndef swc_ranger_FileName_h

#add_subdirectory(manager)      #ifndef swc_manager_FileName_h

add_subdirectory(fsbroker)      #ifndef swc_fsbroker_FileName_h

add_subdirectory(thrift)        #ifndef swc_thrift_FileName_h



add_subdirectory(utils)      #ifndef swc_fsbroker_FileName_h

