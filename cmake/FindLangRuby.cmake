#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


if (LANGS OR LANG_RUBY)

  exec_program(env ARGS ruby --version OUTPUT_VARIABLE RUBY_VERSION
               RETURN_VALUE Ruby_RETURN)
  if (Ruby_RETURN STREQUAL "0")
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
