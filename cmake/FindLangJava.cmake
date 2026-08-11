#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



set(java_home)
if(JAVA_INSTALL_PATH)
  set(java_home ${JAVA_INSTALL_PATH})
else()
  set(java_home $ENV{JAVA_HOME})
endif()

SET_DEPS(
  NAME "JAVA"
  LIB_PATHS ${java_home}/jre/lib/amd64
            ${java_home}/jre/lib/amd64/server
            ${java_home}/lib/server
            ${java_home}/lib
  INC_PATHS ${java_home}/include
  # STATIC libjvm.a libjava.a libverify.a # libjawt.a
  SHARED    jvm java verify # jawt(requires Xrender, Xtst, Xi)
  INCLUDE   jni.h
)



execute_process(
  COMMAND java --version
  OUTPUT_VARIABLE JAVARE_VERSION
  RESULT_VARIABLE Java_RETURN
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_VARIABLE JAVARE_VERSION_ERR
)
# Some JREs print version on stderr
if(NOT JAVARE_VERSION AND JAVARE_VERSION_ERR)
  set(JAVARE_VERSION ${JAVARE_VERSION_ERR})
endif()
if (Java_RETURN EQUAL 0)
  set(JAVARE_FOUND TRUE)
  string(REPLACE "\n" ";" JAVARE_VERSION ${JAVARE_VERSION})
  list(GET JAVARE_VERSION 0 JAVARE_VERSION)

  message(STATUS "Found Java Runtime Enviroment:")
  message("       ${JAVARE_VERSION}")

else()
  set(JAVARE_VERSION )
  set(JAVARE_FOUND FALSE)
  message(STATUS "Not Found Java Runtime Enviroment")
endif()


execute_process(
  COMMAND javac --version
  OUTPUT_VARIABLE JAVAC_VERSION
  RESULT_VARIABLE Java_RETURN
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_VARIABLE JAVAC_VERSION_ERR
)
if(NOT JAVAC_VERSION AND JAVAC_VERSION_ERR)
  set(JAVAC_VERSION ${JAVAC_VERSION_ERR})
endif()
if (Java_RETURN EQUAL 0)
  set(JAVAC_FOUND TRUE)
  string(REPLACE "\n" ";" JAVAC_VERSION ${JAVAC_VERSION})
  list(GET JAVAC_VERSION 0 JAVAC_VERSION)

  message(STATUS "Found Java Compiler:")
  message("       ${JAVAC_VERSION}")

else()
  set(JAVAC_VERSION )
  set(JAVAC_FOUND FALSE)
  message(STATUS "Not Found Java Compiler")
endif()


execute_process(
  COMMAND mvn --version
  OUTPUT_VARIABLE MAVEN_VERSION
  RESULT_VARIABLE Java_RETURN
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX REPLACE "\n.*" "" MAVEN_VERSION "${MAVEN_VERSION}")
if(MAVEN_VERSION MATCHES "Apache Maven")
  set(MAVEN_FOUND TRUE)
  list(GET MAVEN_VERSION 0 MAVEN_VERSION)

  message(STATUS "Found Maven:")
  message("       ${MAVEN_VERSION}")

else()
  set(MAVEN_VERSION )
  set(MAVEN_FOUND FALSE)
  message(STATUS "Not Found Maven")
endif()
