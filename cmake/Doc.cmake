#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#

if(SWC_DOCUMENTATION) 

  find_package(Doxygen REQUIRED)

  configure_file(doc/Doxyfile ${CMAKE_CURRENT_BINARY_DIR} @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)



  if(THRIFT_COMPILER_FOUND)
    add_custom_target(doc_thrift thrift -gen html:standalone
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