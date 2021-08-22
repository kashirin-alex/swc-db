#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


# Languages Config


message(STATUS "Supported Languages:")

string(TOUPPER "${SWC_LANGUAGES}" SWC_LANGUAGES )

# Build support for all possible or only requested languages
if (NOT SWC_LANGUAGES OR SWC_LANGUAGES STREQUAL "ALL" OR SWC_LANGUAGES STREQUAL "")
  set(LANGS ON)
  message("       Any possibly supported")

else ()
  string(REPLACE "," ";" LANGS "${SWC_LANGUAGES}")
  foreach(lg ${LANGS})
    SET("LANG_${lg}" ON)
    message("       Building for ${lg}")
  endforeach()
  set(LANGS OFF)
endif ()





# Specific to to a Language

if (LANGS OR LANG_PY2 OR LANG_PY3 OR LANG_PYPY2 OR LANG_PYPY3)
  find_package(LangPy)
endif ()

if (LANGS OR LANG_RUBY)
  find_package(LangRuby)
endif ()

if (LANGS OR LANG_JAVA)
  find_package(LangJava)
endif ()

