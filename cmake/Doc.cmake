#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>

if(SWC_DOCUMENTATION) 

  find_package(Doxygen REQUIRED)

  configure_file(${CMAKE_MODULE_PATH}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR} @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)



  if(THRIFT_COMPILER_FOUND)

    add_custom_target(doc_thrift_html thrift -gen html
                      -o ${CMAKE_CURRENT_BINARY_DIR}/doc
                      ${CMAKE_CURRENT_SOURCE_DIR}/src/thrift/swcdb/Service.thrift)
    add_custom_command(TARGET doc POST_BUILD COMMAND make doc_thrift_html)

    ## require - thrift t_markdown_generator
    #   https://github.com/kashirin-alex/thrift/commit/b371bbf9750f196f21d244b8c3d58527c8e3961b
    ## add_custom_target(doc_thrift_md thrift -gen markdown:suffix=md
    ##                   -o ${CMAKE_CURRENT_BINARY_DIR}/doc
    ##                   ${CMAKE_CURRENT_SOURCE_DIR}/src/thrift/swcdb/Service.thrift)
    ## add_custom_command(TARGET doc POST_BUILD COMMAND make doc_thrift_md)

  endif ()


  add_custom_command(TARGET doc POST_BUILD COMMAND 
                     XZ_OPT=-e9 tar -cJf swc-db-doc.tar.xz doc/*
                     )


endif ()