#
# Copyright (C) 2019-2020 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

set(PYTHON_EXECUTABLES )

# PYTHON 2
if (LANGS OR LANG_PY2)
	execute_process(COMMAND python -c "from distutils import sysconfig as s;
print(s.get_python_inc(plat_specific=True));
print(s.get_python_version());
"
  RESULT_VARIABLE _PY_SUCCESS
  OUTPUT_VARIABLE _PY_VALUES
	)
	if (_PY_SUCCESS  STREQUAL "0")
		string(REGEX REPLACE ";" "\\\\;" _PY_VALUES ${_PY_VALUES})
		string(REGEX REPLACE "\n" ";" _PY_VALUES ${_PY_VALUES})
		list(GET _PY_VALUES 0 PY_inc)
		list(GET _PY_VALUES 1 PY_version)

		if (PY_inc)
			set(PYTHON2_FOUND ON)
			set(PYTHON2_INCLUDE_DIR ${PY_inc})
			message(STATUS "Found Python${PY_version}: ${PYTHON2_INCLUDE_DIR}")
			
			set(PYTHON_EXECUTABLES ${PYTHON_EXECUTABLES} "python")
		endif ()
	endif ()

	if (NOT PYTHON2_FOUND AND LANG_PY2)
	    message(FATAL_ERROR "Requested for language, python2 is not available")
	endif ()
endif ()



# PYTHON 3
if (LANGS OR LANG_PY3)
	execute_process(COMMAND python3 -c "from distutils import sysconfig as s;
print(s.get_python_inc(plat_specific=True));
print(s.get_python_version());
"
  RESULT_VARIABLE _PY_SUCCESS
  OUTPUT_VARIABLE _PY_VALUES
	)
	if (_PY_SUCCESS  STREQUAL "0")
		string(REGEX REPLACE ";" "\\\\;" _PY_VALUES ${_PY_VALUES})
		string(REGEX REPLACE "\n" ";" _PY_VALUES ${_PY_VALUES})
		list(GET _PY_VALUES 0 PY_inc)
		list(GET _PY_VALUES 1 PY_version)

		if (PY_inc)
			set(PYTHON3_FOUND ON)
			set(PYTHON3_INCLUDE_DIR ${PY_inc})
			message(STATUS "Found Python${PY_version}: ${PYTHON3_INCLUDE_DIR}")

			set(PYTHON_EXECUTABLES ${PYTHON_EXECUTABLES} "python3")
		endif ()
	endif ()

	if (NOT PYTHON3_FOUND AND LANG_PY3)
	    message(FATAL_ERROR "Requested for language, python3 is not available")
	endif ()
endif ()


# PYPY 2
if (LANGS OR LANG_PYPY2)
	execute_process(COMMAND pypy -c "from distutils import sysconfig as s;
print(s.get_python_inc(plat_specific=True));
print(s.get_python_version());
"
  RESULT_VARIABLE _PY_SUCCESS
  OUTPUT_VARIABLE _PY_VALUES
	)
	if (_PY_SUCCESS  STREQUAL "0")
		string(REGEX REPLACE ";" "\\\\;" _PY_VALUES ${_PY_VALUES})
		string(REGEX REPLACE "\n" ";" _PY_VALUES ${_PY_VALUES})
		list(GET _PY_VALUES 0 PY_inc)
		list(GET _PY_VALUES 1 PY_version)

		if (PY_inc)
			set(PYPY2_FOUND ON)
			set(PYPY2_INCLUDE_DIR ${PY_inc})
			message(STATUS "Found PyPy${PY_version}: ${PYPY2_INCLUDE_DIR}")
			
			set(PYTHON_EXECUTABLES ${PYTHON_EXECUTABLES} "pypy")
		endif ()
	endif ()

	if (NOT PYPY2_FOUND AND LANG_PYPY2)
	    message(FATAL_ERROR "Requested for language, pypy2 is not available")
	endif ()
endif ()



# PYPY 3
if (LANGS OR LANG_PYPY3)
	execute_process(COMMAND pypy3 -c "from distutils import sysconfig as s;
print(s.get_python_inc(plat_specific=True));
print(s.get_python_version());
"
  RESULT_VARIABLE _PY_SUCCESS
  OUTPUT_VARIABLE _PY_VALUES
	)
	if (_PY_SUCCESS  STREQUAL "0")
		string(REGEX REPLACE ";" "\\\\;" _PY_VALUES ${_PY_VALUES})
		string(REGEX REPLACE "\n" ";" _PY_VALUES ${_PY_VALUES})
		list(GET _PY_VALUES 0 PY_inc)
		list(GET _PY_VALUES 1 PY_version)

		if (PY_inc)
			set(PYPY3_FOUND ON)
			set(PYPY3_INCLUDE_DIR ${PY_inc})
			message(STATUS "Found PyPy${PY_version}: ${PYPY3_INCLUDE_DIR}")
			
			set(PYTHON_EXECUTABLES ${PYTHON_EXECUTABLES} "pypy3")
		endif ()
	endif ()

	if (NOT PYPY3_FOUND AND LANG_PYPY3)
	    message(FATAL_ERROR "Requested for language, pypy3 is not available")
	endif ()
endif ()





#if (PY2_FOUND OR PY3_FOUND OR PYPY2_FOUND OR PYPY3_FOUND)
#	SET_DEPS(
#		NAME "PYBIND11" 
#		INCLUDE pybind11/pybind11.h
#	)
#	
#	if (NOT PYBIND11_INCLUDE_PATHS)
#		set(PYBIND11_FOUND OFF)
#		if(NOT LANGS) 
#			message(FATAL_ERROR "Requested for language pypy, dependency Pybind11 is not available")
#		endif ()
#	endif ()
#endif ()