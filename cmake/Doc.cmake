#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>

if(SWC_DOCUMENTATION) 

  find_package(Doxygen REQUIRED)

  configure_file(Doxyfile ${CMAKE_CURRENT_BINARY_DIR} @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)



  if(THRIFT_COMPILER_FOUND)
    add_custom_target(doc_thrift thrift -gen html
                      -o ${CMAKE_CURRENT_BINARY_DIR}/doc
                      ${CMAKE_CURRENT_SOURCE_DIR}/src/thrift/swcdb/Service.thrift)
    add_custom_command(TARGET doc POST_BUILD COMMAND make doc_thrift)
  endif ()


  add_custom_command(TARGET doc POST_BUILD COMMAND 
                     tar --xz
                     --directory=${CMAKE_CURRENT_BINARY_DIR}
                     --create --file=swc-db-doc.tar.xz 
                     doc )

endif ()