#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


if (LANGS OR LANG_RUBY)

  execute_process(
    COMMAND ruby --version
    OUTPUT_VARIABLE RUBY_VERSION
    RESULT_VARIABLE Ruby_RETURN
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (Ruby_RETURN EQUAL 0)
    set(RUBY_FOUND TRUE)
    string(REPLACE "\n" ";" RUBY_VERSION ${RUBY_VERSION})
    list(GET RUBY_VERSION 0 RUBY_VERSION)
    message(STATUS "Found Ruby: ${RUBY_VERSION}")
  else()
    set(RUBY_FOUND FALSE)
    message(STATUS "Not Found Ruby")
  endif()

  if (NOT RUBY_FOUND AND LANG_RUBY)
    message(FATAL_ERROR "Requested for language, ruby is not available")
  endif ()

endif ()
