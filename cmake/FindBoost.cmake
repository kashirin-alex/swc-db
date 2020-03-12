#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#

set(BOOST_LIBNAMES 
 # system 
 # filesystem 
 # iostreams
 # thread 
 # chrono 
)

SET(BOOST_STATIC_NAMES "")
SET(BOOST_SHARED_NAMES "")
foreach(libname ${BOOST_LIBNAMES})
	set(BOOST_STATIC_NAMES ${BOOST_STATIC_NAMES} "libboost_${libname}.a")	
	set(BOOST_SHARED_NAMES ${BOOST_SHARED_NAMES} "boost_${libname}")	
endforeach()

SET_DEPS(
	NAME "BOOST" 
	REQUIRED TRUE
	LIB_PATHS ${BOOST_INCDIR_SEARCH}
	INC_PATHS ${BOOST_LIBDIR_SEARCH}
	STATIC ${BOOST_STATIC_NAMES}
	SHARED ${BOOST_SHARED_NAMES}
	INCLUDE boost/config.hpp
)