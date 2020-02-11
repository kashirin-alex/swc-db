#
# Copyright (C) 2019-2020 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


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
	find_package(Py)
endif ()
